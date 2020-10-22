/*
 * Cockcrow
 * created by robson on 20/10/2020.
 *
 * This litle program simulates the singing of a neighborhood of roosters using threads,
 * mutex and conditional variables.
 *
    Rules:
        1. the roosters can only sing at sunrise;
        2. only one rooster can sing at a time;
        3. one rooster 2 seconds singing;
        4. after [rule 3], its must be 4 unable to crow for 4 seconds;
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#include <mqueue.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>


#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
#define SHARED "/sharedmem5"


// in seconds
#define UNTIL_SUNRISE 10            // time until next sunrise
#define SUNRISE_TIME 5              // sunrise duration
#define TIME_CROWING 1              // time of a crowing [rule 3]
#define TIME_UNABLE 4               // time unable to crow [rule 4]


//pthread_mutex_t m_cockcrow, *prt_m;         // mutex for cockcrow [rule 2]
//pthread_cond_t sunrise, *prt_sr;             // conditional variable for (is or isn't) sunrise [rule 1]
//int countdown_sunrise = 0, *prt_ct_sr;              // countdown for  sunrise [rule 1]
//int fd1, fd2, fd3;
int fd;

struct mutex_cond{
    pthread_mutex_t m_cockcrow;
    pthread_cond_t sunrise;
    int countdown_sunrise;
};

struct mutex_cond *p_m_c;


void *rooster(void *id){            // thread for a rooster
    time_t t_sec;                   // aux. store time in seconds
    struct tm *t_frmt;              // aux. store formatted time

    for(;;){
        pthread_mutex_lock(&p_m_c->m_cockcrow);                // <enter critical region> - require turn to sing [rule 2]

        // WAIT FOR CONDITION
        while(p_m_c->countdown_sunrise <= 0) {                       // while isn't sunrise...
            pthread_cond_wait(&p_m_c->sunrise, &p_m_c->m_cockcrow);   // ... <condition> - wait for (condition) time
            printf("\t\t\tcondition not accept!\n");
        }
//        printf("\n\tAccept condition!\n");

       /* // TESTING CONSUMER
        printf("read sunrise.__align: %lld\n", p_m_c->sunrise.__align);
        printf("read countdown_sunrise: %i\n", p_m_c->countdown_sunrise);
        // END TEST CONSUMER - IT'S OK!*/
#
        // END WAIT FOR CONDITION

        time(&t_sec);                                   // capture time
        t_frmt = localtime(&t_sec);                     // formats time (to hh:mm:ss)

        // crowing and print id, time and thread pid
        printf("Rooster %d is crowing ", (int) id);
        printf("at %02d:%02d:%02d!\n", t_frmt->tm_hour, t_frmt->tm_min, t_frmt->tm_sec);
        printf("\tPID thread %ld\n", (long)pthread_self());
        sleep(TIME_CROWING);                            // time crowing [rule 3]
        pthread_mutex_unlock(&p_m_c->m_cockcrow);              // <leave critical region> - release to others to sing [rule 2]
//
        sleep(TIME_UNABLE);                             // time unable to crow [rule 4]
    }
//    pthread_exit(NULL);                               // not accessed because loop above is infinite
}

int main(int argc, char *argv[]){

    if(argc != 2){
        printf("Invalid param!\n");
        exit(-1);
    }
    int num_threads = atoi(argv[1]);        // get number thread by command line and set (casting to int) var
    pthread_t roosters[num_threads];             // array for roosters

    if((fd = shm_open(SHARED, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) == -1){
        perror("shm_open");
        exit(-1);
    }

    if(ftruncate(fd, sizeof(struct mutex_cond)) == -1){
        perror("ftruncate");
        exit(-1);
    }

    if((p_m_c = mmap(NULL, sizeof(struct mutex_cond), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED){
        perror("mmap");
        exit(-1);
    }

    pthread_mutex_init(&p_m_c->m_cockcrow, NULL);  // start mutex for cockcrow

/*    close(fd1);
    close(fd2);
    close(fd3);*/

/*    printf("p_m_c: %p\n\n", p_m_c);

    printf("read: %d in prt_sr->__align\n", p_m_c->sunrise.__align);

    printf("read: %d in prt_sr->__align\n", p_m_c->countdown_sunrise);

    printf("read: %d in prt_sr->__align\n", p_m_c->m_cockcrow.__align);

    sleep(20);
    exit(1);*/

/*//  TESTING PRODUCER
    while(1){
        printf("read sunrise.__align: %lld\n", p_m_c->sunrise.__align);
        printf("read countdown_sunrise: %i\n", p_m_c->countdown_sunrise);
        sleep(1);
    }
//  END TEST CONSUMER - IT'S OK!*/

    for(int i = 0; i < num_threads; i++) {
        if(pthread_create(&roosters[i], NULL, rooster, (void *)i)){      // creating roosters, passing i as id
            perror("pthread_create");                                      // if not create, exit
            exit(-1);
        }
    }
    shm_unlink(SHARED);
    pthread_mutex_destroy(&p_m_c->m_cockcrow);             // destroy mutex
    pthread_exit(NULL);                      // close main thread
}