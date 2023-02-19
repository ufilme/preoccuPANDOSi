#include <list.h>
#include <hashtable.h>
#include "pandos_types.h"
#include <pcb.h>
#include <ash.h>

static semd_t semd_table[MAXPROC];
static LIST_HEAD(semdFree_h);
static HLIST_HEAD(semd_h);

semd_t *getSemFromASH(int *semAdd){
    semd_t *s;
    struct hlist_node *cursor;

    //search a semaphore with key semAdd in active semaphores list
    hlist_for_each(cursor, &semd_h){
        s = container_of(cursor, semd_t, s_link);
        if (s->s_key == semAdd){
            return s;
        }
    }

    //return NULL if such semaphore doesn't exist
    return NULL;
};

void emptyBlockedProcQ(semd_t *s){
    //if the semaphore list of blocked pcb is empty
    if (emptyProcQ(&s->s_procq)){
        //removes sem from active sem list
        hlist_del(&s->s_link);
        //adds the semaphore to the free sem list
        list_add(&s->s_freelink, &semdFree_h);
    }
}

int insertBlocked(int *semAdd, pcb_t *p){
    semd_t *s = getSemFromASH(semAdd);

    if (s == NULL){
        //allocating a new semd is not possible, return TRUE
        if (list_empty(&semdFree_h))
            return TRUE;
        
        //gets an element (and removes it) from the free semaphores list
        s = container_of(semdFree_h.next, semd_t, s_freelink);
        list_del(semdFree_h.next);

        //initializes the semaphore
        p->p_semAdd = semAdd;
        s->s_key = semAdd;
        INIT_LIST_HEAD(&s->s_procq);
        //adds p to its queue
        insertProcQ(&s->s_procq, p);
        //adds s to active semaphores list
        hlist_add_head(&s->s_link, &semd_h);
    } else {
        //semaphore with key semAdd is in the active semaphores list
        p->p_semAdd = semAdd;
        //adds p to the semaphore queue
        insertProcQ(&s->s_procq, p);
    }

    //if there isn't a semaphore with key semAdd and there are no free semaphores
    return FALSE;
};

pcb_t *removeBlocked(int *semAdd){
    semd_t *s = getSemFromASH(semAdd);

    //if there's no active sem with key semAdd
    if (s == NULL)
        return NULL;
    
    //remove first pcb from blocked pcb list of the semaphore
    pcb_t *p = removeProcQ(&s->s_procq);

    //if there are no blocked pcb
    if (p == NULL)
        return NULL;
    
    //p is no longer blocked on the semaphore s
    p->p_semAdd = NULL;

    //if list of blocked pcb is now empty removes sem from active list and adds it to free list
    emptyBlockedProcQ(s);

    return p;
};

pcb_t *outBlocked(pcb_t *p){
    semd_t *s = getSemFromASH(p->p_semAdd);

    //error: no active semaphore with key p_semAdd
    if (s == NULL)
        return NULL;

    //removes p from s blocked pcb list
    p = outProcQ(&s->s_procq, p);

    //if p was not in the blocked pcb list of semaphore s
    if (p == NULL)
        return NULL;

    //p is no longer blocked on the semaphore s
    p->p_semAdd = NULL;
    
    //if list of blocked pcb is now empty removes sem from active list and adds it to free list
    emptyBlockedProcQ(s);

    return p;
};

pcb_t *headBlocked(int *semAdd){
    semd_t *s = getSemFromASH(semAdd);

    //there's no active semaphore with key semAdd or it's blocked pcb list is empty
    if (s == NULL || emptyProcQ(&s->s_procq))
        return NULL;

    //return first pcb from blocked pcb list without removing it
    pcb_t *p = headProcQ(&s->s_procq);

    return p;
};

void initASH(){
    //semdFree_h = empty list
    INIT_LIST_HEAD(&semdFree_h);

    //adds every semd_table element to the semdFree_h list
    for (int i = 0; i < MAXPROC; i++){
        semd_t *s = &semd_table[i];
        list_add(&s->s_freelink, &semdFree_h);
    }
};