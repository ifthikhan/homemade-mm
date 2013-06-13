/* mallocinterface */
int hmm_mm_init(void);
void *hmm_mm_malloc(size_t size);
void hmm_mm_free(void *bp);

/* Unused. Just to keep us compatible with the 15-213 malloc driver */
typedef struct {
    char *team;
    char *name1, *email1;
    char *name2, *email2;
} team_t;

extern team_t team;
