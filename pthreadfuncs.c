#include "pthreadfuncs.h"
#include <time.h>
#include <stdlib.h>
// common resources - is a file for logging
int g_fd = -1;
int g_counter = 0;
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
    int is_detached_malloc = (t->id == 100);   // Задание 26: структура из malloc
    // Задание 33: проверка pthread_equal
    pthread_t self_check = pthread_self();
    if (pthread_equal(self_check, pthread_self())) {
       char eq_buf[128];
       snprintf(eq_buf, sizeof(eq_buf),
                 "[tag = %s] pthread_equal(self, self) = true\n", t->tag);
        write_line(eq_buf);
}
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
    // Задание 25: для НЕ-detached потоков возвращаем malloc-строку
    // Задание 25: для НЕ-detached потоков (кроме id=100) возвращаем malloc-строку
    for (int j = 0; j < 100000; ++j) {
    g_counter++;
}
    if (t->id != 1 && t->id != 100) {
        char *result = (char *)malloc(64);
        if (result != NULL) {
            snprintf(result, 64, "Thread %d done %d iterations", t->id, COUNT_ITERATIONS);
        }
        pthread_exit((void *)result);
    }

    // Задание 26: освобождаем malloc-структуру detached потока
    if (is_detached_malloc) {
        free(t);
    }

    return NULL;
}


void about()  {
    printf("Pthread example\n");
}
