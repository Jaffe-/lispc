#include <stdio.h>
#include <stdlib.h>
#include "lisp.h"
#include "list.h"
#include "primitives.h"

Procedure* make_primitive_procedure(Value* (*code) (List*), int num_args)
{
    Procedure* procedure = (Procedure*)malloc(sizeof(Procedure));

    procedure->type = PROCEDURE_PRIMITIVE;
    procedure->environment = NULL;
    procedure->free_variables = make_list();
    procedure->free_variables->length = num_args;
    procedure->code = code;
    return procedure;
}

void append_primitive_procedure(List* env, char* name, int num_args, Value* (*code) (List*))
{
    list_append(env, make_value(TYPE_BINDING, make_binding(make_value(TYPE_SYMBOL, name), make_value(TYPE_PROCEDURE, make_primitive_procedure(code, num_args)))));
}

List* setup_environment()
{
    List* env = make_list();

    append_primitive_procedure(env, "+", 0, &primitive_plus);
    append_primitive_procedure(env, "EQ", 0, &primitive_eq);
    append_primitive_procedure(env, "CONS", 2, &primitive_cons);
    append_primitive_procedure(env, "CAR", 1, &primitive_car);
    append_primitive_procedure(env, "CDR", 1, &primitive_cdr);
    append_primitive_procedure(env, "LIST", 0, &primitive_list);
    append_primitive_procedure(env, "MOD", 2, &primitive_mod);
    append_primitive_procedure(env, "PRINT", 1, &primitive_print);
//	append_primitive_procedure(env, "AND", 0, &primitive_and);
//	append_primitive_procedure(env, "OR", 0, &primitive_or);
//	append_primitive_procedure(env, "NOT", 1, &primitive_not);
//	append_primitive_procedure(env, "XOR", 0, &primitive_xor);
//	append_primitive_procedure(env, ">", 2, &primitive_greater);
//	append_primitive_procedure(env, "<", 2, &primitive_less);
    append_primitive_procedure(env, "-", 0, &primitive_minus);
    append_primitive_procedure(env, "*", 0, &primitive_multiply);
    return env;
}

Value* apply_arithmetic_primitive(List* arguments, int operation)
{
    Node* current_argument = arguments->first;

    int* result = (int*)malloc(sizeof(int));
    for (int i = 0; i < arguments->length; i++) {
	if (current_argument->value->type != TYPE_NUMBER)
	    return make_value(TYPE_ERROR, "expects integer arguments");
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
    return make_value(TYPE_NUMBER, result);
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
	    return make_value(TYPE_SYMBOL, "NIL");
	current_argument = current_argument->next;
    }
    return make_value(TYPE_SYMBOL, "T");
}

Value* primitive_cons(List* arguments)
{
    Value* car = arguments->first->value;
    Value* cdr = arguments->first->next->value;
    return make_value(TYPE_CONS, make_cons(car, cdr));
}

Value* primitive_car(List* arguments)
{
    Value* arg = arguments->first->value;
    if (arg->type != TYPE_CONS) {
	printf("%d", arg->type);
	return make_value(TYPE_ERROR, "CAR expects an argument of type CONS");
    }
    return ((Cons*)arg->data)->car;
}

Value* primitive_cdr(List* arguments)
{
    Value* arg = arguments->first->value;
    if (arg->type != TYPE_CONS)
	return make_value(TYPE_ERROR, "CDR expects an argument of type CONS");
    return ((Cons*)arg->data)->cdr;
}

Value* primitive_list(List* arguments)
{
    Node* current_argument = arguments->first;
    if (arguments->length == 0) return make_value(TYPE_SYMBOL, "NIL");
    else {
	List* rest = make_list();
	rest->length = arguments->length - 1;
	rest->first = current_argument->next;
	rest->last = arguments->last;
	List* cons_args = make_list();
	Node* next_cons = (Node*)malloc(sizeof(Node));
	cons_args->length = 2;
	cons_args->first = current_argument;
	cons_args->first->next = next_cons;
	next_cons->value = primitive_list(rest);
	return primitive_cons(cons_args);
    }
}

Value* primitive_mod(List* arguments)
{
    Value* a = arguments->first->value;
    Value* b = arguments->first->next->value;

    if (a->type != TYPE_NUMBER || b->type != TYPE_NUMBER)
	return make_value(TYPE_ERROR, "MOD requires numeric arguments");
    return make_value(TYPE_NUMBER, allocate_integer(*(int*)a->data % *(int*)b->data));
}

Value* primitive_print(List* arguments)
{
    value_print(arguments->first->value); printf("\n");
    return arguments->first->value;
}
