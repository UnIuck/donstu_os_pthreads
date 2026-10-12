#include "pthreadfuncs.h"
#include <time.h>

// common resources - is a file for logging
int g_fd = -1;
pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

// git ID of current thread
pid_t getThreadID(void) {
    return (pid_t) syscall(SYS_gettid);
}

// write a string from thread with mutex
int write_line(const char *msg) {
    pthread_mutex_lock(&g_lock);
    ssize_t n = write(g_fd, msg, strlen(msg));
    if (n < 0) {
        fprintf(stderr, "write() failed: %s, [file descr = %d]\n", strerror(errno), g_fd);
        pthread_mutex_unlock(&g_lock);
        return -1;
    }
    pthread_mutex_unlock(&g_lock);
    return 0;
}

// function for thread
void *func_thread(void *arg){
struct ThreadArgs *t = (struct ThreadArgs *)arg;
    char buf[128];
    // write something in opened file
    for (int i = 0; i < COUNT_ITERATIONS; ++i) {
        // here we write process id, parent process id, thread id, and name of thread with iteration
        snprintf(buf, sizeof(buf),
                 "[tag = %s] pid = %d ppid = %d tid = %d iter = %d\n", t->tag, getpid(), getppid(), getThreadID(), i);
        write_line(buf);
        // imitation of something
        struct timespec req = {0, 100000000};
	nanosleep(&req, NULL); /* 100 мс */
    }
    return NULL;
}

void about()  {
    printf("Pthread example\n");
}
