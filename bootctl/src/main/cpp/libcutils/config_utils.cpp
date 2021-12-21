/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cutils/config_utils.h>

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <cutils/misc.h>

cnode* config_node(const char *name, const char *value)
{
    cnode* node = static_cast<cnode*>(calloc(sizeof(cnode), 1));
    if(node) {
        node->name = name ? name : "";
        node->value = value ? value : "";
    }

    return node;
}

cnode* config_find(cnode *root, const char *name)
{
    cnode *node, *match = NULL;

    /* we walk the whole list, as we need to return the last (newest) entry */
    for(node = root->first_child; node; node = node->next)
        if(!strcmp(node->name, name))
            match = node;

    return match;
}

static cnode* _config_create(cnode *root, const char *name)
{
    cnode *node;

    node = config_node(name, NULL);

    if(root->last_child)
        root->last_child->next = node;
    else
        root->first_child = node;

    root->last_child = node;

    return node;
}

int config_bool(cnode *root, const char *name, int _default)
{
    cnode *node;
        
    node = config_find(root, name);
    if(!node)
        return _default;

    switch(node->value[0]) {
    case 'y':
    case 'Y':
    case '1':
        return 1;
    default:
        return 0;
    }
}

const char* config_str(cnode *root, const char *name, const char *_default)
{
    cnode *node;

    node = config_find(root, name);
    if(!node)
        return _default;
    return node->value;
}

void config_set(cnode *root, const char *name, const char *value)
{
    cnode *node;

    node = config_find(root, name);
    if(node)
        node->value = value;
    else {
        node = _config_create(root, name);
        node->value = value;
    }
}

#define T_EOF 0
#define T_TEXT 1
#define T_DOT 2
#define T_OBRACE 3
#define T_CBRACE 4

typedef struct
{
    char *data;
    char *text;
    int len;
    char next;
} cstate;

static int _lex(cstate *cs, int value)
{
    char c;
    char *s;
    char *data;

    data = cs->data;

    if(cs->next != 0) {
        c = cs->next;
        cs->next = 0;
        goto got_c;
    }

restart:
    for(;;) {
        c = *data++;
    got_c:
        if(isspace(c))
            continue;

        switch(c) {
        case 0:
            return T_EOF;

        case '#':
            for(;;) {
                switch(*data) {
                case 0:
                    cs->data = data;
                    return T_EOF;
                case '\n':
                    cs->data = data + 1;
                    goto restart;
                default:
                    data++;
                }
            }
            break;
            
        case '.':
            cs->data = data;
            return T_DOT;

        case '{':
            cs->data = data;
            return T_OBRACE;

        case '}':
            cs->data = data;
            return T_CBRACE;

        default:
            s = data - 1;

            if(value) {
                for(;;) {
                    if(*data == 0) {
                        cs->data = data;
                        break;
                    }
                    if(*data == '\n') {
                        cs->data = data + 1;
                        *data-- = 0;
                        break;
                    }
                    data++;
                }

                    /* strip trailing whitespace */
                while(data > s){
                    if(!isspace(*data)) break;
                    *data-- = 0;
                }

                goto got_text;                
            } else {
                for(;;) {
                    if(isspace(*data)) {
                        *data = 0;
                        cs->data = data + 1;
                        goto got_text;
                    }
                    switch(*data) {
                    case 0:
                        cs->data = data;
                        goto got_text;
                    case '.':
                    case '{':
                    case '}':
                        cs->next = *data;
                        *data = 0;
                        cs->data = data + 1;
                        goto got_text;
                    default:
                        data++;
                    }
                }
            }
        }
    }

got_text:
    cs->text = s;
    return T_TEXT;
}

#if 0
char *TOKENNAMES[] = { "EOF", "TEXT", "DOT", "OBRACE", "CBRACE" };

static int lex(cstate *cs, int value)
{
    int tok = _lex(cs, value);
    printf("TOKEN(%d) %s %s\n", value, TOKENNAMES[tok],
           tok == T_TEXT ? cs->text : "");
    return tok;
}
#else
#define lex(cs,v) _lex(cs,v)
#endif

static int parse_expr(cstate *cs, cnode *node);

static int parse_block(cstate *cs, cnode *node)
{
    for(;;){
        switch(lex(cs, 0)){
        case T_TEXT:
            if(parse_expr(cs, node)) return -1;
            continue;

        case T_CBRACE:
            return 0;

        default:
            return -1;
        }
    }
}

static int parse_expr(cstate *cs, cnode *root)
{
    cnode *node;

        /* last token was T_TEXT */
    node = config_find(root, cs->text);
    if(!node || *node->value)
        node = _config_create(root, cs->text);

    for(;;) {
        switch(lex(cs, 1)) {
        case T_DOT:
            if(lex(cs, 0) != T_TEXT)
                return -1;
            node = _config_create(node, cs->text);
            continue;

        case T_TEXT:
            node->value = cs->text;
            return 0;

        case T_OBRACE:
            return parse_block(cs, node);

        default:
            return -1;
        }
    }
}

void config_load(cnode *root, char *data)
{
    if(data != 0) {
        cstate cs;
        cs.data = data;
        cs.next = 0;

        for(;;) {
            switch(lex(&cs, 0)) {
            case T_TEXT:
                if(parse_expr(&cs, root))
                    return;
                break;
            default:
                return;
            }
        }
    }
}

void config_load_file(cnode *root, const char *fn)
{
    char* data = static_cast<char*>(load_file(fn, nullptr));
    config_load(root, data);
    // TODO: deliberate leak :-/
}

void config_free(cnode *root)
{
    cnode *cur = root->first_child;

    while (cur) {
        cnode *prev = cur;
        config_free(cur);
        cur = cur->next;
        free(prev);
    }
}
