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
#define UNTIL_SUNRISE 5            // time until next sunrise
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

pthread_cond_t tmp_sunrise;

void *timer_sunrise(){              // thread for the timer of sunrise

    for(;;){
        p_m_c->countdown_sunrise = SUNRISE_TIME;

        /*//        JUST TESTING PRODUCER
        p_m_c->sunrise.__align = (int)(random() % 10);
        p_m_c->countdown_sunrise = (int)(random() % 100);
        printf("\twrote sunrise.__align: %lld\n", p_m_c->sunrise.__align);
        printf("\twrote countdown_sunrise: %i\n", p_m_c->countdown_sunrise);
//        END TEST PRODUCER - IT'S OK!*/

        // SET CONDITION
        int rtn = pthread_cond_broadcast(&p_m_c->sunrise);       // send a signal for all roosters(threads) waiting for sunrise [rule 1]]
        printf("\nThe sunrise is begun!\n");
        if(rtn != 0){
            perror("pthread_cond_broadcast");
            exit(-1);
        }
        // END SET CONDITION

        //  DECREMENT SUNRISE TIME
        for(; 0 < p_m_c->countdown_sunrise;) {         // <condition> - when countdown is 0, all roosters wait
            printf("->%d\n", p_m_c->countdown_sunrise);
            sleep(1);                  // pass from second to second;
            // JUST TESTING
//            p_m_c->sunrise.__align =  10 * p_m_c->countdown_sunrise;
            p_m_c->countdown_sunrise--;                // decrement sunrise timer
            printf("\t\tNEW countdown_sunrise: %i\n", p_m_c->countdown_sunrise);
//            printf("\t\tNEW sunrise.__align: %i\n", p_m_c->sunrise.__align);
        }
        //  DECREMENT SUNRISE TIME - IT'S OK!

        printf("\nThe sunrise is over!\n");
        sleep(UNTIL_SUNRISE);                   // wait until next sunrise
    }
//    pthread_exit(NULL);                       // not accessed because loop above is infinite
}

int main(int argc, char *argv[]){

    pthread_t timer;                             // var to timer_sunrise

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

/*    close(fd1);
    close(fd2);
    close(fd3);*/

/*//  TESTING PRODUCER
    while(1){
        p_m_c->sunrise.__align = (int)(random() % 10);
        p_m_c->countdown_sunrise = (int)(random() % 100);

        printf("wrote sunrise.__align: %lld\n", p_m_c->sunrise.__align);
        printf("wrote countdown_sunrise: %i\n", p_m_c->countdown_sunrise);
        sleep(1);
    }
//  END TEST PRODUCER - IT'S OK!*/

/*    printf("p_m_c: %p\n\n", p_m_c);

    p_m_c->sunrise.__align = 1;
    printf("wrote: %d in prt_sr->__align\n", p_m_c->sunrise.__align);

    p_m_c->countdown_sunrise = 2;
    printf("wrote: %d in prt_sr->__align\n", p_m_c->countdown_sunrise);

    p_m_c->m_cockcrow.__align = 3;
    printf("wrote: %d in prt_sr->__align\n", p_m_c->m_cockcrow.__align);

    sleep(20);
    exit(1);*/
    if(pthread_create(&timer, NULL, timer_sunrise, NULL)){         // creating timer_sunrise
        perror("pthread_create");                                         // if not create, exit
        exit(-1);
    }

    pthread_exit(NULL);                      // close main thread
}


#pragma clang diagnostic pop