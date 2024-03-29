#include "pcb.h"
#include "ash.h"
#include "scheduler.h"
#include "pandos_const.h"
#include <umps/libumps.h>
#include <umps/cp0.h>

#define TIME_SLICE 5000

/*
    5 devices:
        disk, flash, network card, printer and terminal
        * 8 instances of each device 
            + terminal devices are actually two independent sub-devices
    1 pseudo-device
        the pseudo-clock
*/
#define DEVICE_COUNT 49

int processCount;
int softBlockCount;
pcb_t *currentProcess;
static LIST_HEAD(readyQueue);
int semdCount[DEVICE_COUNT];

pcb_t *getCurrentProcess(){
    return currentProcess;
}

void incrementProcessCount(){
    processCount++;
}

void decrementProcessCount(){
    processCount--;
}

void incrementSBlockedCount(){
    softBlockCount++;
}

void decrementSBlockedCount(){
    softBlockCount--;
}

void addToReadyQueue(pcb_t *p){
    insertProcQ(&readyQueue, p);
}

void removeFromReadyQueue(pcb_t *p){
    outProcQ(&readyQueue, p);
}

int *getClockSemaphore(){
    return &semdCount[DEVICE_COUNT - 1];
}

void schedule(bool isBlocked){
    // check if there are processes in the ready queue
    if (processCount == 0)
        HALT();
    
    if (softBlockCount > 0){
        int status = getSTATUS();
        status |= STATUS_IEc;
        status ^= STATUS_TE;
        WAIT();
    } else {
        PANIC();
    }

    //setTIMER(TRANSLATE_TIME(TIME_SLICE));
    setTIMER(TIME_SLICE);

    if (currentProcess != NULL && !isBlocked){
        // adds pointer to the pcb pointed by currentProcess to the readyQueue
        addToReadyQueue(&(*currentProcess));
    }

    // get the first process in the ready queue
    pcb_t *p = removeProcQ(&readyQueue);
    currentProcess = p;
    currentProcess->p_s.status ^= STATUS_TE;
    //LDST(currentProcess->p_s);
    LDST(&currentProcess->p_s);
}

void initScheduler(){
    processCount = 1;
    softBlockCount = 0;
    currentProcess = NULL;

    for (int i = 0; i < DEVICE_COUNT; i++)
        semdCount[i] = 0;

    mkEmptyProcQ(&readyQueue);
}