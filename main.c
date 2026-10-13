#include "pthreadfuncs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>

int main(void) {
    // headline
    about();

    // array with threads
    pthread_t threads[COUNT_THREADS];
    struct ThreadArgs args[COUNT_THREADS];
    char main_buf[256];

    // sys call - open
    // file, modes, rights
    g_fd = open("output.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (g_fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    // задание 8: вместо printf → write_line (через snprintf)
    snprintf(main_buf, sizeof(main_buf),
             "main: pid = %d, opened file: 'output.log' (fd = %d)\n",
             getThreadID(), g_fd);
    write_line(main_buf);

    // задание 7: цикл snprintf вместо ручного strcpy
    for (int i = 0; i < COUNT_THREADS; ++i) {
    args[i].id = i + 1;
    snprintf(args[i].tag, sizeof(args[i].tag), "T%d", i);
    snprintf(args[i].message, sizeof(args[i].message),
             "Hello from thread %d", i);
    }

    // create threads
    for (int i = 0; i < COUNT_THREADS; ++i) {
        int rc = pthread_create(&threads[i], NULL, func_thread, &args[i]);
        if (rc != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(rc));
            return EXIT_FAILURE;
        }
    }

    // wait stopping all threads
    for (int i = 0; i < COUNT_THREADS; ++i) {
    void *retval = NULL;
    int rc = pthread_join(threads[i], &retval);
    if (rc != 0) {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "main: pthread_join для потока %d: %s (rc=%d)\n",
                 i, strerror(rc), rc);
        write_line(msg);
    } else if (retval != NULL) {
        char *received = (char *)retval;
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "main: поток %d вернул: '%s'\n", i, received);
        write_line(msg);
        free(received);   
    } else {
        write_line("main: поток вернул NULL (detached)\n");
    }
}

    // последнее сообщение в лог — ДО уничтожения мьютекса!
    write_line("main: all threads finished, file closed\n");

    // remove mutex
    pthread_mutex_destroy(&g_lock);

    // sys call for close file
    if (close(g_fd) < 0) {
        perror("close");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
