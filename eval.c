#include "lambda_calc.h"

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
static void substitute(int var_letter, struct expr *replacement, struct expr *expr)
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
		// [x/a] λy.e = λy.([x/a] e)
		substitute(var_letter, replacement, expr->u.lambda.body); // TODO: fv
		return;
	default:
		panic("internal error"); // TODO: Is this needed?
	}
}
