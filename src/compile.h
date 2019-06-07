/* Compiler. Translation to assembler. */

#ifndef __COMPILE_H__
#define __COMPILE_H__

struct atom;

enum arch_type {
	ARCH_X86,
	ARCH_X86_64,
};

void atom_compile(struct atom *a, enum arch_type arch);

#endif /* __COMPILE_H__ */
