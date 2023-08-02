void handle_interrupt();

void handle_TLB_trap();

void handle_program_trap();

/*
 * SYSCALL 1
*/
void _create_process(pcb_t *p);

/*
 * SYSCALL 2 
*/
void kill_progeny(pcb_t *p);

/*
 * SYSCALL 3
*/
void _terminate_process(pcb_t *p);

/*
 * SYSCALL 4
*/
void _passeren(pcb_t *p);

/*
 * SYSCALL 5
*/
void _verhogen(pcb_t *p);

/*
 * SYSCALL 6
*/
void _do_io(pcb_t *p);

/*
 * SYSCALL 7
*/
void _get_cpu_time(pcb_t *p);

/*
 * SYSCALL 8
*/
void _wait_for_clock(pcb_t *p);

/*
 * Terminate calling process if not in kernel mode
 * or called syscall with non-existing code
 * 
 * Calls appropriate function to handle the syscall request
*/
void handle_syscall();

/*
 * Check the ExcCode field of the Cause register and based on that
 * calls the right function to handle
 * interrupts, syscalls, TRLB traps or program traps 
*/
void eccccezzzioni();