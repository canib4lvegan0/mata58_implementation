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
//#include <string.h>
//#include <semaphore.h>

#define NAME_SHARED "/sharedmem5"   // shared memory name
/*in seconds*/
#define UNTIL_SUNRISE 5             // time until next sunrise
#define SUNRISE_TIME 5              // sunrise duration
#define TIME_CROWING 1              // time of a crowing [rule 3]
#define TIME_UNABLE 4               // time unable to crow [rule 4]


/*struct with shared variables*/
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


/* thread for the timer of sunrise */
void *timer_sunrise(){

    for(;;){

/*
        // TESTING PRODUCER
        p_m_c->sunrise.__align = (int)(random() % 10);
        p_m_c->countdown_sunrise = (int)(random() % 100);
        printf("\twrote sunrise.__align: %lld\n", p_m_c->sunrise.__align);
        printf("\twrote countdown_sunrise: %i\n", p_m_c->countdown_sunrise);
        // END TEST PRODUCER
*/

        /* set condition */
        p_m_c->countdown_sunrise = SUNRISE_TIME;
        printf("\nThe sunrise is begun!\n");

        /* TODO solve pthread_cond_broadcast()
         * For some reason I dont know, pthread_cond_broadcast() is crashing
         * this process, don't make effect to condition and the other consumer processes.
        */
        /* send a signal for all roosters(threads) waiting for sunrise [rule 1]] */
        if(pthread_cond_broadcast(&p_m_c->sunrise) != 0){
            perror("pthread_cond_broadcast");
            exit(-1);
        }

        /* decrement sunrise time */
        for(; 0 < p_m_c->countdown_sunrise;) {      // <condition> - when countdown is 0, all roosters wait
            printf("->%d\n", p_m_c->countdown_sunrise);
            sleep(1);                      // pass from second to second
            p_m_c->countdown_sunrise--;
        }

        printf("\nThe sunrise is over!\n");
        sleep(UNTIL_SUNRISE);                       // wait until next sunrise
    }
        // pthread_exit(NULL);          // not accessed because loop above is infinite
}

int main(int argc, char *argv[]){

    pthread_t timer;                             // var to timer_sunrise

    /* open|create a shared memory pointer to fd*/
    if((fd = shm_open(NAME_SHARED, O_RDWR | O_CREAT, 0777)) == -1){
        perror("shm_open");
        exit(-1);
    }

    /* truncate the memory region this process*/
    if(ftruncate(fd, sizeof(struct mutex_cond)) == -1){
        perror("ftruncate");
        exit(-1);
    }

    /* set p_m_c como pointer to new shared memory region*/
    if((p_m_c = mmap(NULL, sizeof(struct mutex_cond), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED){
        perror("mmap");
        exit(-1);
    }
    close(fd);          // closing fd because it's no longer necessary

    // TESTING PRODUCER
    while(1){
        p_m_c->sunrise.__align = (int)(random() % 10);
        p_m_c->countdown_sunrise = (int)(random() % 100);

        if((p_m_c->countdown_sunrise % 2) == 0){
        // pthread_cond_broadcast(&p_m_c->sunrise);
            printf("\t\thave condition!\n");
        }

        // printf("wrote sunrise.__align: %lld\n", p_m_c->sunrise.__align);
        printf("wrote countdown_sunrise: %i\n", p_m_c->countdown_sunrise);
        sleep(1);
    }
    // END TEST PRODUCER - IT'S OK!

    /* creating timer_sunrise */
    if(pthread_create(&timer, NULL, timer_sunrise, NULL)){
        perror("pthread_create");
        exit(-1);
    }

    pthread_exit(NULL);         // close main thread
}


#pragma clang diagnostic pop