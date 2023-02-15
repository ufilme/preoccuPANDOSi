#include <list.h>
#include "pandos_types.h"
#include <pcb.h>

static LIST_HEAD(pcbFree_h);
static pcb_t pcbFree_table[MAXPROC];

void initPcbs(){
    INIT_LIST_HEAD(&pcbFree_h);

    for (int i = 0; i < MAXPROC; i++){
        list_add(&pcbFree_table[i].p_list, &pcbFree_h);
    }
};

void freePcb(pcb_t *p){
    list_add(&p->p_list, &pcbFree_h);
};

pcb_t *allocPcb(){
    if(list_empty(&pcbFree_h))
        return NULL;
    
    pcb_t *el = container_of(pcbFree_h.next, pcb_t, p_list);
    list_del(pcbFree_h.next);

    INIT_LIST_HEAD(&el->p_list);
    el->p_parent = NULL;
    INIT_LIST_HEAD(&el->p_child);
    INIT_LIST_HEAD(&el->p_sib);

    el->p_s.entry_hi = 0;
    el->p_s.cause = 0;
    el->p_s.status = 0;
    el->p_s.pc_epc = 0;
    for (int i = 0; i < STATE_GPR_LEN; i++)
        el->p_s.gpr[i] = 0;
    el->p_s.hi = 0;
    el->p_s.lo = 0;

    el->p_time = 0;
    el->p_semAdd = NULL;

    for (int i = 0; i < NS_TYPE_MAX; i++){
        el->namespaces[i]->n_type = 0;
        LIST_HEAD(n_link);
        el->namespaces[i]->n_link = n_link;
    }

    return el;
};

void mkEmptyProcQ(struct list_head *head){
    INIT_LIST_HEAD(head);
};

int emptyProcQ(struct list_head *head){
    if(list_empty(head))
        return TRUE;
    return FALSE;
};

void insertProcQ(struct list_head* head, pcb_t *p){
    list_add(&p->p_list, head);
};

pcb_t* headProcQ(struct list_head *head){
    if (list_empty(head))
        return NULL;
    return container_of(head->next, pcb_t, p_list);
};

pcb_t* removeProcQ(struct list_head *head){
    if (list_empty(head))
        return NULL;
    
    pcb_t *el = container_of(head->next, pcb_t, p_list);
    list_del(head->next);

    return el;
};

pcb_t* outProcQ(struct list_head *head, pcb_t *p){
    struct list_head *cursor;

    list_for_each(cursor, head){
        if (container_of(cursor, pcb_t, p_list) == p){
            list_del(&p->p_list);
            return p;
        }
    }
    
    return NULL;
};

int emptyChild(pcb_t *p){
    return list_empty(&p->p_child);
};

void insertChild(pcb_t *prnt, pcb_t *p){
    list_add_tail(&p->p_list, &prnt->p_child);
    p->p_parent = prnt;
};

pcb_t *removeChild(pcb_t *p){
    if(list_empty(&p->p_child))
        return NULL;

    pcb_t *el = container_of(p->p_child.next, pcb_t, p_list);
    list_del(p->p_child.next);
    INIT_LIST_HEAD(&el->p_child);
    el->p_parent = NULL;

    return el;
};

pcb_t *outChild(pcb_t *p){
    pcb_t *el = p->p_parent;

    if (el == NULL)
        return NULL;
    
    list_del(&p->p_sib);
    INIT_LIST_HEAD(&p->p_sib);
    p->p_parent = NULL;

    return p;
};