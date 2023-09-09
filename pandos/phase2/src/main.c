#include <pandos_const.h>
#include <pandos_types.h>
#include <umps/libumps.h>
#include <umps/cp0.h>
#include <ash.h>
#include <pcb.h>
#include <ns.h>
#include <p2test.h>
#include <scheduler.h>
#include <exceptions.h>

#define US_TO_DS 100000 // microseconds to 100ms
#define TIME_SLICE 5000
#define CONVERT_TIME (time) time * (*(int *)(TIMESCALEADDR))

int main(int argc, char *agrv[]){
    initPcbs();
    initASH();
    //initSemd();
    initNamespaces();


    passupvector_t *puv = (passupvector_t *)PASSUPVECTOR;
    puv->tlb_refill_handler = (memaddr) uTLB_RefillHandler;
    puv->tlb_refill_stackPtr = (memaddr) KERNELSTACK;
    puv->exception_handler = (memaddr) eccccezzzioni;
    puv->exception_stackPtr = (memaddr) KERNELSTACK;

    //setTIMER(TRANSLATE_TIME(TIME_SLICE));      // set local timer for time slice
    setTIMER(TIME_SLICE);      // set local timer for time slice
    LDIT(US_TO_DS);     // load timer interval

    pcb_t *p = allocPcb();
    RAMTOP(p->p_s.reg_sp);
    p->p_s.pc_epc = (memaddr)test;  // pc to test func
    p->p_s.reg_t9 = (memaddr)test;      // set t9 = pc as pops requests
    p->p_s.status |= STATUS_IEp;    // enable interrupts
    p->p_s.status |= STATUS_KUc;    // enable kernel mode
    p->p_s.status |= STATUS_TE;     // enable local timer
    
    initScheduler();
    addToReadyQueue(p);
    schedule(0);

    return 0;
}