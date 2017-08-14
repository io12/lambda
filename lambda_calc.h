#ifdef __GNUC__
#define NORETURN __attribute__((noreturn))
#else
#define NORETURN
#endif

#define TOK_TO_STR(X) (X == EOF ? "EOF" : (char[]){'\'', X, '\'', '\0'})

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

int next_tok(void);
int peek_tok(void);
void expect_tok(const int expected_tok);
void *xmalloc(const size_t size);
NORETURN void panic(const char *fmt, ...);
Expr *parse_line(void);
