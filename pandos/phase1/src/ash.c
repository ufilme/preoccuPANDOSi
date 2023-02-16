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

    hlist_for_each(cursor, &semd_h){
        s = container_of(cursor, semd_t, s_link);
        if (s->s_key == semAdd){
            return s;
        }
    }

    return NULL;
};

void emptyBlockedProcQ(semd_t *s){
    if (emptyProcQ(&s->s_procq)){
        hlist_del(&s->s_link);
        list_add(&s->s_freelink, &semdFree_h);
    }
}

int insertBlocked(int *semAdd, pcb_t *p){
    semd_t *s = getSemFromASH(semAdd);

    if (s == NULL){
        if (list_empty(&semdFree_h))
            return TRUE;
        
        s = container_of(semdFree_h.next, semd_t, s_freelink);
        list_del(semdFree_h.next);

        p->p_semAdd = semAdd;
        s->s_key = semAdd;
        INIT_LIST_HEAD(&s->s_procq);
        insertProcQ(&s->s_procq, p);
        hlist_add_head(&s->s_link, &semd_h);
    } else {
        p->p_semAdd = semAdd;
        insertProcQ(&s->s_procq, p);
    }

    return FALSE;
};

pcb_t *removeBlocked(int *semAdd){
    semd_t *s = getSemFromASH(semAdd);

    if (s == NULL)
        return NULL;
    
    pcb_t *p = removeProcQ(&s->s_procq);

    if (p == NULL)
        return NULL;
    
    p->p_semAdd = NULL;

    emptyBlockedProcQ(s);

    return p;
};

pcb_t *outBlocked(pcb_t *p){
    semd_t *s = getSemFromASH(p->p_semAdd);

    if (s == NULL)
        return NULL;

    p = outProcQ(&s->s_procq, p);

    if (p == NULL)
        return NULL;

    p->p_semAdd = NULL;
    
    emptyBlockedProcQ(s);

    return p;
};

pcb_t *headBlocked(int *semAdd){
    semd_t *s = getSemFromASH(semAdd);

    if (s == NULL || emptyProcQ(&s->s_procq))
        return NULL;

    pcb_t *p = headProcQ(&s->s_procq);

    return p;
};

void initASH(){
    INIT_LIST_HEAD(&semdFree_h);

    for (int i = 0; i < MAXPROC; i++){
        semd_t *s = &semd_table[i];
        list_add(&s->s_freelink, &semdFree_h);
    }
};