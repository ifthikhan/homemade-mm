/*
 * Common utility functions
 */

/*
 * Prints an error message to stderr and exits
 */
void unix_error(char *msg) {

    fprintf(stderr, "%s: %s", msg, strerror(errno));
    exit(0);
}
