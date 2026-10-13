#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>

/* Общие ресурсы */
int g_fd = -1;                    /* файл output.log */
pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;

int g_data = 0;                   /* общий буфер (1 элемент) */
int g_ready = 0;                  /* есть данные для потребителя */
int g_done = 0;                   /* производитель закончил */

/* Производитель */
void *producer(void *arg) {
    (void)arg;
    for (int i = 1; i <= 10; ++i) {
        pthread_mutex_lock(&g_lock);
        g_data = i;
        g_ready = 1;
        pthread_mutex_unlock(&g_lock);

        pthread_cond_signal(&g_cond);   /* будим потребителя */

        struct timespec req = {0, 50000000};  /* 50 мс */
        nanosleep(&req, NULL);
    }

    pthread_mutex_lock(&g_lock);
    g_done = 1;                          /* сообщаем: закончили */
    pthread_mutex_unlock(&g_lock);

    pthread_cond_signal(&g_cond);         /* финальное пробуждение */
    return NULL;
}

/* Потребитель */
void *consumer(void *arg) {
    (void)arg;
    while (1) {
        pthread_mutex_lock(&g_lock);

        /* ОБЯЗАТЕЛЬНО while, а не if — защита от ложных пробуждений! */
        while (!g_ready && !g_done) {
            pthread_cond_wait(&g_cond, &g_lock);
        }

        if (g_ready) {
            char buf[128];
            int len = snprintf(buf, sizeof(buf), "Consumer got: %d\n", g_data);
            write(g_fd, buf, len);
            g_ready = 0;
        }

        int done = g_done;
        pthread_mutex_unlock(&g_lock);

        if (done && !g_ready) break;
    }

    return NULL;
}

int main(void) {
    g_fd = open("output.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (g_fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    pthread_t prod_thread, cons_thread;
    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_create(&cons_thread, NULL, consumer, NULL);

    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    close(g_fd);
    pthread_mutex_destroy(&g_lock);
    pthread_cond_destroy(&g_cond);

    printf("Program finished\n");
    return EXIT_SUCCESS;
}
