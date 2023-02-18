#include <list.h>
#include <pandos_types.h>
#include <pcb.h>

static pcb_t pcbFree_table[MAXPROC];
static LIST_HEAD(pcbFree_h);

void initPcbs(){
    //initializes empty list
    INIT_LIST_HEAD(&pcbFree_h);

    //adds every pcb in pcbFree_table array to the pcbFree_h list
    for (int i = 0; i < MAXPROC; i++){
        pcb_t *p = &pcbFree_table[i];
        list_add(&p->p_list, &pcbFree_h);
    }
};

void freePcb(pcb_t *p){
    //add p in pcbFree_h list
    list_add_tail(&p->p_list, &pcbFree_h);
};

pcb_t *allocPcb(){
    //check if pcbFree_h is empty
    if(list_empty(&pcbFree_h))
        return NULL;
    
    //if not removes an element from the list
    pcb_t *el = container_of(pcbFree_h.next, pcb_t, p_list);
    list_del(pcbFree_h.next);

    //initializes its fields and returns it
    INIT_LIST_HEAD(&el->p_list);
    el->p_parent = NULL;
    INIT_LIST_HEAD(&el->p_child);
    INIT_LIST_HEAD(&el->p_sib);

    el->p_time = 0;
    el->p_semAdd = NULL;

    return el;
};

void mkEmptyProcQ(struct list_head *head){
    //initializes list as an empty list
    INIT_LIST_HEAD(head);
};

int emptyProcQ(struct list_head *head){
    //check if list is empty
    if(list_empty(head))
        return TRUE;
    return FALSE;
};

void insertProcQ(struct list_head* head, pcb_t *p){
    //adds element pointed by p to the process queue
    list_add_tail(&p->p_list, head);
};

pcb_t* headProcQ(struct list_head *head){
    //check if empty
    if (list_empty(head))
        return NULL;
    //return first element (without removing it)
    return container_of(head->next, pcb_t, p_list);
};

pcb_t* removeProcQ(struct list_head *head){
    //check if empty
    if (list_empty(head))
        return NULL;
    
    //gets first element, removes it from the list
    pcb_t *el = container_of(head->next, pcb_t, p_list);
    list_del(head->next);

    //return pointer to the removed element
    return el;
};

pcb_t* outProcQ(struct list_head *head, pcb_t *p){
    struct list_head *cursor;

    //searches for p in the list
    list_for_each(cursor, head){
        if (container_of(cursor, pcb_t, p_list) == p){
            //removes p from the list
            list_del(&p->p_list);
            return p;
        }
    }
    //if p is not in the list return NULL
    return NULL;
};

int emptyChild(pcb_t *p){
    //check if p_child list is empty (p has no children)
    return list_empty(&p->p_child);
};

void insertChild(pcb_t *prnt, pcb_t *p){
    //adds p to p_child list of prnt
    list_add_tail(&p->p_sib, &prnt->p_child);
    //sets prnt as parent of p
    p->p_parent = prnt;
};

pcb_t *removeChild(pcb_t *p){
    //check if empty (return NULL)
    if(list_empty(&p->p_child))
        return NULL;

    //gets first child, removes it
    pcb_t *el = container_of(p->p_child.next, pcb_t, p_list);
    list_del(p->p_child.next);
    INIT_LIST_HEAD(&el->p_child);
    el->p_parent = NULL;

    return el;
};

pcb_t *outChild(pcb_t *p){
    pcb_t *el = p->p_parent;

    //return NULL if p doesn't have a parent
    if (el == NULL)
        return NULL;
        
    //removes p from children list
    list_del(&p->p_sib);
    INIT_LIST_HEAD(&p->p_sib);
    p->p_parent = NULL;

    return p;
};