#include <list.h>
#include <pandos_types.h>
#include <ns.h>

//pid type namespace
static nsd_t pid_nsd[MAXPROC];
static LIST_HEAD(pid_nsFree_h);
static LIST_HEAD(pid_nsList_h);

void initNamespaces(){
    INIT_LIST_HEAD(&pid_nsFree_h);

    for (int i = 0; i < MAXPROC; i++){
        //adds nsd in pid nsd to list of free pid type nsd
        nsd_t *ns = &pid_nsd[i];
        list_add(&ns->n_link, &pid_nsFree_h);
    }
};

nsd_t *getNamespace(pcb_t *p, int type){
    //return namespace of type "type" of process p
    return p->namespaces[type];
};

int addNamespace(pcb_t *p, nsd_t *ns){
    pcb_t *pChild;
    struct list_head *cursor;

    //adds p and child processes to the ns namespace
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
        //check if there are free ns
        if (list_empty(&pid_nsFree_h))
            return NULL;
        
        //gets first element, removes it from list of free ns
        ns = container_of(pid_nsFree_h.next, nsd_t, n_link);
        list_del(pid_nsFree_h.next);

        //initializes ns, adds it to list of active namespaces
        ns->n_type = type;
        INIT_LIST_HEAD(&ns->n_link);
        list_add(&ns->n_link, &pid_nsList_h);

        return ns;
    }

    return NULL;
};

void freeNamespace(nsd_t *ns){
    struct list_head *cursor;

    //check every element in the active namespaces list
    list_for_each(cursor, &pid_nsList_h){
        //if element is in namespace ns
        if (ns == container_of(cursor, nsd_t, n_link)){
            //removes from active ns list, adds to free ns list
            list_del(&ns->n_link);
            list_add(&ns->n_link, &pid_nsFree_h);
        }
    }
};