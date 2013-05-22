#include "lisp.h"
#include <stdio.h>
#include <string.h>

void value_print(Value* value)
{
    switch (value->type) {
    case TYPE_NUMBER:
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
    case TYPE_CONS:
    {
	Cons* cons = (Cons*)value->data;
	printf("(");
	if (cons->cdr->type == TYPE_CONS) list_print(cons);
	else {
	    value_print(cons->car);
	    printf(" . ");
	    value_print(cons->cdr);
	}
    }
    printf(")");
    break;
    }
}

void list_print(Cons* list)
{
    while (list->cdr->type == TYPE_CONS) {
	value_print(list->car);
	printf(" ");
	list = list->cdr->data;
    } 
    value_print(list->car);
    if (!(list->cdr->type == TYPE_SYMBOL && !strcmp(list->cdr->data, "NIL"))) {
	printf(" . ");
	value_print(list->cdr);
    }
}


