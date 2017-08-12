#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>

#ifdef __GNUC__
#define noreturn __attribute__((noreturn))
#else
#define noreturn
#endif

#define new(type) (type *) xmalloc(sizeof(type))

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

char *read_input(void);
int next_char(void);
int next_tok(void);
int peek_tok(void);
void expect_tok(const int expected_tok);
int next_nonspace_char(void);
struct expr *parse_expr(void);
struct expr *parse_paren_expr(void);
struct expr *parse_lambda(void);
struct expr *parse_var(void);
void *xmalloc(const size_t size);
void panic(const char *fmt, ...);

static int peeked_tok = 0;

char *read_input(void)
{
	char *input;

	for (;;) {
		input = readline("(λ)-> ");
		if (input == NULL) {
			putchar('\n');
			exit(EXIT_SUCCESS);
		}
		if (input[0] != '\0') {
			break;
		}
		free(input);
	}
	return input;
}

int next_char(void)
{
	static char *s = NULL;
	static int n = 0;

	if (s == NULL) {
		s = read_input();
	}
	if (s[n] == '\0') {
		free(s);
		s = read_input();
		n = 0;
		return EOF;
	}
	return s[n++];
}

int next_tok(void)
{
	int c, tok;

	if (peeked_tok != 0) {
		tok = peeked_tok;
		peeked_tok = 0;
		return tok;
	}
	c = next_nonspace_char();
	if (c == EOF || c == '.' || c == '\\' || c == '(' || c == ')' || isalpha(c)) {
		return c;
	}
	panic("invalid token '%c'", c);
}

int peek_tok(void)
{
	int tok;

	if (peeked_tok != 0) {
		return peeked_tok;
	}
	tok = next_tok();
	peeked_tok = tok;
	return tok;
}

void expect_tok(const int expected_tok)
{
	int recieved_tok;

	recieved_tok = next_tok();
	if (expected_tok != recieved_tok) {
		panic("expected '%c', instead got '%c'", expected_tok, recieved_tok);
	}
}

int next_nonspace_char(void)
{
	int c;

	do {
		c = next_char();
	} while (isspace(c));
	return c;
}

struct expr *parse_expr(void)
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
	panic("stray token '%c'", tok);
}

struct expr *parse_paren_expr(void)
{
	struct expr *expr, *super_expr;

	expect_tok('(');
	for (;;) {
		expr = parse_expr();
		if (peek_tok() == ')') {
			next_tok();
			break;
		}
		super_expr = new(struct expr);
		super_expr->type = APP;
		super_expr->u.app.l = expr;
		super_expr->u.app.r = parse_expr();
		expr = super_expr;
	}
	return expr;
}

struct expr *parse_lambda(void)
{
	int param_letter;
	struct expr *expr;

	expect_tok('\\');
	param_letter = next_tok();
	if (!isalpha(param_letter)) {
		panic("expected a letter, instead recieved '%c'", param_letter);
	}
	expect_tok('.');
	expr = new(struct expr);
	expr->type = LAMBDA;
	expr->u.lambda.param_letter = param_letter;
	expr->u.lambda.body = parse_expr();
	return expr;
}

struct expr *parse_var(void)
{
	int var_letter;
	struct expr *expr;

	var_letter = next_tok();
	if (!isalpha(var_letter)) {
		panic("expected a letter, instead recieved '%c'", var_letter);
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

noreturn void panic(const char *fmt, ...)
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
	for (;;) {
		parse_expr();
	}
}
