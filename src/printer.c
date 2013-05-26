#include <stdio.h>
#include <string.h>
#include "lisp.h"
#include "list.h"

void value_print(Value* value)
{
    switch (value->type) {
    case TYPE_INTEGER:
	printf("%i", *(int*)value->data);
	break;
    case TYPE_SYMBOL:
	printf("%s", (char*)value->data);
	break;
    case TYPE_PROCEDURE:
	printf("[PROCEDURE]");
	break;
    case TYPE_ERROR:
	printf("ERROR! %s", (char*)value->data);
	break;
    case TYPE_LIST: 
    {
	List* lst = (List*)value->data;
	Node* current = lst->first;
	printf("(");
	for (int i = 0; i < lst->length; i++) {
	    value_print(current->value);
	    if (i < lst->length - 1) 
		printf(" ");
	    current = current->next;
	}
	printf(")");
	break;
    }
    }
}

