#include <stdbool.h>
#include <string.h>
#include "lambda_calc.h"

static bool beta_reduce(struct expr *expr);
static void substitute(const int var_letter, struct expr *replacement, /* in */ struct expr *expr);
static bool is_free_var(const int var_letter, /* in */ const struct expr *expr);

// returns true if fully reduced
static bool beta_reduce(struct expr *expr)
{
	struct expr *lambda;

	switch (expr->type) {
	case VAR:
		return true;
	case APP:
		if (expr->u.app.l->type != LAMBDA) {
			return beta_reduce(expr->u.app.l) &&
				beta_reduce(expr->u.app.r);
		}
		lambda = expr->u.app.l;
		substitute(lambda->u.lambda.param_letter,
				expr->u.app.r, lambda->u.lambda.body);
		return false;
	case LAMBDA:
		return beta_reduce(expr->u.lambda.body);
	}
}

// TODO: add calls to free()
static void substitute(const int var_letter, struct expr *replacement, /* in */ struct expr *expr)
{
	switch (expr->type) {
	case VAR:
		// [x/a] x = a
		if (expr->u.var.letter == var_letter) {
			memcpy(expr, replacement, sizeof(struct expr));
		}
		// [x/a] y = y
		return;
	case APP:
		// [x/a] (e e') = ([x/a] e) ([x/a] e')
		substitute(var_letter, replacement, expr->u.app.l);
		substitute(var_letter, replacement, expr->u.app.r);
		return;
	case LAMBDA:
		// [x/a] λx.e = λx.e
		if (expr->u.lambda.param_letter == var_letter) {
			return;
		}
		// [x/a] λy.e = λy.([x/a] e) if y is not a free var in a
		if (!is_free_var(expr->u.lambda.param_letter, replacement)) {
			substitute(var_letter, replacement, expr->u.lambda.body);
		} else {
			// TODO
		}
		return;
	}
}

static bool is_free_var(const int var_letter, /* in */ const struct expr *expr)
{
	switch (expr->type) {
	case VAR:
		return expr->u.var.letter == var_letter;
	case APP:
		return is_free_var(var_letter, expr->u.app.l) ||
			is_free_var(var_letter, expr->u.app.r);
	case LAMBDA:
		return expr->u.lambda.param_letter == var_letter
			? false
			: is_free_var(var_letter, expr->u.lambda.body);
	}
}
