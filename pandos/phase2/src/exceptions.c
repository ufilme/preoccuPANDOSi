#include "exceptions.h"
#include "scheduler.h"
#include "pandos_const.h"

#define US_TO_DS 100000 // microseconds to 100ms
#define TIME_SLICE 5000

void _interrupt_local_timer(){
    pcb_t *currentProcess = getCurrentProcess();
    setTIMER(TRANSLATE_TIME(TIME_SLICE));
    memcpy(&currentProcess->p_s, (state_t *)BIOSDATAPAGE, sizeof(state_t));
    addToReadyQueue(currentProcess);
    #31#
}

void _interrupt_timer(){
    int *sem;
    LDIT(US_TO_DS);     // load timer interval
    while ((sem = getClockSemaphore()) != 1){
        V(sem);
    }
}

void _interrupt_devices(int cause){
    //TO DO
}

void _interrupt_terminal(){
    //TO DO
}

void handle_interrupt(int cause){
    if (cause & CAUSE_IP(IL_CPUTIMER))
        return _interrupt_local_timer();
    else if (cause & CAUSE_IP(IL_TIMER))
        return _interrupt_timer();
    else if (cause & (CAUSE_IP(IL_DISK) | CAUSE_IP(IL_FLASH) | CAUSE_IP(IL_ETHERNET) | CAUSE_IP(IL_PRINTER)))
        return _interrupt_devices(cause);
    else if (cause & CAUSE_IP(IL_TERMINAL))
        return _interrupt_terminal();
}

void _pass_up_or_die(memaddr addr){
    context_t context;
    pcb_t *currentProcess = getCurrentProcess();
    if (currentProcess->p_supportStruct == NULL){
        _terminate_process(currentProcess);
    } else {
        memcpy(&currentProcess->p_supportStruct->sup_exceptContext[addr], BIOSDATAPAGE, sizeof(state_t));
        context = &currentProcess->p_support->sup_except_context[addr]
        LDCXT(context->stack_ptr, context->status, context->pc);
    }
}

void handle_TLB_trap(){
    _pass_up_or_die((memaddr)PGFAULTEXCEPT);
}

void handle_program_trap(){
    _pass_up_or_die((memaddr)GENERALEXCEPT);
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
        //process is blocked on the semaphore, call scheduler (done in eccccezzzioni)
        insertBlocked(sem_value, p);
    }
}

void P(pcb_t *p, int *sem_value){
    if (sem_value != NULL)
        p->p_s.reg_a1 = sem_value;
    _passeren(p);
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
        //process is blocked on the semaphore, call scheduler (done in eccccezzzioni)
        insertBlocked(sem_value, p);
    }
}

//TO DO
void V(pcb_t *p, int *sem_value){
    if (sem_value != NULL)
        p->p_s.reg_a1 = sem_value;
    _verhogen(p);
}

// //TO DO
//     incrementSBlockedCount();
//     //TO DO
//     decrementProcessCount();
void _do_io(pcb_t *p){
    int *cmdAddr, *cmdValue, *sem;
    int cmdAddr = p->p_s.reg_a1;
    int cmdValue = p->p_s.reg_a2;

}

void _get_cpu_time(pcb_t *p){
    p->p_s.reg_v0 = p->p_time;
}

void _wait_for_clock(pcb_t *p){ //TO DO
    //process is blocked on the pseudo-clock semaphore, call scheduler
    int *sem = getClockSemaphore();
    //insertBlocked(sem, p);
    P(p, sem);
}

void _get_support_data(pcb_t *p){
    p->p_s.reg_v0 = p->p_supportStruct;
}

void _get_process_id(pcb_t *p){
    if (p->p_s.reg_a1 == 0)
        p->p_s.reg_v0 = p->p_pid;
    else {
        if (p->namespaces[0] == p->p_parent->namespaces[0])
            p->p_s.reg_v0 = p->p_parent->p_pid;
        else
            p->p_s.reg_v0 = 0;
    }   
}

void _get_children_pid(pcb_t *p){
    int children_number = 0;
    struct list_head child = p->p_child;

    while (child != NULL)
    {
        if (i < p->p_s.reg_a2){
            p->p_s.reg_a1[i] = child->p_pid;
        }

        children_number++;
        child = child->next;
    }

    p->p_s.reg_v0 = children_number;
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
                _passeren(currentProcess, NULL);
                break;
            case 4:
                _verhogen(currentProcess);
                break;
            case 5:
                _do_io(currentProcess);
                break;
            case 6:
                _get_cpu_time(currentProcess);
                break;
            case 7:
                _wait_for_clock(currentProcess);
                break;
            case 8:
                _get_support_data(currentProcess);
                break;
            case 9:
                _get_process_id(currentProcess);
                break;
            case 10:
                break;
                _get_children_pid(currentProcess);
            default:
                //killed if called a syscall with non-existing code
                kill_progeny(currentProcess);
                break;
        }
        switch(currentProcess->p_s.reg_a0){
            case 3:
            case 5:
            case 7:
                memcpy(&currentProcess->p_s, BIOSDATAPAGE, sizeof(state_t));
                break;
        }
    }
}

void eccccezzzioni(){
    size_t cause = getCAUSE();
    int start_tod, end_tod;
    pcb_t *currentProcess;
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
            STCK(&start_tod);
            handle_syscall();
            STCK(&end_tod);
            currentProcess = getCurrentProcess();
            currentProcess->p_s.pc_epc += WORDLEN;
            currentProcess->p_time += (end_tod - start_tod);
            LDST(BIOSDATAPAGE);
            break;
        default:
            handle_program_trap();
            break;
    }
    schedule();
}