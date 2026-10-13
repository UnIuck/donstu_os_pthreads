#include "pthreadfuncs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>        // для nanosleep, struct timespec

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

    // Задание 8: вместо printf -> write_line
    snprintf(main_buf, sizeof(main_buf),
             "main: pid = %d, opened file: 'output.log' (fd = %d)\n",
             getThreadID(), g_fd);
    write_line(main_buf);

    // Задание 7: цикл snprintf вместо ручного strcpy
    // Задание 24: заполняем message
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

    // Задание 26: создаём ОТДЕЛЬНЫЙ detached поток с malloc-структурой
    struct ThreadArgs *detached_arg =
        (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
    if (detached_arg != NULL) {
        detached_arg->id = 100;
        snprintf(detached_arg->tag, sizeof(detached_arg->tag), "DT");
        snprintf(detached_arg->message, sizeof(detached_arg->message),
                 "Detached thread from malloc");

        pthread_t detached_thread;
        int rc = pthread_create(&detached_thread, NULL, func_thread, detached_arg);
        if (rc != 0) {
            fprintf(stderr, "pthread_create (detached): %s\n", strerror(rc));
            free(detached_arg);
        } else {
            pthread_detach(detached_thread);
            // detached_arg НЕ освобождаем — освободит поток в func_thread
        }
    }

    // Дадим detached потоку время завершиться (500 мс)
    struct timespec wait_delay = {0, 500000000};
    nanosleep(&wait_delay, NULL);

    // wait stopping all threads
    // Задание 22: обрабатываем ошибку pthread_join (для detached)
    // Задание 25: принимаем malloc-строку и освобождаем
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
            free(received);  // обязательно освобождаем malloc-строку
        } else {
            write_line("main: поток вернул NULL (detached)\n");
        }
    }

    // последнее сообщение в лог — ДО уничтожения мьютекса
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
