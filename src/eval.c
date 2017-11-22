#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c.h"
#include "eval.h"

static unsigned long *memory;

static unsigned long eval_cond(enum cond_type type, unsigned long left,
						    unsigned long right)
{
	switch (type) {
	case COND_LESS:
		return (unsigned long)(left < right);
	case COND_EQUALS:
		return (unsigned long)(left == right);
	}

	assert(0);
	return 0;
}

static unsigned long eval_expr(enum expr_type type, unsigned long left,
						    unsigned long right)
{
	switch (type) {
	case EXPR_ADD:
		return left + right;
	case EXPR_MUL:
		return left * right;
	}

	assert(0);
	return 0;
}

static unsigned long eval_rec(struct atom *a)
{
	struct atom_list *l;
	unsigned long     left;
	unsigned long     right;

	switch (a->a_type) {
	case ATOM_INT:
		return a->a.int_val;
	case ATOM_BOOL:
		return (unsigned long)a->a.bool_val;
	case ATOM_ID:
		return memory[a->a.id->av_idx];
	case ATOM_COND:
		left  = eval_rec(a->a.cond.ac_lexpr);
		right = eval_rec(a->a.cond.ac_rexpr);
		return eval_cond(a->a.cond.ac_type, left, right);
	case ATOM_SKIP:
		/* Do nothing. */
		return 0;
	case ATOM_IF:
		if (eval_rec(a->a.branch.ai_cond))
			return eval_rec(a->a.branch.ai_then);
		else
			return eval_rec(a->a.branch.ai_else);
	case ATOM_WHILE:
		while (eval_rec(a->a.loop.aw_cond))
			(void)eval_rec(a->a.loop.aw_body);
		return 0;
	case ATOM_EXPR:
		left  = eval_rec(a->a.expr.ae_lexpr);
		right = eval_rec(a->a.expr.ae_rexpr);
		return eval_expr(a->a.expr.ae_type, left, right);
	case ATOM_ASSIGN:
		memory[a->a.assign.aa_id->a.id->av_idx] =
						eval_rec(a->a.assign.aa_expr);
		return 0;
	case ATOM_LIST:
		for (l = a->a.list; l != NULL; l = l->al_next)
			(void)eval_rec(l->al_cmd);
		return 0;
	}

	assert(0);
	return 0;
}

void atom_eval(struct atom *a)
{
	unsigned var_nr = atom_var_nr();

	memory = malloc(sizeof *memory * var_nr);
	assert(memory != NULL);
	memset(memory, 0, sizeof *memory * var_nr);

	(void)eval_rec(a);
	printf("result = %lu\n", memory[0]);

	free(memory);
}
