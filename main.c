#include "sepolicy-inject.h"

static void usage(char *arg0) {
	fprintf(stderr, "%s [--live] [--minimal] [--load <infile>] [--save <outfile>] [policystatement...]\n\n", arg0);
	fprintf(stderr, "Supported policy statements:\n\n");
	fprintf(stderr, "\"allow source-class target-class permission-class permission\"\n");
	fprintf(stderr, "\"deny source-class target-class permission-class permission\"\n");
	fprintf(stderr, "\"permissive class\"\n");
	fprintf(stderr, "\"enforcing class\"\n");
	fprintf(stderr, "\"attradd class attribute\"\n");
	fprintf(stderr, "\n");
	exit(1);
}

// Pattern 1: action { source } { target } class { permission }
static int parse_pattern_1(int action, char* statement) {
	int state = 0, in_bracket = 0;
	char *tok, *class;
	struct vector source, target, permission, *temp;
	vec_init(&source);
	vec_init(&target);
	vec_init(&permission);
	tok = strtok(statement, " ");
	while (tok != NULL) {
		if (tok[0] == '{') {
			if (in_bracket || state == 2) return 1;
			in_bracket = 1;
			if (tok[1]) {
				++tok;
				continue;
			}
		} else if (tok[strlen(tok) - 1] == '}') {
			if (!in_bracket || state == 2) return 1;
			in_bracket = 0;
			if (strlen(tok) - 1) {
				tok[strlen(tok) - 1] = '\0';
				continue;
			}
		} else {
			if (tok[0] == '*') tok = ALL;
			switch (state) {
				case 0:
					temp = &source;
					break;
				case 1:
					temp = &target;
					break;
				case 2:
					temp = NULL;
					class = tok;
					break;
				case 3:
					temp = &permission;
					break;
				default:
					return 1;
			}
			vec_push_back(temp, tok);
		}
		if (!in_bracket) ++state;
		tok = strtok(NULL, " ");
	}
	if (state != 4) return 1;
	for(int i = 0; i < source.size; ++i)
		for (int j = 0; j < target.size; ++j)
			for (int k = 0; k < permission.size; ++k)
				switch (action) {
					case 0:
						allow(source.data[i], target.data[j], class, permission.data[k]);
						break;
					case 1:
						deny(source.data[i], target.data[j], class, permission.data[k]);
						break;
					case 2:
						auditallow(source.data[i], target.data[j], class, permission.data[k]);
						break;
					case 3:
						auditdeny(source.data[i], target.data[j], class, permission.data[k]);
						break;
					default:
						return 1;
				}
	vec_destroy(&source);
	vec_destroy(&target);
	vec_destroy(&permission);
	return 0;
}

int main(int argc, char *argv[]) {
	char *infile = NULL, *outfile, *tok, cpy[ARG_MAX];
	int live = 0, minimal = 0;
	struct vector rules;

	vec_init(&rules);

	if (argc < 2) usage(argv[0]);
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] == '-' && argv[i][1] == '-') {
			if (strcmp(argv[i], "--live") == 0)
				live = 1;
			else if (strcmp(argv[i], "--minimal") == 0)
				minimal = 1;
			else if (strcmp(argv[i], "--load") == 0) {
				if (i + 1 >= argc) usage(argv[0]);
				infile = argv[i + 1];
				i += 1;
			} else if (strcmp(argv[i], "--save") == 0) {
				if (i + 1 >= argc) usage(argv[0]);
				outfile = argv[i + 1];
				i += 1;
			} else 
				usage(argv[0]);
		} else
			vec_push_back(&rules, argv[i]);
	}

	policydb_t policydb;
	struct policy_file pf;
	sidtab_t sidtab;

	// Use current policy if not specified
	if(!infile)
		infile = "/sys/fs/selinux/policy";

	sepol_set_policydb(&policydb);
	sepol_set_sidtab(&sidtab);

	if (load_policy(infile, &policydb, &pf)) {
		fprintf(stderr, "Could not load policy\n");
		return 1;
	}

	if (policydb_load_isids(&policydb, &sidtab))
		return 1;

	policy = &policydb;

	if (!minimal && rules.size == 0) su_rules();
	if (minimal) min_rules();

	for (int i = 0; i < rules.size; ++i) {
		// Since strtok will modify the origin string, copy the policy for error messages
		strcpy(cpy, rules.data[i]);
		tok = strtok(rules.data[i], " ");
		if (strcmp(tok, "allow") == 0) {
			if (parse_pattern_1(0, rules.data[i] + strlen(tok) + 1))
				printf("Syntax error in \"%s\"\n", cpy);
		} else if (strcmp(tok, "deny") == 0) {
			if (parse_pattern_1(1, rules.data[i] + strlen(tok) + 1))
				printf("Syntax error in \"%s\"\n", cpy);
		} else if (strcmp(tok, "auditallow") == 0) {
			if (parse_pattern_1(2, rules.data[i] + strlen(tok) + 1))
				printf("Syntax error in \"%s\"\n", cpy);
		} else if (strcmp(tok, "auditdeny") == 0) {
			if (parse_pattern_1(3, rules.data[i] + strlen(tok) + 1))
				printf("Syntax error in \"%s\"\n", cpy);
		}
	}

	vec_destroy(&rules);

	if (live) 
		outfile = "/sys/fs/selinux/load";

	if (outfile) {
		int fd, ret;
		void *data = NULL;
		size_t len;
		policydb_to_image(NULL, policy, &data, &len);
		if (data == NULL) {
			fprintf(stderr, "Fail to dump policydb image!");
			return 1;
		}

		fd = open(outfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		if (fd < 0) {
			fprintf(stderr, "Can't open '%s':  %s\n",
			        outfile, strerror(errno));
			return 1;
		}
		ret = write(fd, data, len);
		close(fd);
		if (ret < 0) {
			fprintf(stderr, "Could not write policy to %s\n",
			        outfile);
			return 1;
		}
	}

	policydb_destroy(&policydb);
	return 0;
}
