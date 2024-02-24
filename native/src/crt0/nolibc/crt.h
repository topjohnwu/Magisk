/* SPDX-License-Identifier: LGPL-2.1 OR MIT */
/*
 * C Run Time support for NOLIBC
 * Copyright (C) 2023 Zhangjin Wu <falcon@tinylab.org>
 */

#ifndef _NOLIBC_CRT_H
#define _NOLIBC_CRT_H

char **environ;
const unsigned long *_auxv;
void _exit(int);
void __init_stdio(void);

typedef void init_func_t(int, char*[], char*[]);
typedef void fini_func_t(void);

extern init_func_t *__preinit_array_start[];
extern init_func_t *__preinit_array_end[];
extern init_func_t *__init_array_start[];
extern init_func_t *__init_array_end[];
extern fini_func_t *__fini_array_start[];
extern fini_func_t *__fini_array_end[];

static void call_array(init_func_t **start, init_func_t **end, int argc, char *argv[], char *envp[]) {
	unsigned long count = end - start;
	while (count-- > 0) {
		init_func_t* function = *start++;
		(*function)(argc, argv, envp);
	}
}

void __attribute__((used)) _start_c(long *sp)
{
	long argc;
	char **argv;
	char **envp;
	const unsigned long *auxv;
	/* silence potential warning: conflicting types for 'main' */
	int _nolibc_main(int, char **, char **) __asm__ ("main");

	/*
	 * sp  :    argc          <-- argument count, required by main()
	 * argv:    argv[0]       <-- argument vector, required by main()
	 *          argv[1]
	 *          ...
	 *          argv[argc-1]
	 *          null
	 * environ: environ[0]    <-- environment variables, required by main() and getenv()
	 *          environ[1]
	 *          ...
	 *          null
	 * _auxv:   _auxv[0]      <-- auxiliary vector, required by getauxval()
	 *          _auxv[1]
	 *          ...
	 *          null
	 */

	/* assign argc and argv */
	argc = *sp;
	argv = (void *)(sp + 1);

	/* find environ */
	environ = envp = argv + argc + 1;

	/* find _auxv */
	for (auxv = (void *)envp; *auxv++;)
		;
	_auxv = auxv;

	/* call preinit and init */
    __init_stdio();
	call_array(__preinit_array_start, __preinit_array_end, argc, argv, envp);
	call_array(__init_array_start, __init_array_end, argc, argv, envp);

	/* go to application */
	_exit(_nolibc_main(argc, argv, envp));
}

#endif /* _NOLIBC_CRT_H */
