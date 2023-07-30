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

void handle_syscall(){
    pcb_t *currentProcess = getCurrentProcess();
    if (currentProcess->p_s.status & STATUS_KUp){
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
                break;
            case 4:
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