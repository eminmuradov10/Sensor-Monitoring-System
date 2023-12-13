/**
 * \author Emin Muradov
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"

/*
 * definition of error codes
 * */
#define DPLIST_NO_ERROR 0
#define DPLIST_MEMORY_ERROR 1 // error due to mem alloc failure
#define DPLIST_INVALID_ERROR 2 //error due to a list operation applied on a NULL list 

#ifdef DEBUG
#define DEBUG_PRINTF(...) 									                                        \
        do {											                                            \
            fprintf(stderr,"\nIn %s - function %s at line %d: ", __FILE__, __func__, __LINE__);	    \
            fprintf(stderr,__VA_ARGS__);								                            \
            fflush(stderr);                                                                         \
                } while(0)
#else
#define DEBUG_PRINTF(...) (void)0
#endif


#define DPLIST_ERR_HANDLER(condition, err_code)                         \
    do {                                                                \
            if ((condition)) DEBUG_PRINTF(#condition " failed\n");      \
            assert(!(condition));                                       \
        } while(0)


/*
 * The real definition of struct list / struct node
 */

struct dplist_node {
    dplist_node_t *prev, *next;
    void *element;
};

struct dplist {
    dplist_node_t *head;

    void *(*element_copy)(void *src_element);

    void (*element_free)(void **element);

    int (*element_compare)(void *x, void *y);
};

/** Create and allocate memory for a new list
 * \param element_copy callback function to duplicate 'element'; If needed allocated new memory for the duplicated element.
 * \param element_free callback function to free memory allocated to element
 * \param element_compare callback function to compare two element elements; returns -1 if x<y, 0 if x==y, or 1 if x>y
 * \return a pointer to a newly-allocated and initialized list.
 */
dplist_t *dpl_create(// callback functions
        void *(*element_copy)(void *src_element),
        void (*element_free)(void **element),
        int (*element_compare)(void *x, void *y)
) {
    dplist_t *list;
    list = malloc(sizeof(struct dplist));
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_MEMORY_ERROR);
    list->head = NULL;
    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    return list;
}

/** Deletes all elements in the list
 * - Every list node of the list needs to be deleted. (free memory)
 * - The list itself also needs to be deleted. (free all memory)
 * - '*list' must be set to NULL.
 * \param list a double pointer to the list
 * \param free_element if true call element_free() on the element of the list node to remove
 */
void dpl_free(dplist_t **list, bool free_element){
	
	if (*list!=NULL){
		if ((*list)->head!=NULL){
			dplist_node_t* next_node=((*list)->head);
			while(next_node->next!=NULL){
				dplist_node_t* current_node=next_node;
				next_node=current_node->next;
				if (free_element){
					(*list)->element_free((void**)&(current_node->element));//*list is a pointer that points to dplist_t
					current_node->element=NULL;								//which has the pointer function prototype
				}
				free(current_node);
			
			}
			if(free_element){
				(*list)->element_free((void**)&(next_node->element));
			}

			free(next_node);
			(*list)->head=NULL;
		}
	}
		free(*list);	//allocated memory spaced is freed, the pointer is set to null
		*list=NULL;
}

/** Returns the number of elements in the list.
 * - If 'list' is is NULL, -1 is returned.
 * \param list a pointer to the list
 * \return the size of the list
 */ 
int dpl_size(dplist_t *list){
	
	int dpl_size=0;
	
	if(list==NULL){		
		return -1;
	}

	if(list->head!=NULL){
		dpl_size=1;
		dplist_node_t* next_node=(list->head);
		while(next_node->next!=NULL){
			dpl_size++;

			dplist_node_t* current_node=next_node;
			next_node=current_node->next;
		}
	}

	return dpl_size;
}

/** Inserts a new list node containing an 'element' in the list at position 'index'
 * - the first list node has index 0.
 * - If 'index' is 0 or negative, the list node is inserted at the start of 'list'.
 * - If 'index' is bigger than the number of elements in the list, the list node is inserted at the end of the list.
 * - If 'list' is is NULL, NULL is returned.
 * \param list a pointer to the list
 * \param element a pointer to the data that needs to be inserted
 * \param index the position at which the element should be inserted in the list
 * \param insert_copy if true use element_copy() to make a copy of 'element' and use the copy in the new list node, otherwise the given element pointer is added to the list
 * \return a pointer to the list or NULL
 */
