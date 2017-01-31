#include "sepolicy-inject.h"

static void set_domain(char* s, int value) {
	type_datum_t *type;
	if (!exists(s))
		create_domain(s);
	type = hashtab_search(policy->p_types.table, s);
	if (type == NULL) {
			fprintf(stderr, "type %s does not exist\n", s);
			return;
	}
	if (ebitmap_set_bit(&policy->permissive_map, type->s.value, value)) {
		fprintf(stderr, "Could not set bit in permissive map\n");
		return;
	}
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
	set_domain(s, 1);
}

void enforce(char *s) {
	set_domain(s, 0);
}

void attradd(char *s, char *a) {
	add_type(s, a);
}

int exists(char* source) {
	return !! hashtab_search(policy->p_types.table, source);
}
