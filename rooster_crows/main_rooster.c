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

// in seconds
#define UNTIL_SUNRISE 10            // time until next sunrise
#define SUNRISE_TIME 5              // sunrise duration
#define TIME_CROWING 1              // time of a crowing [rule 3]
#define TIME_UNABLE 4               // time unable to crow [rule 4]


pthread_mutex_t m_cockcrow;         // mutex for cockcrow [rule 2]
pthread_cond_t sunrise;             // conditional variable for (is or isn't) sunrise [rule 1]
int countdown_sunrise = 0;              // countdown for  sunrise [rule 1]


void *rooster(void *id){            // thread for a rooster
    time_t t_sec;                   // aux. store time in seconds
    struct tm *t_frmt;              // aux. store formated time

    for(;;){
        pthread_mutex_lock(&m_cockcrow);                // <enter critical region> - require turn to sing [rule 2]
        while(countdown_sunrise <= 0)                       // while isn't sunrise...
            pthread_cond_wait(&sunrise, &m_cockcrow);   // ... <condition> - wait for (condition) time

        time(&t_sec);                                   // capture time
        t_frmt = localtime(&t_sec);                     // formats time (to hh:mm:ss)

        // crowing and print id, time and thread pid
        printf("Rooster %d is crowing ", (int) id);
        printf("at %02d:%02d:%02d!\n", t_frmt->tm_hour, t_frmt->tm_min, t_frmt->tm_sec);
        printf("\tPID thread %ld\n", (long)pthread_self());
        sleep(TIME_CROWING);                            // time crowing [rule 3]
        pthread_mutex_unlock(&m_cockcrow);              // <leave critical region> - release to others to sing [rule 2]

        sleep(TIME_UNABLE);                             // time unable to crow [rule 4]
    }
//    pthread_exit(NULL);                               // not accessed because loop above is infinite
}

void *timer_sunrise(){              // thread for the timer of sunrise

    for(;;){
        countdown_sunrise = SUNRISE_TIME;       // charge sunrise timer
        printf("\nThe sunrise is begun!\n");
        pthread_cond_broadcast(&sunrise);       // send a signal for all roosters(threads) waiting for sunrise [rule 1]

        for(; 0 < countdown_sunrise;) {         // <condition> - when countdown is 0, all roosters wait
            sleep(1);                  // pass from second to second
            countdown_sunrise--;                // decrement sunrise timer
        }
        printf("\nThe sunrise is over!\n\t waiting for...");

        sleep(UNTIL_SUNRISE);                   // wait until next sunrise
    }
//    pthread_exit(NULL);                       // not accessed because loop above is infinite
}

int main(int argc, char *argv[]){

    if(argc != 2){
        printf("Invalid param!\n");
        exit(-1);
    }

    int num_threads = atoi(argv[1]);        // get number thread by command line and set (casting to int) var
    pthread_t timer;                             // var to timer_sunrise
    pthread_t roosters[num_threads];             // array for roosters

    pthread_mutex_init(&m_cockcrow, NULL);  // start mutex for cockcrow

    if(pthread_create(&timer, NULL, timer_sunrise, NULL)){         // creating timer_sunrise
        perror("pthread_create");                                         // if not create, exit
        exit(-1);
    }

    for(int i = 0; i < num_threads; i++) {
       if(pthread_create(&roosters[i], NULL, rooster, (void *)i)){      // creating roosters, passing i as id
           perror("pthread_create");                                      // if not create, exit
           exit(-1);
       }
    }

    pthread_mutex_destroy(&m_cockcrow);             // destroy mutex
    pthread_exit(NULL);                      // close main thread
}