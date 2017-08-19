#include <stdbool.h>
#include <stddef.h>
#include "lambda_calc.h"

static Expr *subst(const int var, /* for */ const Expr *subst,
		/* in */ const Expr *expr);
static bool is_free_var(const int var, /* in */ const Expr *expr);
static int find_unused_or_bound_var_in(const Expr *expr);

bool eval_done(const Expr *expr)
{
	switch (expr->type) {
	case VAR:
		return true;
	case APP:
		return expr->u.app.l->type != LAMBDA &&
			eval_done(expr->u.app.l) && eval_done(expr->u.app.r);
	case LAMBDA:
		return eval_done(expr->u.lambda.body);
	}
	panic("internal error"); // NOTREACHED (silence gcc warning)
}

Expr *beta_reduce(const Expr *expr)
{
	Expr *result;

	switch (expr->type) {
	case VAR:
		// x β-> x
		result = expr_dup_no_recurse(expr);
		break;
	case APP:
		if (expr->u.app.l->type != LAMBDA) {
			result = expr_dup_no_recurse(expr);
			result->u.app.l = beta_reduce(expr->u.app.l);
			result->u.app.r = beta_reduce(expr->u.app.r);
			break;
		}
		// (λx.a)y β-> [x/y]a
		const Expr *lambda = expr->u.app.l;
		result = subst(lambda->u.lambda.var, /* for */
				expr->u.app.r, /* in */ lambda->u.lambda.body);
		break;
	case LAMBDA:
		result = expr_dup_no_recurse(expr);
		result->u.lambda.body = beta_reduce(expr->u.lambda.body);
		break;
	}
	return result;
}

static Expr *subst(const int var, /* for */ const Expr *subst_expr,
		/* in */ const Expr *expr)
{
	Expr *result;

	switch (expr->type) {
	case VAR:
		// [x/a] x = a
		if (expr->u.var == var) {
			result = expr_dup(subst_expr);
			break;
		}
		// [x/a] y = y
		result = expr_dup(expr);
		break;
	case APP:
		// [x/a] (e e') = ([x/a] e) ([x/a] e')
		result = expr_dup_no_recurse(expr);
		result->u.app.l = subst(var, /* for */ subst_expr,
				/* in */ expr->u.app.l);
		result->u.app.r = subst(var, /* for */ subst_expr,
				/* in */ expr->u.app.r);
		break;
	case LAMBDA:
		// [x/a] λx.e = λx.e
		if (expr->u.lambda.var == var) {
			result = expr_dup(expr);
			break;
		}
		result = expr_dup_no_recurse(expr);
		// Alpha-conversion (capture-avoiding substitution)
		if (is_free_var(expr->u.lambda.var, subst_expr)) {
			const int subst_letter =
				find_unused_or_bound_var_in(subst_expr);
			Expr *subst_var = new(Expr);
			subst_var->type = VAR;
			subst_var->u.var = subst_letter;
			result->u.lambda.var = subst_letter;
			result->u.lambda.body = subst(expr->u.lambda.var,
					/* for */ subst_var,
					/* in */ expr->u.lambda.body);
		}
		// [x/a] λy.e = λy.([x/a] e) if y is not a free var in a
		result->u.lambda.body = subst(var, /* for */ subst_expr,
				/* in */ expr->u.lambda.body);
		break;
	}
	return result;
}

static bool is_free_var(const int var, /* in */ const Expr *expr)
{
	switch (expr->type) {
	case VAR:
		return expr->u.var == var;
	case APP:
		return is_free_var(var, expr->u.app.l) ||
			is_free_var(var, expr->u.app.r);
	case LAMBDA:
		return expr->u.lambda.var == var
			? false
			: is_free_var(var, expr->u.lambda.body);
	}
	panic("internal error"); // NOTREACHED (silence gcc warning)
}

static int find_unused_or_bound_var_in(const Expr *expr)
{
	int letter;

	for (letter = 'a'; letter <= 'z'; letter++) {
		if (!is_free_var(letter, expr)) {
			return letter;
		}
	}
	panic("no unused or bound vars available");
}
