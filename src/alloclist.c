#include <stdlib.h>
#include <stdio.h>
#include "alloclist.h"

// initialize allocation list
void alloc_list_init(alloc_list *list){
    list->head = NULL;
}

// try to add a new allocation node to list
int alloc_list_add(alloc_list *list, void *data){
    alloc_node *new_node = NULL;
    if((new_node=(alloc_node*)malloc(sizeof(alloc_node)))==NULL){
        return -1;
    }
    new_node->data = data;
    new_node->next = list->head;
    list->head = new_node;
    return 0;
}

// remove the first element from allocation list
int alloc_list_pop(alloc_list *list, void **data){
    if(list->head==NULL){
        return -1;
    }
    alloc_node *old_node = list->head;
    list->head = list->head->next;
    old_node->next = NULL;
    *data = old_node->data;
    free(old_node);
    return 0;
}

// destroy only the allocation list nodes without freeing what they point to
void alloc_list_destroy_clean(alloc_list *list){
    void *data = NULL;
    while(alloc_list_pop(list,&data)==0){}
}

// destroy the whole allocation list and free all elements it points to
void alloc_list_destroy(alloc_list *list){
    void *data = NULL;
    while(alloc_list_pop(list,&data)==0){
        free(data);
    }
}