dplist_t* dpl_insert_at_index(dplist_t *list, void *element, int index, bool insert_copy){
	
	if (list==NULL){
			return NULL;
	}

	dplist_node_t* new_list_node=malloc(sizeof(dplist_node_t));
	DPLIST_ERR_HANDLER(new_list_node == NULL, DPLIST_MEMORY_ERROR);
	if(insert_copy){
			new_list_node->element=list->element_copy(element);
	}else{
			new_list_node->element=element;
	}

	dplist_node_t* searched_node_at_reference;
	if(list->head==NULL){			//the situation when list is present but has no list_nodes
		new_list_node->prev=NULL;
		new_list_node->next=NULL;
		list->head=new_list_node;
	}else if(index<=0){				//the list_node is present at 0, and a new list_node is added before the inserted one
		new_list_node->prev=NULL;
		new_list_node->next=list->head;
		list->head=new_list_node;
		list->head->prev=new_list_node;
	}else{
		searched_node_at_reference=dpl_get_reference_at_index(list,index);	//the index is between 0 and dpl_size(list)-1

		if(index<=(dpl_size(list)-1)){//the index is between 0 and dpl_size(list)-1

			assert(searched_node_at_reference!=NULL);
			new_list_node->prev=searched_node_at_reference->prev;
			new_list_node->next=searched_node_at_reference;

			searched_node_at_reference->prev->next=new_list_node;
			searched_node_at_reference->prev=new_list_node;
		}else{	//if index shows the end of list
				assert(searched_node_at_reference->next==NULL);
				new_list_node->next=NULL;
				new_list_node->prev=searched_node_at_reference;
				searched_node_at_reference->next=new_list_node;
			}
		}
	
	return list;
}


/** Removes the list node at index 'index' from the list.
 * - The list node itself should always be freed.
 * - If 'index' is 0 or negative, the first list node is removed.
 * - If 'index' is bigger than the number of elements in the list, the last list node is removed.
 * - If the list is empty, return the unmodified list.
 * - If 'list' is is NULL, NULL is returned.
 * \param list a pointer to the list
 * \param index the position at which the node should be removed from the list
 * \param free_element if true, call element_free() on the element of the list node to remove
 * \return a pointer to the list or NULL
 */
dplist_t* dpl_remove_at_index(dplist_t *list, int index, bool free_element){

	if(list==NULL){
		return NULL;
	}

	if(list->head==NULL){
		return list;
	}
	
	if(dpl_size(list)==1){
		if(free_element){
			list->element_free((void**)&(list->head->element));
			
		}

		free(list->head);
		list->head=NULL;
		return list;
		
	}
	
	dplist_node_t* middle_node;
	if(index<=0){
		middle_node=list->head;
		list->head=middle_node->next;
		list->head->prev=NULL;

		if(free_element){
			list->element_free((void**)&(middle_node->element));
		}

		free(middle_node);
		return list;
	}

	dplist_node_t* previous_node;
	dplist_node_t* next_node;
	if(index>=(dpl_size(list)-1)){	//freeing last node
		middle_node=dpl_get_reference_at_index(list,(dpl_size(list)-1));
		previous_node=middle_node->prev;
		previous_node->next=NULL;

		if(free_element){
			list->element_free((void**)&(middle_node->element));
		}

		free(middle_node);
		return list;
	}else{							//freeing node in given reference 
		middle_node=dpl_get_reference_at_index(list,index);
		previous_node=middle_node->prev;
		next_node=middle_node->next;
		previous_node->next=next_node;
		next_node->prev=previous_node;

		if(free_element){
			list->element_free((void**)&(middle_node->element));
		}

		free(middle_node);
		return list;

	}

	return list;
}

