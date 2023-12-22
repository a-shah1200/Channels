#include "linked_list.h"

// Creates and returns a new list
list_t* list_create()
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    list_t* li = malloc(sizeof(list_t));
    li->head = malloc(sizeof(list_node_t));
    li->head->next=NULL;
    li->head->prev=NULL;
    li->head->data=NULL;
    li->count=0;
    return li;
}

// Destroys a list
void list_destroy(list_t* list)
{
    while (list->count>0){
        list_remove(list,list_begin(list));
    }
    free(list->head);
    free(list);
   
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
}

// Returns beginning of the list
list_node_t* list_begin(list_t* list)
{
     if (list->head->next!=NULL){
        return list->head->next;
    }
 
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return NULL;
}

// Returns next element in the list
list_node_t* list_next(list_node_t* node)
{
    if ((node->next!=NULL) && (node!= NULL)){
        return node->next;
    }
   
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return NULL;
}

// Returns data in the given list node
void* list_data(list_node_t* node){
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return node->data;
}

// Returns the number of elements in the list
size_t list_count(list_t* list){
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return list->count;
}

// Finds the first node in the list with the given data
// Returns NULL if data could not be found
list_node_t* list_find(list_t* list, void* data)
{
    list_node_t* node=list_begin(list);
    while (node!=NULL){
        if ((node->data==data)&&(node->data!=NULL)){
            return node;
        }
        node=list_next(node);
    }

    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return NULL;
}

// Inserts a new node in the list with the given data
void list_insert(list_t* list, void* data)
{   
    
    list_node_t* node=malloc(sizeof(list_node_t));
    list_node_t* head=list->head;
    node->next=head->next;
    node->prev=head;
    if ((head->next)!=NULL){
        head->next->prev=node;
    }
    head->next=node;
    node->data=data;
    list->count=list->count+1;
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
}

// Removes a node from the list and frees the node resources
void list_remove(list_t* list, list_node_t* node)
{   
    if ((node!=list->head) && (node!=NULL)){
        node->prev->next=node->next;
        if ((node->next)!=NULL){
            node->next->prev=node->prev;
        }
        node->next=NULL;
        node->prev=NULL;
        list->count=list->count-1;
        free(node);
    }


    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    }

// Executes a function for each element in the list
void list_foreach(list_t* list, void (*func)(void* data))
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
}