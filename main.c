#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "lambda_calc.h"

static void free_expr(Expr *expr);
static void print_expr(const Expr *expr);
static void print_expr_1(const Expr *expr, const bool should_paren_app);

void *xmalloc(const size_t size)
{
	void *p;

	p = malloc(size);
	if (p == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	return p;
}

static void free_expr(Expr *expr)
{
	switch (expr->type)
	{
	case VAR:
		break;
	case APP:
		free_expr(expr->u.app.l);
		free_expr(expr->u.app.r);
		break;
	case LAMBDA:
		free_expr(expr->u.lambda.body);
		break;
	}
	free(expr);
}

static void print_expr(const Expr *expr)
{
	print_expr_1(expr, false);
	putchar('\n');
}

static void print_expr_1(const Expr *expr, const bool should_paren_app)
{
	switch (expr->type) {
	case VAR:
		putchar(expr->u.var.letter);
		break;
	case APP:
		if (should_paren_app) {
			putchar('(');
		}
		print_expr_1(expr->u.app.l, false);
		print_expr_1(expr->u.app.r, true);
		if (should_paren_app) {
			putchar(')');
		}
		break;
	case LAMBDA:
		printf("λ%c.", expr->u.lambda.param_letter);
		print_expr_1(expr->u.lambda.body, true);
		break;
	}
}

NORETURN void panic(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
	exit(EXIT_FAILURE);
}

int main(void)
{
	Expr *expr;

	for (;;) {
		expr = parse_line();
		print_expr(expr);
		free_expr(expr);
	}
}
