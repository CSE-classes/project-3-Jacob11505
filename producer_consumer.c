#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 5

char buffer[BUFFER_SIZE];
int in = 0, out = 0, count = 0;

pthread_mutex_t lock;
pthread_cond_t not_full, not_empty;

FILE *fp;

void *producer(void *arg) {
    char ch;

    while ((ch = fgetc(fp)) != EOF) {
        pthread_mutex_lock(&lock);

        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&not_full, &lock);
        }

        buffer[in] = ch;
        in = (in + 1) % BUFFER_SIZE;
        count++;

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&lock);
    }

    return NULL;
}

void *consumer(void *arg) {
    char ch;

    while (1) {
        pthread_mutex_lock(&lock);

        while (count == 0) {
            pthread_cond_wait(&not_empty, &lock);
        }

        ch = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;

        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&lock);

        printf("%c", ch);
        fflush(stdout);
    }

    return NULL;
}

int main() {
    pthread_t p, c;

    fp = fopen("message.txt", "r");
    if (!fp) {
        printf("Error opening file\n");
        return -1;
    }

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&not_full, NULL);
    pthread_cond_init(&not_empty, NULL);

    pthread_create(&p, NULL, producer, NULL);
    pthread_create(&c, NULL, consumer, NULL);

    pthread_join(p, NULL);
    pthread_cancel(c);

    fclose(fp);
    return 0;
}
