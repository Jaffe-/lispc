#include <stdlib.h>
#include "lisp.h"
#include "list.h"

List* alloc_list() 
{
    List* list = (List*)malloc(sizeof(List));
    list->length = 0;
    list->first = NULL;
    list->last = NULL;
    return list;
}

Node* list_nth_node(List* lst, int index)
{
    Node* current = lst->first;
    for (int i = 0; i < index; i++)
	current = current->next;
    return current;
}

void list_delete(List* lst, int index)
{
    Node* current = lst->first;
    Node* last;
    int i;
    if (index == 0)
	lst->first = current->next;
    else {
	for (i = 0; i < index; i++) {
	    last = current;
	    current = current->next;
	}
	last->next = current->next;
    }
    if (i == lst->length - 1)
	lst->last = last;
    list_node_free(current);
    lst->length--;
}

List* list_copy(List* source)
{
    List* new = alloc_list();
    Node* current = source->first;
    for (int i = 0; i < source->length; i++) { 
	list_append(new, current->value);
	current = current->next;
    }
    return new;
}

void list_copy_new(List* dest, List* source) 
{
    Node* current = source->first;
    for (int i = 0; i < source->length; i++) {
	if (list_find(dest, current->value) == -1)
	    list_append(dest, current->value);
	current = current->next;
    }
}

void list_append(List* list, Value* new_item)
{
    Node* new = (Node*)malloc(sizeof(Node));
    new->value = (Value*)malloc(sizeof(Value));
    new->value->type = new_item->type;
    new->value->data = new_item->data;
    new->next = NULL;
    if (list->length == 0) 
	list->first = new;
    else
	list->last->next = new;
    list->last = new;
    list->length++;
}

int list_find(List* list, Value* item)
{
    if (list->length == 0) return -1;
    Node* current = list->first;
    for (int i = 0; i < list->length; i++) {
	if (compare_values(current->value, item)) return i;
	current = current->next;
    }
    return -1;
}

void list_node_free(Node* node)
{
    free(node->value);
    free(node);
}

void list_destruct(List* list)
{
    Node* current = list->first;
    for (int i = 0; i < list->length; i++) {
        list_node_free(current);
    }
    free(list);
}

List list_pop(List* list)
{
    List new_head = {
	list->length - 1,
	list->first->next,
	list->last
    };
    return new_head;
}
