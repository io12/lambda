#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>

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

char *read_input(void);
int next_char(void);
int next_tok(void);
int peek_tok(void);
void expect_tok(const int expected_tok);
int next_nonspace_char(void);
struct expr *parse_line(void);
struct expr *parse_app(const int end_tok);
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
	add_history(input);
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
		s = NULL;
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
	panic("invalid token %s", TOK_TO_STR(c));
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
		panic("expected %s, instead got %s",
				TOK_TO_STR(expected_tok), TOK_TO_STR(recieved_tok));
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

struct expr *parse_line(void)
{
	return parse_app(EOF);
}

struct expr *parse_app(const int end_tok)
{
	struct expr *expr, *super_expr;

	expr = parse_expr();
	if (peek_tok() == end_tok) {
		next_tok();
		return expr;
	}
	super_expr = new(struct expr);
	super_expr->type = APP;
	super_expr->u.app.l = expr;
	super_expr->u.app.r = parse_app(end_tok);
	return super_expr;
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
	expr->u.lambda.body = parse_expr();
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

void print_expr(struct expr *expr)
{
	switch (expr->type) {
	case VAR:
		putchar(expr->u.var.letter);
		break;
	case APP:
		// TODO: need parens
		print_expr(expr->u.app.l);
		print_expr(expr->u.app.r);
		break;
	case LAMBDA:
		printf("λ%c.", expr->u.lambda.param_letter);
		print_expr(expr->u.lambda.body);
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
		putchar('\n');
		free_expr(expr);
	}
}