/** Returns a reference to the list node with index 'index' in the list.
 * - If 'index' is 0 or negative, a reference to the first list node is returned.
 * - If 'index' is bigger than the number of list nodes in the list, a reference to the last list node is returned.
 * - If the list is empty, NULL is returned.
 * - If 'list' is is NULL, NULL is returned.
 * \param list a pointer to the list
 * \param index the position of the node for which the reference is returned
 * \return a pointer to the list node at the given index or NULL
 */
dplist_node_t* dpl_get_reference_at_index(dplist_t *list, int index){

	if(list==NULL||list->head==NULL){
		return NULL;
	}
	
	dplist_node_t* searched_node;
	searched_node=list->head;
	if(index<=0){
		return searched_node;
	}

	int counter=0;
	while(searched_node->next!=NULL){
		searched_node=searched_node->next;
		counter++;
		if(counter>=index){
			return searched_node;
		}
	}

	return searched_node;
}

/** Returns the list element contained in the list node with index 'index' in the list.
 * - return is not returning a copy of the element with index 'index', i.e. 'element_copy()' is not used.
 * - If 'index' is 0 or negative, the element of the first list node is returned.
 * - If 'index' is bigger than the number of elements in the list, the element of the last list node is returned.
 * - If the list is empty, NULL is returned.
 * - If 'list' is NULL, NULL is returned.
 * \param list a pointer to the list
 * \param index the position of the node for which the element is returned
 * \return a pointer to the element at the given index or NULL
 */
void *dpl_get_element_at_index(dplist_t *list, int index){

	if(list==NULL||list->head==NULL){
		return NULL;
	}

	dplist_node_t* searched_node=list->head;
	if(index<=0){
		return searched_node->element;
	}

	int counter=0;
	while(searched_node->next!=NULL){
		searched_node=searched_node->next;
		counter++;
		if(counter>=index){
			return searched_node->element;
		}
	}
	
	return searched_node->element;
}

/** Returns an index to the first list node in the list containing 'element'.
 * - the first list node has index 0.
 * - Use 'element_compare()' to search 'element' in the list, a match is found when 'element_compare()' returns 0.
 * - If 'element' is not found in the list, -1 is returned.
 * - If 'list' is NULL, NULL is returned.
 * \param list a pointer to the list
 * \param element the element to look for
 * \return the index of the element that matches 'element'
 */
int dpl_get_index_of_element(dplist_t *list, void *element){

	if(list==NULL){
		return -1;
	}

	if(element==NULL){
		return -1;
	}

	dplist_node_t* searched_node=list->head;
	if(searched_node==NULL){//or list->head
		return -1;
	}

	int index=0;
	if(list->element_compare(element,searched_node->element)==0){
		return index;
	}
	
	while(searched_node->next!=NULL){
		index++;
		searched_node=searched_node->next;	
		if(list->element_compare(element,searched_node->element)==0){
			return index;
		}
	}

	return -1;
}

/** Returns the element contained in the list node with reference 'reference' in the list.
 * - If the list is empty, NULL is returned.
 * - If 'list' is is NULL, NULL is returned.
 * - If 'reference' is NULL, NULL is returned.
 * - If 'reference' is not an existing reference in the list, NULL is returned.
 * \param list a pointer to the list
 * \param reference a pointer to a certain node in the list
 * \return the element contained in the list node or NULL
 */
void *dpl_get_element_at_reference(dplist_t *list, dplist_node_t *reference){

	if(list==NULL||list->head==NULL){
		return NULL;
	}

	if (reference==NULL){
		return NULL;
	}

	dplist_node_t* searched_node=list->head;
	if(searched_node==reference){
		return reference->element;
	}

	while(searched_node->next!=NULL){
		searched_node=searched_node->next;
		if(searched_node==reference){
			return reference->element;
		}
	}

	return NULL;
}
//*** HERE STARTS THE EXTRA SET OF OPERATORS ***//


/** Returns a reference to the first list node of the list.
 * - If the list is empty, NULL is returned.
 * - If 'list' is is NULL, NULL is returned.
 * \param list a pointer to the list
 * \return a reference to the first list node of the list or NULL
 */
dplist_node_t* dpl_get_first_reference(dplist_t *list){

	if(list==NULL||list->head==NULL){
		return NULL;
	}

	return list->head;
}

