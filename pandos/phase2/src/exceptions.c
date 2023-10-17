#include "exceptions.h"
#include "scheduler.h"
#include "ash.h"
#include "ns.h"
#include "pandos_const.h"
#include <umps/libumps.h>
#include <umps/arch.h>
#include <umps/cp0.h>

#define US_TO_DS 100000 // microseconds to 100ms
#define TIME_SLICE 5000

void memcpy(void *dest, void *src, int len){
    char *s = (char *)src;
    char *d = (char *)dest;
    for (int i = 0; i < len; ++i)
        d[i] = s[i];
}

void _interrupt_local_timer(){
    pcb_t *currentProcess = getCurrentProcess();
    //setTIMER(TRANSLATE_TIME(TIME_SLICE));
    setTIMER(TIME_SLICE);
    memcpy(&currentProcess->p_s, (state_t *)BIOSDATAPAGE, sizeof(state_t));
    addToReadyQueue(currentProcess);
}

void _verhogen(pcb_t *p){
    int *sem_value = p->p_s.reg_a1;
    if (*sem_value == 0){
        //V is permitted, process continues to run
        pcb_t *unblocked = removeBlocked(sem_value);
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


void _interrupt_timer(pcb_t *p){
    int *sem;
    LDIT(US_TO_DS);     // load timer interval
    while (*(sem = getClockSemaphore()) != 1){
        V(p, sem);
    }
}

void _interrupt_devices(int cause){
    //TO DO
}

void _interrupt_terminal(){
    //TO DO
}

void handle_interrupt(pcb_t *p, int cause){
    if (cause & CAUSE_IP(IL_CPUTIMER))
        return _interrupt_local_timer();
    else if (cause & CAUSE_IP(IL_TIMER))
        return _interrupt_timer(p);
    else if (cause & (CAUSE_IP(IL_DISK) | CAUSE_IP(IL_FLASH) | CAUSE_IP(IL_ETHERNET) | CAUSE_IP(IL_PRINTER)))
        return _interrupt_devices(cause);
    else if (cause & CAUSE_IP(IL_TERMINAL))
        return _interrupt_terminal();
}

void _pass_up_or_die(memaddr addr){
    context_t *context;
    pcb_t *currentProcess = getCurrentProcess();
    if (currentProcess->p_supportStruct == NULL){
        _terminate_process(currentProcess);
    } else {
        memcpy(&currentProcess->p_supportStruct->sup_exceptContext[addr], (state_t *)BIOSDATAPAGE, sizeof(state_t));
        context = &currentProcess->p_supportStruct->sup_exceptContext[addr];
        LDCXT(context->stackPtr, context->status, context->pc);
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
    //a1 should contain a pointer to a processor state (state_t *)
    //p_s is of type state_t
    //new->p_s = *(p->p_s.reg_a1);
    new->p_s = *(state_t *)p->p_s.reg_a1;
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
    int *sem = p->p_semAdd;
    if (*sem < 0)           //(*sem < 0 && not blocked on a device sem)
        *sem = *sem + 1;
    if (sem != NULL)
        decrementSBlockedCount();
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
    decrementProcessCount();
    //TODO check it p is blocked on a device sem, in that case decrease sblocked count
    outChild(p);   //removes p from its parent's children list

    /*
    ** If the value of a semaphore is negative, it is an invariant that the
    ** absolute value of the semaphore equal the number of pcb’s blocked on that
    ** semaphore. Hence if a terminated process is blocked on a semaphore,
    ** the value of the semaphore must be adjusted; i.e. incremented.
    */
    int *sem = p->p_semAdd;
    if (*sem < 0)           //(*sem < 0 && not blocked on a device sem)
        *sem = *sem + 1;
    if (sem != NULL)
        decrementSBlockedCount();
}

void _passeren(pcb_t *p){
    int *sem_value = p->p_s.reg_a1;
    if (*sem_value == 1){
        //P is permitted, process continues to run
        pcb_t *unblocked = removeBlocked(sem_value);
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

// //TO DO
//     incrementSBlockedCount();
//     //TO DO
//     decrementProcessCount();
void _do_io(pcb_t *p){
    int *cmdAddr, *cmdValue, *sem;
    cmdAddr = p->p_s.reg_a1;
    cmdValue = p->p_s.reg_a2;
    incrementSBlockedCount();

    decrementProcessCount();
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

// void _get_children_pid(pcb_t *p){
//     int children_number = 0;
//     struct list_head *child = &(p->p_child);

//     while (child != NULL)
//     {
//         if (children_number < p->p_s.reg_a2){
//             p->p_s.reg_a1 = child;
//         }

//         children_number++;
//         child = child->next;
//     }

//     p->p_s.reg_v0 = children_number;
// }

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
                //_get_children_pid(currentProcess);
            default:
                //killed if called a syscall with non-existing code
                kill_progeny(currentProcess);
                break;
        }
        switch(currentProcess->p_s.reg_a0){
            case 3:
            case 5:
            case 7:
                memcpy(&currentProcess->p_s, (state_t *)BIOSDATAPAGE, sizeof(state_t));
                break;
        }
    }
}

void eccccezzzioni(){
    cpu_t start_tod, end_tod;
    pcb_t *currentProcess;

    switch(CAUSE_GET_EXCCODE(getCAUSE())){
        case 0:
            currentProcess = getCurrentProcess();
            handle_interrupt(currentProcess, CAUSE_GET_EXCCODE(getCAUSE()));
            break;
        case 1:
        case 2:
        case 3:
            handle_TLB_trap();
            break;
        case 8:
            STCK(start_tod);
            handle_syscall();
            STCK(end_tod);
            currentProcess = getCurrentProcess();
            currentProcess->p_s.pc_epc += WORDLEN;
            currentProcess->p_time += (end_tod - start_tod);
            LDST((state_t *)BIOSDATAPAGE);
            break;
        default:
            handle_program_trap();
            break;
    }
    schedule(false);
}