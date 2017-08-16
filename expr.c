#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "lambda_calc.h"

static void expr_print__(const Expr *expr, const bool should_paren_app);

Expr *expr_dup(const Expr *expr)
{
	Expr *dup;

	dup = expr_dup_no_recurse(expr);
	switch (expr->type) {
	case VAR:
		break;
	case APP:
		dup->u.app.l = expr_dup(expr->u.app.l);
		dup->u.app.r = expr_dup(expr->u.app.r);
		break;
	case LAMBDA:
		dup->u.lambda.body = expr_dup(expr->u.lambda.body);
		break;
	}
	return dup;
}

Expr *expr_dup_no_recurse(const Expr *expr)
{
	return memcpy(new(Expr), expr, sizeof(Expr));
}

void expr_free(Expr *expr)
{
	switch (expr->type)
	{
	case VAR:
		break;
	case APP:
		expr_free(expr->u.app.l);
		expr_free(expr->u.app.r);
		break;
	case LAMBDA:
		expr_free(expr->u.lambda.body);
		break;
	}
	free(expr);
}

void expr_print(const Expr *expr)
{
	expr_print__(expr, false);
	putchar('\n');
}

static void expr_print__(const Expr *expr, const bool should_paren_app)
{
	switch (expr->type) {
	case VAR:
		putchar(expr->u.var.letter);
		break;
	case APP:
		if (should_paren_app) {
			putchar('(');
		}
		expr_print__(expr->u.app.l, false);
		expr_print__(expr->u.app.r, true);
		if (should_paren_app) {
			putchar(')');
		}
		break;
	case LAMBDA:
		printf("λ%c.", expr->u.lambda.param_letter);
		expr_print__(expr->u.lambda.body, true);
		break;
	}
}