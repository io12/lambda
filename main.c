#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "lambda_calc.h"

#ifdef __GNUC__
#define NORETURN __attribute__((noreturn))
#else
#define NORETURN
#endif

#define new(type) ((type *) xmalloc(sizeof(type)))

#define TOK_TO_STR(X) (X == EOF ? "EOF" : (char[]){'\'', X, '\'', '\0'})

struct expr {
	enum { VAR, APP, LAMBDA } type;
	union {
		struct {
			int letter;
		} var;
		struct {
			struct expr *l, *r;
		} app;
		struct {
			int param_letter;
			struct expr *body;
		} lambda;
	} u;
};

struct expr *parse_line(void)
{
	return parse_app(EOF);
}

struct expr *parse_app(const int end_tok)
{
	struct expr *expr, *super_expr;

	expr = parse_term();
	for (;;) {
		if (peek_tok() == end_tok) {
			next_tok();
			return expr;
		}
		super_expr = new(struct expr);
		super_expr->type = APP;
		super_expr->u.app.l = expr;
		super_expr->u.app.r = parse_term();
		expr = super_expr;
	}
}

struct expr *parse_term(void)
{
	int tok;

	tok = peek_tok();
	switch (tok) {
	case '(':
		return parse_paren_expr();
	case '\\':
		return parse_lambda();
	}
	if (isalpha(tok)) {
		return parse_var();
	}
	panic("stray token %s", TOK_TO_STR(tok));
}

struct expr *parse_paren_expr(void)
{
	expect_tok('(');
	return parse_app(')');
}

struct expr *parse_lambda(void)
{
	int param_letter;
	struct expr *expr;

	expect_tok('\\');
	param_letter = next_tok();
	if (!isalpha(param_letter)) {
		panic("expected a letter, instead recieved %s", TOK_TO_STR(param_letter));
	}
	expect_tok('.');
	expr = new(struct expr);
	expr->type = LAMBDA;
	expr->u.lambda.param_letter = param_letter;
	expr->u.lambda.body = parse_term();
	return expr;
}

struct expr *parse_var(void)
{
	int var_letter;
	struct expr *expr;

	var_letter = next_tok();
	if (!isalpha(var_letter)) {
		panic("expected a letter, instead recieved %s", TOK_TO_STR(var_letter));
	}
	expr = new(struct expr);
	expr->type = VAR;
	expr->u.var.letter = var_letter;
	return expr;
}

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

void free_expr(struct expr *expr)
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
	default:
		panic("internal error");
	}
	free(expr);
}

void print_expr(const struct expr *expr)
{
	print_expr_1(expr, false);
	putchar('\n');
}

void print_expr_1(const struct expr *expr, const bool in_r_app)
{
	switch (expr->type) {
	case VAR:
		putchar(expr->u.var.letter);
		break;
	case APP:
		if (in_r_app) {
			putchar('(');
		}
		print_expr_1(expr->u.app.l, false);
		print_expr_1(expr->u.app.r, true);
		if (in_r_app) {
			putchar(')');
		}
		break;
	case LAMBDA:
		printf("λ%c.", expr->u.lambda.param_letter);
		print_expr_1(expr->u.lambda.body, true);
		break;
	default:
		panic("internal error");
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
	struct expr *expr;

	for (;;) {
		expr = parse_line();
		print_expr(expr);
		free_expr(expr);
	}
}
