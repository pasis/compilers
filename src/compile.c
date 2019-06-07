#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "c.h"
#include "compile.h"

static const char *ident = "\t";
static const char *vars_label = "vars";

static void putline(FILE *f, const char *cmd, const char *comment)
{
	fprintf(f, "%s", ident);
	if (cmd != NULL) {
		fprintf(f, "%s", cmd);
		if (comment != NULL)
			fprintf(f, "\t");
	}
	if (comment != NULL)
		fprintf(f, "; %s", comment);
	fprintf(f, "\n");
}

static void putcmd(FILE *f, const char *cmd)
{
	putline(f, cmd, NULL);
}

static void putcomment(FILE *f, const char *comment)
{
	putline(f, NULL, comment);
}

static void putlabel(FILE *f, const char *label)
{
	fprintf(f, "%s:\n", label);
}

static const char *getlabel(const char *base, void *ptr)
{
	static char buf[64];

	snprintf(buf, sizeof buf, "%s_%p", base, ptr);
	return buf;
}

static void comp_header_x86_64(FILE *f)
{
	fprintf(f, "format ELF64 executable 3\n\n");
	fprintf(f, "segment readable executable\n");
	fprintf(f, "entry $\n\n");
}

static void comp_result_x86_64(FILE *f)
{
	putlabel(f, "result_to_string");
	putcmd(f, "mov rax, qword [vars]");
	putcmd(f, "lea rsi, [result + 28]");
	putlabel(f, ".loop");
	putcmd(f, "xor rdx, rdx");
	putcmd(f, "mov rbx, 10");
	putcmd(f, "div rbx");
	putcmd(f, "add rdx, '0'");
	putcmd(f, "mov byte [rsi], dl");
	putcmd(f, "dec rsi");
	putcmd(f, "test rax, rax");
	putcmd(f, "jnz .loop");
	putcmd(f, "ret");
}

static void comp_footer_x86_64(FILE *f)
{
	unsigned vars_nr = atom_var_nr();

	putcomment(f, "print value of the special variable \"result\"");
	putcmd(f, "call result_to_string");
	putcmd(f, "mov edx, result_size");
	putcmd(f, "lea rsi, [result]");
	putcmd(f, "mov edi, 1");
	putcmd(f, "mov eax, 1");
	putcmd(f, "syscall");

	putcomment(f, "sys_exit");
	putcmd(f, "xor edi, edi");
	putcmd(f, "mov eax, 60");
	putcmd(f, "syscall");

	fprintf(f, "\n");
	comp_result_x86_64(f);

	fprintf(f, "\n");
	fprintf(f, "segment readable writable\n");
	putlabel(f, vars_label);
	fprintf(f, "%stimes (%lu) db 0\n", ident, vars_nr * sizeof(uint64_t));
	putlabel(f, "result");
	putcmd(f, "db 'result =                     ', 0xa");
	putcmd(f, "result_size = $ - result");
}

static void comp_rec(FILE *f, struct atom *a)
{
	struct atom_list *l;
	const char       *label;

	switch (a->a_type) {
	case ATOM_INT:
		fprintf(f, "mov rax, %lu\n", a->a.int_val);
		break;
	case ATOM_BOOL:
		fprintf(f, "mov rax, %lu\n", (unsigned long)a->a.bool_val);
		break;
	case ATOM_ID:
		fprintf(f, "mov rax, qword [%s + %lu]\n",
			vars_label, a->a.id->av_idx * sizeof(uint64_t));
		break;
	case ATOM_COND:
		comp_rec(f, a->a.cond.ac_lexpr);
		putcmd(f, "push rax");
		comp_rec(f, a->a.cond.ac_rexpr);
		putcmd(f, "mov rdx, rax");
		putcmd(f, "pop rbx");
		putcmd(f, "xor rax, rax");
		putcmd(f, "sub rbx, rdx");
		switch (a->a.cond.ac_type) {
		case COND_LESS:
			putcmd(f, "adc rax, 0");
			break;
		case COND_EQUALS:
			label = getlabel("cond", a);
			fprintf(f, "jnz %s\n", label);
			putcmd(f, "mov rax, 1");
			putlabel(f, label);
			break;
		}
		break;
	case ATOM_SKIP:
		/* Do nothing */
		break;
	case ATOM_IF:
		comp_rec(f, a->a.branch.ai_cond);
		label = getlabel("else", a);
		putcmd(f, "test rax, rax");
		fprintf(f, "jz %s\n", label);
		comp_rec(f, a->a.branch.ai_then);
		label = getlabel("fi", a);
		fprintf(f, "jmp %s\n", label);
		label = getlabel("else", a);
		putlabel(f, label);
		comp_rec(f, a->a.branch.ai_else);
		label = getlabel("fi", a);
		putlabel(f, label);
		break;
	case ATOM_WHILE:
		label = getlabel("while", a);
		putlabel(f, label);
		comp_rec(f, a->a.loop.aw_cond);
		putcmd(f, "test rax, rax");
		label = getlabel("done", a);
		fprintf(f, "jz %s\n", label);
		comp_rec(f, a->a.loop.aw_body);
		label = getlabel("while", a);
		fprintf(f, "jmp %s\n", label);
		label = getlabel("done", a);
		putlabel(f, label);
		break;
	case ATOM_EXPR:
		comp_rec(f, a->a.expr.ae_lexpr);
		putcmd(f, "push rax");
		comp_rec(f, a->a.expr.ae_rexpr);
		putcmd(f, "pop rbx");
		switch (a->a.expr.ae_type) {
		case EXPR_ADD:
			putcmd(f, "add rax, rbx");
			break;
		case EXPR_MUL:
			putcmd(f, "mul rbx");
			break;
		}
		break;
	case ATOM_ASSIGN:
		comp_rec(f, a->a.assign.aa_expr);
		fprintf(f, "mov qword [%s + %lu], rax\n", vars_label,
			a->a.assign.aa_id->a.id->av_idx * sizeof(uint64_t));
		break;
	case ATOM_LIST:
		for (l = a->a.list; l != NULL; l = l->al_next)
			comp_rec(f, l->al_cmd);
		break;
	}
}

void atom_compile(struct atom *a, enum arch_type arch)
{
	assert(arch == ARCH_X86_64);

	comp_header_x86_64(stdout);
	comp_rec(stdout, a);
	comp_footer_x86_64(stdout);
}
