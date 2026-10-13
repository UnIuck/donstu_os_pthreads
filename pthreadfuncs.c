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
    char buf[256];
    if (t->id == 1) {
    pthread_detach(pthread_self());
    }
    // write something in opened file
    for (int i = 0; i < COUNT_ITERATIONS; ++i) {
    snprintf(buf, sizeof(buf),
             "[tag = %s] pid = %d tid = %d pthread_self = %lu iter = %d msg = '%s'\n",
             t->tag, getpid(), getThreadID(),
             (unsigned long)pthread_self(), i, t->message);
    write_line(buf);
        // imitation of something
        struct timespec req = {0, 100000000};
	nanosleep(&req, NULL); /* 100 мс */
    }
        // Задание 23: возвращаем 42 для не-detached потоков
    if (t->id != 1) {
        pthread_exit((void *)42L);
    }
    return NULL;
}


void about()  {
    printf("Pthread example\n");
}
