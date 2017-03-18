#include "magiskpolicy.h"

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

int allow(char *s, char *t, char *c, char *p) {
	return add_rule(s, t, c, p, AVTAB_ALLOWED, 0);
}

int deny(char *s, char *t, char *c, char *p) {
	return add_rule(s, t, c, p, AVTAB_ALLOWED, 1);
}

int auditallow(char *s, char *t, char *c, char *p) {
	return add_rule(s, t, c, p, AVTAB_AUDITALLOW, 0);
}

int auditdeny(char *s, char *t, char *c, char *p) {
	return add_rule(s, t, c, p, AVTAB_AUDITDENY, 0);
}

int typetrans(char *s, char *t, char *c, char *d, char *o) {
	if (o == NULL)
		return add_transition(s, t, c, d);
	else
		return add_file_transition(s, t, c, d, o);
}

int permissive(char *s) {
	return set_domain_state(s, 1);
}

int enforce(char *s) {
	return set_domain_state(s, 0);
}

int create(char *s) {
	return create_domain(s);
}

int attradd(char *s, char *a) {
	return add_typeattribute(s, a);
}

int exists(char* source) {
	return !! hashtab_search(policy->p_types.table, source);
}
