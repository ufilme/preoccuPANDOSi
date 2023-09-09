pcb_t *getCurrentProcess();

/**
 * Increases the processCount variable by 1
 */
void incrementProcessCount();
/**
 * Decreases the processCount variable by 1
 */
void decrementProcessCount();

/**
 * Increases the softBlockedCount variable by 1
 */
void incrementSBlockedCount();
/**
 * Decreases the softBlockedCount variable by 1
 */
void decrementSBlockedCount();

void addToReadyQueue(pcb_t *p);

void removeFromReadyQueue(pcb_t *p);

int *getClockSemaphore();

/**
 * Remove a process from the ready queue and set it as the current process
 */
void schedule(bool isBlocked);

/**
 * Set global variables and summon the scheduler
 */
void initScheduler();