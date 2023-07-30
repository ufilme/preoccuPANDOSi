#include <pandos_const.h>
#include <pandos_types.h>
#include <umps3/umps/libumps.h>
#include <ash.h>
#include <pcb.h>
#include <asl.h>
#include <scheduler.h>
#include <exceptions.h>

/*
    5 devices:
        disk, flash, network card, printer and terminal
        * 8 instances of each device 
            + terminal devices are actually two independent sub-devices
    1 pseudo-device
        the pseudo-clock
*/
#define DEVICE_COUNT 49
#define US_TO_DS 100000 // microseconds to 100ms
#define TIME_SLICE 5000
#define CONVERT_TIME (time) time * (*(int *)(TIMESCALEADDR))


int semdCount[DEVICE_COUNT];


int main(int argc, char *agrv[]){
    initPcbs();
    initASH();
    initSemd();


    passupvector_t *puv = (passupvector_t *)PASSUPVECTOR;
    puv->tlb_refill_handler = (memaddr) uTLB_RefillHandler;
    puv->tlb_refill_stackPtr = (memaddr) KERNELSTACK;
    puv->exception_handler = (memaddr) eccccezzzioni;
    puv->exception_stackPtr = (memaddr) KERNELSTACK;

    for (int i = 0; i < DEVICE_COUNT; i++)
        semdCount[i] = 0;

    setTIMER(TRANSLATE_TIME(TIME_SLICE));      // set local timer for time slice
    LDIT(US_TO_DS);     // load timer interval

    pcb_t *p = allocPcb();
    RAMTOP(p->p_s.reg_sp);
    p->p_s.pc_epc = (memaddr)test;  // pc to test func
    p->p_s.t9 = (memaddr)test;      // set t9 = pc as pops requests
    p->p_s.status |= STATUS_IEp;    // enable interrupts
    p->p_s.status |= STATUS_KUc;    // enable kernel mode
    p->p_s.status |= STATUS_TE;     // enable local timer
    
    initScheduler();
    addToReadyQueue(p)
    schedule();

    return 0;
}