#include "pcb.h"
#include "asl.h"
#include "ash.h"
#include "scheduler.h"
#include "pandos_const.h"

int processCount;
int softBlockCount;
pcb_t *currentProcess;
static LIST_HEAD(readyQueue);

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

void schedule(){
    //check if there are processes in the ready queue
    if (processCount == 0)
        HALT();
    
    if (softBlockCount > 0){
        size_t status = getStatus();
        status |= STATUS_IEc;
        status ^= STATUS_TE;
        WAIT();
    } else {
        PANIC();
    }

    setTIMER(TRANSLATE_TIME(TIME_SLICE));

    //get the first process in the ready queue
    pcb_t *p = removeProcQ(&readyQueue);
    currentProcess = p;
    currentProcess->p_s.status ^= STATUS_TE;
    LDST(currentProcess->p_s);
}

void initScheduler(){
    processCount = 1;
    softBlockCount = 0;
    currentProcess = NULL;

    mkEmptyProcQ(&readyQueue);
}