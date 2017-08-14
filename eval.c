#include <stdbool.h>
#include <string.h>
#include "lambda_calc.h"

static bool beta_reduce(Expr *expr);
static void substitute(const int var_letter, /* for */ const Expr *replacement, /* in */ Expr *expr);
static bool is_free_var(const int var_letter, /* in */ const Expr *expr);
static int find_unused_or_bound_var_in(const Expr *expr);

// Returns true if already reduced
static bool beta_reduce(Expr *expr)
{
	Expr *lambda;

	switch (expr->type) {
	case VAR:
		return true;
	case APP:
		if (expr->u.app.l->type != LAMBDA) {
			return beta_reduce(expr->u.app.l) &&
				beta_reduce(expr->u.app.r);
		}
		lambda = expr->u.app.l;
		// TODO
		substitute(lambda->u.lambda.param_letter, /* for */
				expr->u.app.r, /* in */ lambda->u.lambda.body);
		return false;
	case LAMBDA:
		return beta_reduce(expr->u.lambda.body);
	}
}

// TODO: add calls to free()
static void substitute(const int var_letter, /* for */ const Expr *replacement, /* in */ Expr *expr)
{
	switch (expr->type) {
	case VAR:
		// [x/a] x = a
		if (expr->u.var.letter == var_letter) {
			memcpy(expr, replacement, sizeof(Expr));
		}
		// [x/a] y = y
		return;
	case APP:
		// [x/a] (e e') = ([x/a] e) ([x/a] e')
		substitute(var_letter, /* for */ replacement, /* in */ expr->u.app.l);
		substitute(var_letter, /* for */ replacement, /* in */ expr->u.app.r);
		return;
	case LAMBDA:
		// [x/a] λx.e = λx.e
		if (expr->u.lambda.param_letter == var_letter) {
			return;
		}
		// Alpha-conversion (capture-avoiding substitution)
		if (is_free_var(expr->u.lambda.param_letter, replacement)) {
			// TODO
			const int replacement_letter = find_unused_or_bound_var_in(replacement);
			Expr *replacement_var = new(Expr);
			replacement_var->type = VAR;
			replacement_var->u.var.letter = replacement_letter;
			substitute(expr->u.lambda.param_letter, /* for */
					replacement_var, /* in */ expr->u.lambda.body);
			expr->u.lambda.param_letter = replacement_letter;
		}
		// [x/a] λy.e = λy.([x/a] e) if y is not a free var in a
		substitute(var_letter, /* for */ replacement, /* in */ expr->u.lambda.body);
		return;
	}
}

static bool is_free_var(const int var_letter, /* in */ const Expr *expr)
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
