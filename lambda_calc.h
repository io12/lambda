#ifdef __GNUC__
#define NORETURN __attribute__((noreturn))
#else
#define NORETURN
#endif

#define TOK_TO_STR(X) \
	(X == '\n' \
		? "<END OF LINE>" \
		: (char[]){'\'', X, '\'', '\0'})

#define new(type) ((type *) xmalloc(sizeof(type)))

typedef struct expr {
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
} Expr;

void *xmalloc(const size_t size);
NORETURN void panic(const char *fmt, ...);
int next_tok(void);
int peek_tok(void);
void expect_tok(const int expected_tok);
Expr *parse_line(void);
Expr *expr_dup(const Expr *expr);
Expr *expr_dup_no_recurse(const Expr *expr);
void expr_free(Expr *expr);
void expr_print(const Expr *expr);
bool eval_done(const Expr *expr);
Expr *beta_reduce(const Expr *expr);
