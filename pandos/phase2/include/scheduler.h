/**
 * Set global variables and summon the scheduler
 */
void initScheduler();
/**
 * Remove a process from the ready queue and set it as the current process
 */
void schedule();

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