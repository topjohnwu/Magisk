#include "sepolicy-inject.h"

void vec_init(vector *v) {
	v->size = 0;
	v->cap = 1;
	v->data = (char**) malloc(sizeof(char*));
}

void vec_push_back(vector *v, char* s) {
	if (v == NULL) return;
	if (v->size == v->cap) {
		v->cap *= 2;
		v->data = (char**) realloc(v->data, sizeof(char*) * v->cap);
	}
	v->data[v->size] = s;
	++v->size;
}

void vec_destroy(vector *v) {
	v->size = 0;
	v->cap = 0;
	free(v->data);
}

void allow(char *s, char *t, char *c, char *p) {
	add_rule(s, t, c, p, AVTAB_ALLOWED, 0);
}

void deny(char *s, char *t, char *c, char *p) {
	add_rule(s, t, c, p, AVTAB_ALLOWED, 1);
}

void auditallow(char *s, char *t, char *c, char *p) {
	add_rule(s, t, c, p, AVTAB_AUDITALLOW, 0);
}

void auditdeny(char *s, char *t, char *c, char *p) {
	add_rule(s, t, c, p, AVTAB_AUDITDENY, 0);
}

void permissive(char *s) {
	set_domain_state(s, 1);
}

void enforce(char *s) {
	set_domain_state(s, 0);
}

void attradd(char *s, char *a) {
	add_typeattribute(s, a);
}

int exists(char* source) {
	return !! hashtab_search(policy->p_types.table, source);
}
