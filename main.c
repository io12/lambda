#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "lambda_calc.h"

void *xmalloc(const size_t size)
{
	void *p;

	p = malloc(size);
	if (p == NULL) {
		panic("out of memory");
	}
	return p;
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
	Expr *expr, *old_expr;

	for (;;) {
		expr = parse_line();
		while (!eval_done(expr)) {
			expr_print(expr);
			old_expr = expr;
			expr = beta_reduce(expr);
			expr_free(old_expr);
		}
		expr_print(expr);
		expr_free(expr);
	}
}
