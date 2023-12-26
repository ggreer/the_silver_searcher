#ifndef ALLOC_LIST_H
#define ALLOC_LIST_H

struct alloc_node{
    void *data;
    struct alloc_node *next;
};

struct alloc_list{
    struct alloc_node *head;
};

typedef struct alloc_node alloc_node;
typedef struct alloc_list alloc_list;

void alloc_list_init(alloc_list *list);
int alloc_list_add(alloc_list *list, void *data);
int alloc_list_pop(alloc_list *list, void **data);
void alloc_list_destroy_clean(alloc_list *list);
void alloc_list_destroy(alloc_list *list);

#endif