/** Returns a reference to the last list node of the list.
 * - If the list is empty, NULL is returned.
 * - If 'list' is is NULL, NULL is returned.
 * \param list a pointer to the list
 * \return a reference to the last list node of the list or NULL
 */
dplist_node_t* dpl_get_last_reference(dplist_t *list){

	if(list==NULL||list->head==NULL){
		return NULL;
	}

	return dpl_get_reference_at_index(list,dpl_size(list)-1);
}

/** Returns a reference to the next list node of the list node with reference 'reference' in the list.
 * - If the list is empty, NULL is returned.
 * - If 'list' is is NULL, NULL is returned.
 * - If 'reference' is NULL, NULL is returned.
 * - If 'reference' is not an existing reference in the list, NULL is returned.
 * \param list a pointer to the list
 * \param reference a pointer to a certain node in the list
 * \return a pointer to the node next to 'reference' in the list or NULL
 */
dplist_node_t* dpl_get_next_reference(dplist_t *list, dplist_node_t *reference){

	if(list==NULL||list->head==NULL){
		return NULL;
	}

	if(reference==NULL){
		return NULL;
	}
	
	dplist_node_t* searched_node=list->head;
	if(searched_node==reference){
		if(searched_node->next!=NULL){
			return reference->next;
		}			
	}
	
	while(searched_node->next!=NULL){
		searched_node=searched_node->next;
		if(searched_node==reference){
			if(searched_node->next!=NULL){
				return reference->next;
				
			}
		}
	}

	return NULL;
}

/** Returns a reference to the previous list node of the list node with reference 'reference' in 'list'.
 * - If the list is empty, NULL is returned.
 * - If 'list' is is NULL, NULL is returned.
 * - If 'reference' is NULL, NULL is returned.
 * - If 'reference' is not an existing reference in the list, NULL is returned.
 * \param list a pointer to the list
 * \param reference a pointer to a certain node in the list
 * \return pointer to the node previous to 'reference' in the list or NULL
 */
dplist_node_t* dpl_get_previous_reference(dplist_t *list, dplist_node_t *reference){

	if(list==NULL||list->head==NULL){
		return NULL;
	}

	if(reference==NULL){
		return NULL;
	}

	dplist_node_t* searched_node=list->head;
	if(searched_node==reference){
		return NULL;
	}
	
	while(searched_node->next!=NULL){
		searched_node=searched_node->next;
		if(searched_node==reference){
			return reference->prev;
		}
	}

	return NULL;
}

/** Returns a reference to the first list node in the list containing 'element'.
 * - If the list is empty, NULL is returned.
 * - If 'list' is is NULL, NULL is returned.
 * - If 'element' is not found in the list, NULL is returned.
 * \param list a pointer to the list
 * \param element a pointer to an element
 * \return the first list node in the list containing 'element' or NULL
 */
dplist_node_t* dpl_get_reference_of_element(dplist_t *list, void *element){

	if(list==NULL||list->head==NULL){
		return NULL;
	}

	if(element==NULL){
		return NULL;
	}

	dplist_node_t* searched_node=list->head;
	if(list->element_compare(element,searched_node->element)==0){
		return searched_node;
	}

	while(searched_node->next!=NULL){
		searched_node=searched_node->next;
		if(list->element_compare(element,searched_node->element)==0){
			return searched_node;
		}
		
	}

	return NULL;
}

/** Returns the index of the list node in the list with reference 'reference'.
 * - the first list node has index 0.
 * - If the list is empty, -1 is returned.
 * - If 'list' is is NULL, -1 is returned.
 * - If 'reference' is NULL, -1 returned.
 * - If 'reference' is not an existing reference in the list, -1 is returned.
 * \param list a pointer to the list
 * \param reference a pointer to a certain node in the list
 * \return the index of the given reference in the list
 */
int dpl_get_index_of_reference(dplist_t *list, dplist_node_t *reference){

	if(list==NULL||list->head==NULL){
		return -1;
	}
	if(reference==NULL){
		return -1;
	}

	dplist_node_t* searched_node=list->head;
	int index=0;
	if(searched_node==reference){
		return index;
	}

	while(searched_node->next!=NULL){
		searched_node=searched_node->next;
		index++;

		if(reference==searched_node){
			return index;
		}
	}

	return -1;
}

