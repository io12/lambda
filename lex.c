#include "lambda_calc.h"

static int peeked_tok = 0;

static char *read_input(void)
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

static int next_char(void)
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
	if ((uint8_t)s[n] == 0xce && (uint8_t)s[n + 1] == 0xbb) { // UTF-8 'λ' U+03bb
		n += 2;
		return '\\';
	}
	return s[n++];
}

static int next_nonspace_char(void)
{
	int c;

	do {
		c = next_char();
	} while (isspace(c));
	return c;
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
