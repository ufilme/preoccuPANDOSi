#include "pandos_const.h"
#include "list.h"
#include "types.h"

/**
 * Insert the element pointed to by p onto the pcbFree list.
 * 
 * @param p pcb_t to put in pcbFree list
 */
void freePcb(pcb_t *p);

/**
 * @brief Return NULL if the pcbFree list is empty. Otherwise, re-
 * move an element from the pcbFree list, provide initial values
 * for ALL of the pcbs fields (i.e. NULL and/or 0) and then
 * return a pointer to the removed element. pcbs get reused, so
 * it is important that no previous value persist in a pcb when
 * it gets reallocated
 * 
 * @return pcb_t* NULL if pcbFree is empty
 *                else pointer to pcbFree removed element
 */
pcb_t *allocPcb();

/**
 * @brief Initialize the pcbFree list to contain all the elements of
 * the static array of MAXPROC pcbs. This method will be
 * called only once during data structure initialization
 * 
 */
void initPcbs();

/**
 * Create a pcb list, by initialising it as an empty list
 */
void mkEmptyProcQ(struct list_head *head);

/**
 * @brief Return TRUE if the queue whose tail is pointed to by head is
 * empty. Return FALSE otherwise
 * 
 * @param head queue
 * @return int true if queue is empty else false if queue is not empty
 */
int emptyProcQ(struct list_head *head);

/**
 * @brief Insert the pcb pointed to by p into the process queue whose tail-
 * pointer is pointed to by head. Note the double indirection through
 * tp to allow for the possible updating of the tail pointer as well
 * 
 * @param head queue
 * @param p pcb
 * 
 */
void insertProcQ(struct list_head *head, pcb_t *p);

/**
 * @brief Remove the first (i.e. head) element from the process queue
 * whose tail-pointer is pointed to by head. Return NULL if the pro-
 * cess queue was initially empty; otherwise return the pointer to
 * the removed element. Update the process queue’s tail pointer if
 * necessary
 * 
 * @param head queue
 * @return pcb_t* NULL if queue is empty else pointer to removed element
 */
pcb_t *removeProcQ(struct list_head *head);

/**
 * @brief Remove the pcb pointed to by p from the process queue whose
 * tail-pointer is pointed to by head. Update the process queue’s tail
 * pointer if necessary. If the desired entry is not in the indicated
 * queue (an error condition), return NULL; otherwise, return p.
 * Note that p can point to any element of the process queue
 * 
 * @param haed queue
 * @param p pcb
 * @return pcb_t* NULL if p is not in tp else p
 */
pcb_t *outProcQ(struct list_head *head, pcb_t *p);

/**
 * @brief Return a pointer to the first pcb from the process queue whose
 * tail is pointed to by head. Do not remove this pcbfrom the process
 * queue. Return NULL if the process queue is empty
 * 
 * @param head queue
 * @return pcb_t* NULL if tp is empty else first pcb is tp
 */
pcb_t *headProcQ(struct list_head *head);

/**
 * @brief Return TRUE if the pcb pointed to by p has no children.
 * Return FALSE otherwise
 * 
 * @param p pcb
 * @return int true if p has no children else false
 */
int emptyChild(pcb_t *p);

/**
 * Make the pcb pointed to by p a child of the pcb pointed to by prnt
 * 
 * @param prnt pcb 
 * @param p pcb
 */
void insertChild(pcb_t *prnt, pcb_t *p);

/**
 * @brief Make the first child of the pcb pointed to by p no longer a
 * child of p. Return NULL if initially there were no children of p.
 * Otherwise, return a pointer to this removed first child pcb
 * 
 * @param p pcb
 * @return pcb_t*  NULL if p no children else pointer to first remove child
 */
pcb_t *removeChild(pcb_t *p);

/**
 * @brief Make the pcb pointed to by p no longer the child of its parent.
 * If the pcb pointed to by p has no parent, return NULL; otherwise,
 * return p. Note that the element pointed to by p need not be the
 * first child of its parent
 * 
 * @param p pcb
 * @param pcb_t* NULL if p has no parent else p
 */
pcb_t *outChild(pcb_t *p);