/** Inserts a new list node containing an 'element' in the list at position 'reference'.
 * - If 'list' is is NULL, NULL is returned.
 * - If 'reference' is NULL, NULL is returned (nothing is inserted).
 * - If 'reference' is not an existing reference in the list, 'list' is returned (nothing is inserted).
 * \param list a pointer to the list
 * \param element a pointer to an element
 * \param reference a pointer to a certain node in the list
 * \param insert_copy if true use element_copy() to make a copy of 'element' and use the copy in the new list node, otherwise the given element pointer is added to the list
 * \return a pointer to the list or NULL
 */
dplist_t* dpl_insert_at_reference(dplist_t *list, void *element, dplist_node_t *reference, bool insert_copy){
	if(list==NULL||list->head==NULL||reference==NULL){
		return NULL;
	}
	
	if(dpl_get_index_of_reference(list,reference)!=-1){
		return dpl_insert_at_index(list, element, dpl_get_index_of_reference(list, reference), insert_copy);
	}

	return list;
}

/** Inserts a new list node containing 'element' in the sorted list and returns a pointer to the new list.
 * - The list must be sorted or empty before calling this function.
 * - The sorting is done in ascending order according to a comparison function.
 * - If two members compare as equal, their order in the sorted array is undefined.
 * - If 'list' is is NULL, NULL is returned.
 * \param list a pointer to the list
 * \param element a pointer to an element
 * \param insert_copy if true use element_copy() to make a copy of 'element' and use the copy in the new list node, otherwise the given element pointer is added to the list
 * \return a pointer to the list or NULL
 */
dplist_t* dpl_insert_sorted(dplist_t *list, void *element, bool insert_copy){
	if (list==NULL){
		return NULL;
	}

	if(list->head==NULL){
		return dpl_insert_at_index(list, element, 0, insert_copy);
	}

	int counter=0;
	dplist_node_t* current_node=list->head;

	while(list->element_compare(element, current_node->element)>0){
    	current_node=current_node->next;
    	counter++;
  	}

  	return dpl_insert_at_index(list, element, counter, insert_copy);
}

/** Removes the list node with reference 'reference' in the list.
 * - The list node itself should always be freed.
 * - If 'reference' is NULL, NULL is returned (nothing is removed).
 * - If 'reference' is not an existing reference in the list, 'list' is returned (nothing is removed).
 * - If 'list' is is NULL, NULL is returned.
 * \param list a pointer to the list
 * \param reference a pointer to a certain node in the list
 * \param free_element if true call element_free() on the element of the list node to remove
 * \return a pointer to the list or NULL
 */
dplist_t* dpl_remove_at_reference(dplist_t *list, dplist_node_t *reference, bool free_element){
	if(list==NULL||reference==NULL){
		return NULL;
	}

	if(dpl_get_index_of_reference(list,reference)!=-1){
		return dpl_remove_at_index(list, dpl_get_index_of_reference(list, reference), free_element);
	}

	return list;
}

/** Finds the first list node in the list that contains 'element' and removes the list node from 'list'.
 * - If 'element' is not found in 'list', the unmodified 'list' is returned.
 * - If 'list' is is NULL, NULL is returned.
 * \param list a pointer to the list
 * \param element a pointer to an element
 * \param free_element if true call element_free() on the element of the list node to remove
 * \return a pointer to the list or NULL
 */
dplist_t* dpl_remove_element(dplist_t *list, void *element, bool free_element){
	if(list==NULL){
		return NULL;
	}

	dplist_node_t* current_node=list->head;
	int counter=0;

	if((list->element_compare(element,current_node->element))==0){
		return dpl_remove_at_index(list, counter, free_element);
	}
	
	while(current_node->next!=NULL){
		current_node=current_node->next;
		counter++;
		if((list->element_compare(element,current_node->element))==0){
			return dpl_remove_at_index(list, counter, free_element);
		}
	
	}
	
	return list;
}



// ---- you can add your extra operators here ----//
