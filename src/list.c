#include <stdlib.h>
#include "lisp.h"
#include "list.h"

List* make_list() 
{
    List* list = (List*)malloc(sizeof(List));
    list->length = 0;
    list->first = NULL;
    list->last = NULL;
    return list;
}

void list_copy_new(List* dest, List* source) 
{
    Node* current = source->first;
    for (int i = 0; i < source->length; i++) {
	if (!list_find(dest, current->value))
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
    Node* current = list->first;
    for (int i = 0; i < list->length; i++) {
	if (compare_values(current->value, item)) return i;
	current = current->next;
    }
    return 0;
}

void list_destruct(List* list)
{
    Node* current = list->first;
    for (int i = 0; i < list->length; i++) {
	free(current->value);
	free(current);
    }
    free(list);
}
