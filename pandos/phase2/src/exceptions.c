#include "exceptions.h"
#include "scheduler.h"
#include "pandos_const.h"

void handle_interrupt(){
    //TODO
}

void handle_TLB_trap(){
    
}

void _create_process(pcb_t *p){
    pcb_t *new = allocPcb();
    new->p_s = p->p_s.reg_a1;
    new->p_supportStruct = p->p_s.reg_a2;
    if (p->p_s.reg_a3)
        addNamespace(new, p->p_s.reg_a3);
    else
        addNamespace(new, getNamespace(p, NS_PID));
    new->p_pid = &new;
    incrementProcessCount();
    addToReadyQueue(new);
    insertChild(p, new);
}

void kill_progeny(pcb_t *p){
    pcb_t *child;
    
    while((child = removeChild(p)) != NULL){
        kill_progeny(child);
    }

    decrementProcessCount();
    removeFromReadyQueue(p);
    freePcb(p);
}

void _terminate_process(pcb_t *p){
    if (p->p_s.reg_a1 == 0){
        kill_progeny(p);
    } else {
        p = getProcessByPid(p->p_s.reg_a1);
        kill_progeny(p);
    }
}

void _passeren(pcb_t *p){
    int *sem_value = p->p_s.reg_a1;
    if (*sem_value == 1){
        //P is permitted, process continues to run
        pcb_t *unblocked = removeBlocked(sem_value)
        if (unblocked == NULL){
            //there are no blocked processes, decrement value
            *sem_value = 0;
        }
        else{
            //one process was removed from the blocked pcb list
            //add it to the ready queue
            addToReadyQueue(unblocked);
        }
    }
    else{
        //process is blocked on the semaphore, call scheduler
        insertBlocked(sem_value, p);
        schedule();
    }
}

void _verhogen(pcb_t *p){
    int *sem_value = p->p_s.reg_a1;
    if (*sem_value == 0){
        //V is permitted, process continues to run
        pcb_t *unblocked = removeBlocked(sem_value)
        if (unblocked == NULL){
            //there are no blocked processes, increment value
            *sem_value = 1;
        }
        else{
            //one process was removed from the blocked pcb list
            //add it to the ready queue
            addToReadyQueue(unblocked);
        }
    }
    else{
        //process is blocked on the semaphore, call scheduler
        insertBlocked(sem_value, p);
        schedule();
    }
}

void handle_syscall(){
    pcb_t *currentProcess = getCurrentProcess();
    if (currentProcess->p_s.status & STATUS_KUp){
        //killed if process is not in kernel mode
        kill_progeny(currentProcess);
        return;
    } else {
        switch(currentProcess->p_s.reg_a0){
            case 1:
                _create_process(currentProcess);
                break;
            case 2:
                _terminate_process(currentProcess);
                break;
            case 3:
                _passeren(currentProcess);
                break;
            case 4:
                _verhogen(currentProcess)
                break;
            case 5:
                break;
            case 6:
                break;
            case 7:
                break;
            case 8:
                break;
            case 9:
                break;
            case 10:
                break;
            default:
                //killed if called a syscall with non-existing code
                kill_progeny(currentProcess);
                break;
        }
    }
}

void handle_program_trap(){
    //TODO
}

void eccccezzzioni(){
    size_t cause = getCAUSE();
    CAUSE_GET_EXCCODE(cause);
    switch(cause){
        case 0:
            handle_interrupt();
            break;
        case 1:
        case 2:
        case 3:
            handle_TLB_trap();
            break;
        case 8:
            handle_syscall();
            break;
        default:
            handle_program_trap();
            break;
    }
}