/* Building parse tree. */

#ifndef __C_H__
#define __C_H__

#include <stdbool.h>

enum atom_type {
	ATOM_INT,
	ATOM_BOOL,
	ATOM_ID,
	ATOM_COND,
	ATOM_SKIP,
	ATOM_IF,
	ATOM_WHILE,
	ATOM_EXPR,
	ATOM_ASSIGN,
	ATOM_LIST,
};

enum cond_type {
	COND_LESS,
	COND_EQUALS,
};

enum expr_type {
	EXPR_ADD,
	EXPR_MUL,
};

/* forward declaration */
struct atom;

struct atom_var {
	char     *av_name;
	unsigned  av_idx;
};

struct atom_cond {
	enum cond_type  ac_type;
	struct atom    *ac_lexpr;
	struct atom    *ac_rexpr;
};

struct atom_if {
	struct atom *ai_cond;
	struct atom *ai_then;
	struct atom *ai_else;
};

struct atom_while {
	struct atom *aw_cond;
	struct atom *aw_body;
};

struct atom_expr {
	enum expr_type  ae_type;
	struct atom    *ae_lexpr;
	struct atom    *ae_rexpr;
};

struct atom_assign {
	struct atom *aa_id;
	struct atom *aa_expr;
};

struct atom_list {
	struct atom      *al_cmd;
	struct atom_list *al_next;
};

struct atom {
	enum atom_type a_type;
	union {
		unsigned long       int_val;
		bool                bool_val;
		struct atom_var    *id;
		struct atom_cond    cond;
		struct atom_if      branch;
		struct atom_while   loop;
		struct atom_expr    expr;
		struct atom_assign  assign;
		struct atom_list   *list;
	} a;
};

int atom_init(void);
void atom_fini(void);

struct atom *atom_new(enum atom_type type);
void atom_free(struct atom *a);
void atom_destroy_tree(struct atom *a);

struct atom *atom_int(const char *val);
struct atom *atom_bool(const char *val);
struct atom *atom_id(const char *name);
struct atom *atom_cond(enum cond_type type, struct atom *left,
					    struct atom *right);
struct atom *atom_skip(void);
struct atom *atom_if(struct atom *cond, struct atom *then_body,
					struct atom *else_body);
struct atom *atom_while(struct atom *cond, struct atom *body);
struct atom *atom_expr(enum expr_type type, struct atom *left,
					    struct atom *right);
struct atom *atom_assign(struct atom *id, struct atom *expr);
struct atom *atom_list(struct atom *cmd);
struct atom *atom_list_add(struct atom *list, struct atom *cmd);

unsigned atom_var_nr(void);

void atom_stub(struct atom *a);

#endif /* __C_H__ */
