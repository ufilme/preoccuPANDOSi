#include "pandos_const.h"
#include "list.h"
#include "types.h"
#include "pcb.h"

/**
 * @brief Initialize the type_nsFree_h list to contain all the elements of the
 * array static nsd_t type_nsd[MAXPROC]
 * This method will be only called once during data structure initialization
 * 
 */
void initNamespaces();

/**
 * @brief Return the pointer to the namespace descriptor of type type
 * associated with the pcb 
 * 
 * @param p pcb
 * @param type namespace type
 * @return nsd_t* namespace or NULL
 */
nsd_t *getNamespace(pcb_t *p, int type);

/**
 * Associates the process p and all its children with the namespace ns
 * 
 * @param p pcb
 * @param ns namespace
 * @return int false if error else true
 */
int addNamespace(pcb_t *p, nsd_t *ns);

/**
 * @brief Allocate a namespace from the type_nsFree_h list and return
 * to the user, this value can be used for the next calls to refer to
 * this namespace
 * 
 * @param type namespace type
 * @return nsd_t* namespace
 */
nsd_t *allocNamespace(int type);

/**
 * @brief Free a namespace, adding its list pointer (n_link) to the
 * correct type_nsFree_h list
 * 
 * @param ns namespace
 */
void freeNamespace(nsd_t *ns);