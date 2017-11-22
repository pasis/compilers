#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "c.h"

struct atom_var_l {
	struct atom_var    avl_var;
	struct atom_var_l *avl_next;
};

static struct atom_var_l *vars    = NULL;
static unsigned           vars_nr = 0;

static struct atom_var *atom_var_get(const char *name);
static void atom_var_list_destroy(void);

int atom_init(void)
{
	struct atom_var *result;

	/* Create special variable. "Result" holds return value. */
	result = atom_var_get("result");
	assert(result->av_idx == 0);

	return 0;
}

void atom_fini(void)
{
	atom_var_list_destroy();
}

static struct atom_var *atom_var_find(const char *name)
{
	struct atom_var_l *l;

	for (l = vars; l != NULL; l = l->avl_next)
		if (strcmp(l->avl_var.av_name, name) == 0)
			break;
	return l == NULL ? NULL : &l->avl_var;
}

static struct atom_var *atom_var_get(const char *name)
{
	struct atom_var   *var;
	struct atom_var_l *l;

	var = atom_var_find(name);
	if (var == NULL) {
		l = malloc(sizeof *l);
		assert(l != NULL);

		l->avl_var.av_idx  = vars_nr;
		l->avl_var.av_name = strdup(name);
		assert(l->avl_var.av_name != NULL);

		l->avl_next = vars;
		vars        = l;
		++vars_nr;
		var = &l->avl_var;
	}
	return var;
}

static void atom_var_list_destroy(void)
{
	struct atom_var_l *l;

	while (vars != NULL) {
		l = vars;
		vars = l->avl_next;
		free(l->avl_var.av_name);
		free(l);
	}
}

static struct atom_list *atom_list_new(struct atom *cmd)
{
	struct atom_list *l;

	l = malloc(sizeof *l);
	if (l != NULL)
		*l = (struct atom_list){ .al_cmd = cmd };
	assert(l != NULL);
	return l;
}

static void atom_list_destroy(struct atom_list *l)
{
	struct atom_list *next;

	while (l != NULL) {
		next = l->al_next;
		free(l);
		l = next;
	}
}

struct atom *atom_new(enum atom_type type)
{
	struct atom *a;

	a = malloc(sizeof *a);
	if (a != NULL)
		*a = (struct atom){ .a_type = type };
	assert(a != NULL);
	return a;
}

void atom_free(struct atom *a)
{
	switch (a->a_type) {
	case ATOM_LIST:
		atom_list_destroy(a->a.list);
		break;
	case ATOM_INT:
		/* fallthrough */
	case ATOM_BOOL:
		/* fallthrough */
	case ATOM_ID:
		/* fallthrough */
	case ATOM_COND:
		/* fallthrough */
	case ATOM_SKIP:
		/* fallthrough */
	case ATOM_IF:
		/* fallthrough */
	case ATOM_WHILE:
		/* fallthrough */
	case ATOM_EXPR:
		/* fallthrough */
	case ATOM_ASSIGN:
		break;
	}
	free(a);
}

void atom_destroy_tree(struct atom *a)
{
	struct atom_list *l;

	switch (a->a_type) {
	case ATOM_COND:
		atom_destroy_tree(a->a.cond.ac_lexpr);
		atom_destroy_tree(a->a.cond.ac_rexpr);
		break;
	case ATOM_IF:
		atom_destroy_tree(a->a.branch.ai_cond);
		atom_destroy_tree(a->a.branch.ai_then);
		atom_destroy_tree(a->a.branch.ai_else);
		break;
	case ATOM_WHILE:
		atom_destroy_tree(a->a.loop.aw_cond);
		atom_destroy_tree(a->a.loop.aw_body);
		break;
	case ATOM_EXPR:
		atom_destroy_tree(a->a.expr.ae_lexpr);
		atom_destroy_tree(a->a.expr.ae_rexpr);
		break;
	case ATOM_ASSIGN:
		atom_destroy_tree(a->a.assign.aa_id);
		atom_destroy_tree(a->a.assign.aa_expr);
		break;
	case ATOM_LIST:
		for (l = a->a.list; l != NULL; l = l->al_next)
			atom_destroy_tree(l->al_cmd);
		break;

	case ATOM_INT:
		/* fallthrough */
	case ATOM_BOOL:
		/* fallthrough */
	case ATOM_ID:
		/* fallthrough */
	case ATOM_SKIP:
		break;
	}
	atom_free(a);
}

struct atom *atom_int(const char *val)
{
	struct atom *a = atom_new(ATOM_INT);

	a->a.int_val = (unsigned long)strtoll(val, NULL, 10);

	return a;
}

struct atom *atom_bool(const char *val)
{
	struct atom *a = atom_new(ATOM_BOOL);

	a->a.bool_val = strcmp(val, "true") == 0;

	return a;
}

struct atom *atom_id(const char *name)
{
	struct atom *a = atom_new(ATOM_ID);

	a->a.id = atom_var_get(name);

	return a;
}

struct atom *atom_cond(enum cond_type type, struct atom *left,
					    struct atom *right)
{
	struct atom *a = atom_new(ATOM_COND);

	a->a.cond.ac_type  = type;
	a->a.cond.ac_lexpr = left;
	a->a.cond.ac_rexpr = right;

	return a;
}

struct atom *atom_skip(void)
{
	return atom_new(ATOM_SKIP);
}

struct atom *atom_if(struct atom *cond, struct atom *then_body,
					struct atom *else_body)
{
	struct atom *a = atom_new(ATOM_IF);

	a->a.branch.ai_cond = cond;
	a->a.branch.ai_then = then_body;
	a->a.branch.ai_else = else_body;

	return a;
}

struct atom *atom_while(struct atom *cond, struct atom *body)
{
	struct atom *a = atom_new(ATOM_WHILE);

	a->a.loop.aw_cond = cond;
	a->a.loop.aw_body = body;

	return a;
}

struct atom *atom_expr(enum expr_type type, struct atom *left,
					    struct atom *right)
{
	struct atom *a = atom_new(ATOM_EXPR);

	a->a.expr.ae_type  = type;
	a->a.expr.ae_lexpr = left;
	a->a.expr.ae_rexpr = right;

	return a;
}

struct atom *atom_assign(struct atom *id, struct atom *expr)
{
	struct atom *a = atom_new(ATOM_ASSIGN);

	a->a.assign.aa_id   = id;
	a->a.assign.aa_expr = expr;

	return a;
}

struct atom *atom_list(struct atom *cmd)
{
	struct atom *a = atom_new(ATOM_LIST);

	a->a.list = atom_list_new(cmd);

	return a;
}

struct atom *atom_list_add(struct atom *list, struct atom *cmd)
{
	struct atom_list *l;

	for (l = list->a.list; l->al_next != NULL; l = l->al_next);
	l->al_next = atom_list_new(cmd);

	return list;
}

unsigned atom_var_nr(void)
{
	return vars_nr;
}

void atom_stub(struct atom *a)
{
}
