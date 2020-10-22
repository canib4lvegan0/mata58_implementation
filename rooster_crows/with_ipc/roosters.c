/*
 * Cockcrow - producer
 * implement the previous cockcrow using IPC (memory shared, pipe or message queues)
*/

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
//#include <string.h>
//#include <semaphore.h>

#define NAME_SHARED "/sharedmem5"   // shared memory name
/*in seconds*/
#define UNTIL_SUNRISE 5             // time until next sunrise
#define SUNRISE_TIME 5              // sunrise duration
#define TIME_CROWING 1              // time of a crowing [rule 3]
#define TIME_UNABLE 4               // time unable to crow [rule 4]


// struct with shared variables
struct mutex_cond{
    pthread_mutex_t m_cockcrow;     // mutex for cockcrow [rule 2]
    pthread_cond_t sunrise;         // conditional variable for (is or isn't) sunrise [rule 1]
    int countdown_sunrise;          // countdown for  sunrise [rule 1]
};

/*
pthread_mutex_t m_cockcrow;        // mutex for cockcrow [rule 2]
pthread_cond_t sunrise;            // conditional variable for (is or isn't) sunrise [rule 1]
int countdown_sunrise = 0;         // countdown for  sunrise [rule 1]
*/
int fd;                        // file descriptor to shared memory
struct mutex_cond *p_m_c;      // pointer to shared memory region


/* thread for a rooster */
void *rooster(void *id)s{

    time_t t_sec;              // aux. store time in seconds
    struct tm *t_frmt;         // aux. store formatted time

    for(;;){
        pthread_mutex_lock(&p_m_c->m_cockcrow);         // <enter critical region> - require turn to sing [rule 2]

/*
         // TESTING CONSUMER
         printf("read sunrise.__align: %lld\n", p_m_c->sunrise.__align);
         printf("read countdown_sunrise: %i\n", p_m_c->countdown_sunrise);
         // END TEST CONSUMER
*/

        /* TODO solve pthread_cond_broadcast
         * For some reason I dont know, pthread_cond_broadcast()
         * doesn't signals here.
        */
        /* wait for condition */
        while(p_m_c->countdown_sunrise <= 0) {          // while isn't sunrise...

            /* wait for a signal to wake up*/
            pthread_cond_wait(&p_m_c->sunrise, &p_m_c->m_cockcrow);   // ... <condition> - wait for (condition) time
            printf("\t\t\tcondition not accept!\n");           // unnecessary
        }

        time(&t_sec);                   // capture time
        t_frmt = localtime(&t_sec);     // formats time (to hh:mm:ss)

        /* crowing - also, print id, time and thread pid */
        printf("Rooster %d is crowing ", (int) id);
        printf("at %02d:%02d:%02d!\n", t_frmt->tm_hour, t_frmt->tm_min, t_frmt->tm_sec);
        printf("\tPID thread %ld\n", (long)pthread_self());
    sleep(TIME_CROWING);                            // time crowing [rule 3]
        pthread_mutex_unlock(&p_m_c->m_cockcrow);   // <leave critical region> - release to others to sing [rule 2]
        sleep(TIME_UNABLE);                         // time unable to crow [rule 4]
    }
    // pthread_exit(NULL);                          // not accessed because loop above is infinite
}

int main(int argc, char *argv[]){

    if(argc != 2){
        printf("Invalid param!\n");
        exit(-1);
    }

    int num_threads = atoi(argv[1]);        // get number thread by command line and set (casting to int) var
    pthread_t roosters[num_threads];              // array for roosters

    /*Open or create a shared memory*/
    if((fd = shm_open(NAME_SHARED, O_RDWR | O_CREAT, 0777)) == -1){
        perror("shm_open");
        exit(-1);
    }

    /* Truncate the memory region this process*/
    if(ftruncate(fd, sizeof(struct mutex_cond)) == -1){
        perror("ftruncate");
        exit(-1);
    }

    /* Set p_m_c como pointer to new shared memory region*/
    if((p_m_c = mmap(NULL, sizeof(struct mutex_cond), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED){
        perror("mmap");
        exit(-1);
    }
    close(fd);      // closing fd because it's no longer necessary

    /* starting mutex m_cockcrow */
    pthread_mutex_init(&p_m_c->m_cockcrow, NULL);

    // TESTING PRODUCER
    while(1){
        pthread_mutex_lock(&p_m_c->m_cockcrow);

//        while((p_m_c->countdown_sunrise % 2) != 0)
//            pthread_cond_wait(&p_m_c->sunrise, &p_m_c->m_cockcrow);

        printf("read sunrise.__align: %lld\n", p_m_c->sunrise.__align);
        printf("read countdown_sunrise: %i\n", p_m_c->countdown_sunrise);
        sleep(1);
        pthread_mutex_unlock(&p_m_c->m_cockcrow);
    }
    // END TEST CONSUMER - IT'S OK!

    /* creating threads/roosters and its IDs*/
    for(int i = 0; i < num_threads; i++) {
        if(pthread_create(&roosters[i], NULL, rooster, (void *)i)){
            perror("pthread_create");
            exit(-1);
        }
    }

    pthread_mutex_destroy(&p_m_c->m_cockcrow);          // destroy mutex
    shm_unlink(NAME_SHARED);                            // remove the shared memory
    pthread_exit(NULL);                          // close main thread
}