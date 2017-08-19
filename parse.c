#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include "lambda_calc.h"

static Expr *parse_app(void);
static Expr *parse_non_app(void);
static Expr *parse_paren(void);
static Expr *parse_lambda(void);
static Expr *parse_var(void);

Expr *parse_line(void)
{
	Expr *expr;

	expr = parse_app();
	expect_tok('\n');
	return expr;
}

static Expr *parse_app(void)
{
	Expr *expr, *super_expr;

	expr = parse_non_app();
	for (;;) {
		if (peek_tok() == ')' || peek_tok() == '\n') {
			return expr;
		}
		super_expr = new(Expr);
		super_expr->type = APP;
		super_expr->u.app.l = expr;
		super_expr->u.app.r = parse_non_app();
		expr = super_expr;
	}
}

static Expr *parse_non_app(void)
{
	int tok;

	tok = peek_tok();
	switch (tok) {
	case '(':
		return parse_paren();
	case '\\':
		return parse_lambda();
	}
	if (isalpha(tok)) {
		return parse_var();
	}
	panic("stray token %s", TOK_TO_STR(tok));
}

static Expr *parse_paren(void)
{
	Expr *expr;

	expect_tok('(');
	expr = parse_app();
	expect_tok(')');
	return expr;
}

static Expr *parse_lambda(void)
{
	int var;
	Expr *expr;

	expect_tok('\\');
	var = next_tok();
	if (!isalpha(var)) {
		panic("expected a letter, instead recieved %s", TOK_TO_STR(var));
	}
	expect_tok('.');
	expr = new(Expr);
	expr->type = LAMBDA;
	expr->u.lambda.var = var;
	expr->u.lambda.body = parse_app();
	return expr;
}

static Expr *parse_var(void)
{
	int var;
	Expr *expr;

	var = next_tok();
	if (!isalpha(var)) {
		panic("expected a letter, instead recieved %s", TOK_TO_STR(var));
	}
	expr = new(Expr);
	expr->type = VAR;
	expr->u.var = var;
	return expr;
}
