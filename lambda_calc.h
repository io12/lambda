int next_tok(void);
int peek_tok(void);
void expect_tok(const int expected_tok);
void *xmalloc(const size_t size);
void free_expr(struct expr *expr);
void print_expr(const struct expr *expr);
void panic(const char *fmt, ...);
struct expr *parse_line(void);
