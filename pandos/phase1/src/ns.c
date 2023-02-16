#include "pandos_types.h"
#include <ns.h>
#include <list.h>

static nsd_t pid_nsd[MAXPROC];
static LIST_HEAD(pid_nsFree_h);
static LIST_HEAD(pid_nsList_h);

void initNamespaces(){
    INIT_LIST_HEAD(&pid_nsFree_h);

    for (int i = 0; i < MAXPROC; i++){
        // pid list
        nsd_t *ns = &pid_nsd[i];
        list_add(&ns->n_link, &pid_nsFree_h);
        // other lists
    }
};

nsd_t *getNamespace(pcb_t *p, int type){
    return p->namespaces[type];
};

int addNamespace(pcb_t *p, nsd_t *ns){
    pcb_t *pChild;
    struct list_head *cursor;

    if (ns->n_type == NS_PID){
        p->namespaces[NS_PID] = ns;

        list_for_each(cursor, &p->p_child){
            pChild = container_of(cursor, pcb_t, p_sib);
            pChild->namespaces[NS_PID] = ns;
        }

        return TRUE;
    }

    return FALSE;
};

nsd_t *allocNamespace(int type){
    nsd_t *ns;
    
    if (type == NS_PID){
        if (list_empty(&pid_nsFree_h))
            return NULL;
        
        ns = container_of(pid_nsFree_h.next, nsd_t, n_link);
        list_del(pid_nsFree_h.next);
        ns->n_type = type;
        INIT_LIST_HEAD(&ns->n_link);
        list_add(&ns->n_link, &pid_nsList_h);

        return ns;
    }

    return NULL;
};

void freeNamespace(nsd_t *ns){
    struct list_head *cursor;

    // pid list
    list_for_each(cursor, &pid_nsList_h){
        if (ns == container_of(cursor, nsd_t, n_link)){
            list_del(&ns->n_link);
            list_add(&ns->n_link, &pid_nsFree_h);
        }
    }
};