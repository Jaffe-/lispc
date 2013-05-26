#include <stdio.h>
#include <stdlib.h>
#include "lisp.h"
#include "list.h"
#include "primitives.h"

#define OPERATION_PLUS 0
#define OPERATION_MINUS 1
#define OPERATION_MULTIPLY 2

/* Internal declarations */
Procedure* alloc_primitive_procedure(Value* (*) (List*), int);
void append_primtive_procedure(List*, char*, int num_args, Value* (*) (List*));
Value* primitive_plus(List*);
Value* primitive_eq(List*);
Value* primitive_greater(List*);
Value* primitive_less(List*);
Value* primitive_greatereq(List*);
Value* primitive_lesseq(List*);
Value* primitive_list(List*);
Value* primitive_first(List*);
Value* primitive_rest(List*);
Value* primitive_mod(List*);
Value* primitive_print(List*);
Value* primitive_apply(List*);
Value* primitive_minus(List*);
Value* primitive_multiply(List*);

Procedure* alloc_primitive_procedure(Value* (*code) (List*), int num_args)
{
    Procedure* procedure = (Procedure*)malloc(sizeof(Procedure));

    procedure->type = PROCEDURE_PRIMITIVE;
    procedure->environment = NULL;
    procedure->free_variables = alloc_list();
    procedure->free_variables->length = num_args;
    procedure->code = code;
    return procedure;
}

void append_primitive_procedure(List* env, char* name, int num_args, Value* (*code) (List*))
{
    list_append(env, alloc_value(TYPE_BINDING, alloc_binding(alloc_value(TYPE_SYMBOL, name), 
							   alloc_value(TYPE_PROCEDURE, alloc_primitive_procedure(code, num_args)))));
}

List* setup_environment()
{
    List* env = alloc_list();

    // fill up 'em procedures ...
    append_primitive_procedure(env, "+", 0, &primitive_plus);
    append_primitive_procedure(env, "=", 0, &primitive_eq);
    append_primitive_procedure(env, "LIST", 0, &primitive_list);
    append_primitive_procedure(env, "FIRST", 1, &primitive_first);
    append_primitive_procedure(env, "REST", 1, &primitive_rest);
    append_primitive_procedure(env, "%", 2, &primitive_mod);
    append_primitive_procedure(env, "PRINT", 1, &primitive_print);
    append_primitive_procedure(env, "APPLY", 2, &primitive_apply);
    append_primitive_procedure(env, ">", 2, &primitive_greater);
    append_primitive_procedure(env, "<", 2, &primitive_less);
    append_primitive_procedure(env, "<=", 2, &primitive_lesseq);
    append_primitive_procedure(env, ">=", 2, &primitive_greatereq);
    append_primitive_procedure(env, "-", 0, &primitive_minus);
    append_primitive_procedure(env, "*", 0, &primitive_multiply);
    return env;
}

Value* apply_arithmetic_primitive(List* arguments, int operation)
{
    Node* current_argument = arguments->first;

    int* result = (int*)malloc(sizeof(int));
    for (int i = 0; i < arguments->length; i++) {
	if (current_argument->value->type != TYPE_INTEGER)
	    return alloc_value(TYPE_ERROR, "expects integer arguments");
	if (i == 0) *result = *(int*)current_argument->value->data;
	else {
	    if (operation == OPERATION_PLUS) 
		*result += *(int*)(current_argument->value->data);
	    if (operation == OPERATION_MINUS) 
		*result -= *(int*)current_argument->value->data;
	    if (operation == OPERATION_MULTIPLY) 
		*result *= *(int*)current_argument->value->data;
	}
	current_argument = current_argument->next;
    }
    return alloc_value(TYPE_INTEGER, result);
}

Value* primitive_plus(List* arguments)
{
    return apply_arithmetic_primitive(arguments, OPERATION_PLUS);
}

Value* primitive_minus(List* arguments)
{
    return apply_arithmetic_primitive(arguments, OPERATION_MINUS);
}

Value* primitive_multiply(List* arguments)
{
    return apply_arithmetic_primitive(arguments, OPERATION_MULTIPLY);
}

Value* primitive_eq(List* arguments)
{
    Node* current_argument = arguments->first;
    for (int i = 0; i < arguments->length - 1; i++) {
	if (!compare_values(current_argument->value, current_argument->next->value))
	    return alloc_value(TYPE_SYMBOL, "NIL");
	current_argument = current_argument->next;
    }
    return alloc_value(TYPE_SYMBOL, "T");
}

Value* primitive_less(List* arguments)
{
    Value* a = arguments->first->value;
    Value* b = arguments->last->value;
    if (a->type != TYPE_INTEGER || b->type != TYPE_INTEGER) 
	return alloc_value(TYPE_ERROR, "arguments not integers");
    else
	return alloc_value(TYPE_SYMBOL, (*(int*)a->data < *(int*)b->data) ? "T" : "NIL"); 
}

Value* primitive_greater(List* arguments)
{
    List lst = {2, arguments->last, arguments->first};
    return primitive_less(&lst);
}

Value* primitive_lesseq(List* arguments)
{
    return primitive_less(arguments) || primitive_eq(arguments);
}

Value* primitive_greatereq(List* arguments)
{
    return primitive_greater(arguments) || primitive_eq(arguments);
}

Value* primitive_list(List* arguments)
{
    return alloc_value(TYPE_LIST, list_copy(arguments));
}

Value* primitive_first(List* arguments)
{
    Value* arg = arguments->first->value;

    if (arg->type != TYPE_LIST)
	alloc_value(TYPE_ERROR, "expects list");
    else {
	List* lst = (List*)arg->data;
	if (lst->length == 0)
	    return alloc_value(TYPE_SYMBOL, "NIL");
	else 
	    return lst->first->value;
    }
}

Value* primitive_rest(List* arguments)
{
    Value* arg = arguments->first->value;

    if (arg->type != TYPE_LIST)
	alloc_value(TYPE_ERROR, "expects list");
    else {
	List* lst = (List*)arg->data;
	if (lst->length <= 1)
	    return arg;
	else {
	    List* rest_lst = alloc_list();
	    rest_lst->length = lst->length - 1;
	    rest_lst->first = lst->first->next;
	    rest_lst->last = lst->last;
	    return alloc_value(TYPE_LIST, rest_lst);
	}
    }
}

Value* primitive_apply(List* arguments)
{
    Value* proc = arguments->first->value;
    Value* list = arguments->last->value;
    List env = {};
    
    if (proc->type != TYPE_PROCEDURE)
	return alloc_value(TYPE_ERROR, "APPLY expects a procedure as first argument.");
    if (list->type != TYPE_LIST)
	return alloc_value(TYPE_ERROR, "APPLY expects list as second argument.");
    return apply((Procedure*)proc->data, (List*)list->data, &env); 
}

Value* primitive_mod(List* arguments)
{
    Value* a = arguments->first->value;
    Value* b = arguments->first->next->value;

    if (a->type != TYPE_INTEGER || b->type != TYPE_INTEGER)
	return alloc_value(TYPE_ERROR, "MOD requires numeric arguments");
    return alloc_value(TYPE_INTEGER, allocate_integer(*(int*)a->data % *(int*)b->data));
}

Value* primitive_print(List* arguments)
{
    value_print(arguments->first->value); printf("\n");
    return arguments->first->value;
}
