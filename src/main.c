#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>


/* - Jumper -
 *  Hop from process to process while preserving state
 *  Challenge: try to kill() it without referencing the process name
 */

#define HOP_INTERVAL 1 /* in seconds */

typedef void (*sighandler_t)(int);

static int log_fd;
static char *argv0;


/* Logger printf replacement */
void s_printnf(char *fmt, ...)
{
    char prefix[512 + sizeof("[00000] ")];
    char buf[512];
    va_list args;

    snprintf(prefix, sizeof(prefix), "[%05d] ", getpid());

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    strncat(prefix, buf, sizeof(prefix));

    write(log_fd, prefix, strlen(prefix));
    write(STDOUT_FILENO, prefix, strlen(prefix));

    fsync(log_fd);
}

void init_handlers(sighandler_t alrm)
{
    signal(SIGALRM, alrm);

    /* Ignore all signals that ask us to stop - we don't care */
    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGXCPU, SIG_IGN);

    alarm(HOP_INTERVAL);
}

void rename_argv0()
{
    int i, len = strlen(argv0);

    /* Randomly lowercase and uppercase - this only effects what ps displays */
    for (i = 0; i < len; i++) {
        argv0[i] = rand() % 26 + 'A';
        argv0[i] += rand() & 2 ? 'a' - 'A' : 0;
    }
}

/* Signal handler for SIGALRM it receives every few seconds */
void alrm_h(int signum)
{
    int res;

    if (signum == SIGALRM) {
#ifndef SILENT
        s_printnf(".-- Jumping\n");
#endif

        res = fork();
        if (res == -1) {
#ifndef SILENT
            s_printnf("!    Fork failed\n");
#endif
            alarm(HOP_INTERVAL);
        } else if (res) {
            exit(0);
        } else {
            init_handlers(alrm_h);

            rename_argv0();
#ifndef SILENT
            s_printnf("^-- Jumped! I am now %s\n", argv0);
#endif
        }
    } else {
        s_printnf("!!! Got some other signal %d\n", signum);
    }
}

/* Inane garbage function - if the number is odd then add 1 and double, then/else half it until we reach 1 */
unsigned int some_stuff(unsigned int i)
{
    if (i & 1)
        i = (i + 1) << 1;
    else
        i >>= 1;

    return i;
}

int main(int argc, char **argv)
{
    struct timeval t;
    unsigned int i = 1;

    argv0 = argv[0];

    /* Seed rng to assist some_stuff */
    gettimeofday(&t, NULL);
    srand(t.tv_sec * 1000000 + t.tv_usec / 1000 + t.tv_usec);

    /* Initialise logging file descriptor */
    log_fd = open("log.txt", O_RDWR | O_CREAT, S_IRWXU);

    /* Create the initial handlers */
    init_handlers(alrm_h);

    while (1) {
        /* Do some inane garbage to prove that jumping saves state */
        if (i == 1) {
            i = rand() % 1000;
            s_printnf("=== %d\n", i);
        } else {
            i = some_stuff(i);
            s_printnf(">>> %d\n", i);
        }

        usleep(100000);
    }

    return 0;
}
