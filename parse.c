#include <ctype.h>
#include <stdio.h>
#include "lambda_calc.h"

#define new(type) ((type *) xmalloc(sizeof(type)))

static Expr *parse_app(const int end_tok);
static Expr *parse_term(void);
static Expr *parse_paren_expr(void);
static Expr *parse_lambda(void);
static Expr *parse_var(void);

Expr *parse_line(void)
{
	return parse_app(EOF);
}

static Expr *parse_app(const int end_tok)
{
	Expr *expr, *super_expr;

	expr = parse_term();
	for (;;) {
		if (peek_tok() == end_tok) {
			next_tok();
			return expr;
		}
		super_expr = new(Expr);
		super_expr->type = APP;
		super_expr->u.app.l = expr;
		super_expr->u.app.r = parse_term();
		expr = super_expr;
	}
}

static Expr *parse_term(void)
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

static Expr *parse_paren_expr(void)
{
	expect_tok('(');
	return parse_app(')');
}

static Expr *parse_lambda(void)
{
	int param_letter;
	Expr *expr;

	expect_tok('\\');
	param_letter = next_tok();
	if (!isalpha(param_letter)) {
		panic("expected a letter, instead recieved %s", TOK_TO_STR(param_letter));
	}
	expect_tok('.');
	expr = new(Expr);
	expr->type = LAMBDA;
	expr->u.lambda.param_letter = param_letter;
	expr->u.lambda.body = parse_term();
	return expr;
}

static Expr *parse_var(void)
{
	int var_letter;
	Expr *expr;

	var_letter = next_tok();
	if (!isalpha(var_letter)) {
		panic("expected a letter, instead recieved %s", TOK_TO_STR(var_letter));
	}
	expr = new(Expr);
	expr->type = VAR;
	expr->u.var.letter = var_letter;
	return expr;
}
