#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lisp.h"
#include "list.h"
#include "operators.h"

int compare_values(Value* a, Value* b) 
{
    // First of all, two values are identical if they point to the same data
    if (a->data == b->data) return 1;

    // Secondly, types have to match
    if (a->type == b->type)
	switch (a->type) {

	case TYPE_NUMBER:
	    return *(int*)a->data == *(int*)b->data;
	case TYPE_SYMBOL:
	    return (strcmp((char*)a->data, (char*)b->data) == 0);
	case TYPE_BINDING:
	    // Bindings are equal when bound values are equal
	    return compare_values(((Binding*)a->data)->value, ((Binding*)b->data)->value);
	case TYPE_PROCEDURE:
	    // Procedures are generally not equal
	    return 0;
	case TYPE_CONS:
	    // Cons pairs are equal when both CARs and CDRs are equal:
	    return 
		compare_values(((Cons*)a->data)->car, ((Cons*)b->data)->car) &&
		compare_values(((Cons*)a->data)->cdr, ((Cons*)b->data)->cdr);
	}
    return 0;
}	

Value* environment_lookup(List* environment, char* name)
{
    Node* current = environment->first;
    Binding* current_binding;
    for (int i = 0; i < environment->length; i++) {
	current_binding = (Binding*)current->value->data;
	if (!strcmp(current_binding->symbol, name)) 
	    return current_binding->value;
	current = current->next;
    }
    char* errorstring = (char*)malloc(sizeof(char) * (strlen(name) + 30));
    strcpy(errorstring, "unbound variable ");
    strcpy(errorstring + strlen(errorstring), name);
    return make_value(TYPE_ERROR, errorstring);
}

int* allocate_integer(int value)
{
    int* integer = (int*)malloc(sizeof(int));
    *integer = value;
    return integer;
}


Cons* make_cons(Value* car, Value* cdr)
{
    Cons* cons = (Cons*)malloc(sizeof(Cons));
    cons->car = car;
    cons->cdr = cdr;
    return cons;
}

Value* make_value(int type, void* data)
{
    Value* value = (Value*)malloc(sizeof(Value));
    value->type = type;
    value->data = data;
    return value;
}

Binding* make_binding(Value* variable, Value* value)
{
    Binding* binding = (Binding*)malloc(sizeof(Binding));
    binding->symbol = variable->data;
    binding->value = (Value*)malloc(sizeof(Value));
    binding->value->type = value->type;
    binding->value->data = value->data;
    return binding;
}

List* make_binding_list(List* variables, List* values)
{
    List* binding_list = make_list();
    Node* current_variable = variables->first;
    Node* current_value = values->first;
    for (int i = 0; i < variables->length; i++) {
	list_append(binding_list, make_value(TYPE_BINDING, make_binding(current_variable->value, current_value->value)));
	current_variable = current_variable->next;
	current_value = current_value->next;
    }
    return binding_list;
}

Procedure* make_procedure(List* variables, Value* code, List* parent_environment)
{
    Procedure* procedure = (Procedure*)malloc(sizeof(Procedure));
    procedure->type = PROCEDURE_LAMBDA;
    procedure->free_variables = variables;
    procedure->code = code;
    procedure->environment = parent_environment;
    return procedure;
}

Value* apply(Procedure* procedure, List* arguments, List* environment)
{
    if (procedure->type == PROCEDURE_LAMBDA || (procedure->type == PROCEDURE_PRIMITIVE && procedure->free_variables->length != 0)) 
	if (procedure->free_variables->length != arguments->length) 
	    return make_value(TYPE_ERROR, "procedure given wrong number of arguments");
    if (procedure->type == PROCEDURE_PRIMITIVE) 
	return ((Value* (*)(List*))procedure->code)(arguments);
    List* new_environment  = make_binding_list(procedure->free_variables, arguments);
    list_copy_new(new_environment, procedure->environment);
    list_copy_new(new_environment, environment);
    return eval(procedure->code, new_environment);
}

Value* eval(Value* expression, List* environment)
{
    if (expression->type == TYPE_NUMBER || expression->type == TYPE_ERROR) 
	return expression;
    if (expression->type == TYPE_SYMBOL && !strcmp(expression->data, "NIL")) 
	return expression;
    if (expression->type == TYPE_SYMBOL) 
	return environment_lookup(environment, expression->data);
    if (expression->type == TYPE_LIST) {
	List* list = (List*)expression->data;
	if (list->length == 0) 
	    return expression;
	if (list->first->value->type == TYPE_SYMBOL) { 
	    char* symbol = (char*)list->first->value->data;
	   
	    // check for operator match	    
	    for (int i = 0; i < num_operators; i++) {
		if (!strcmp(operators[i].name, symbol)) {	
		    List* arguments = make_list();
		    arguments->length = list->length - 1;
		    arguments->first = list->first->next;
		    arguments->last = list->last;
		    return apply_operator(&operators[i], arguments, environment);
		}
	    }	
	}
    
	if (list->first->value->type == TYPE_PROCEDURE) {
	    List* argument_list = make_list();
	    argument_list->length = list->length - 1;
	    argument_list->first = list->first->next;
	    argument_list->last = list->last;
	    return apply((Procedure*)(list->first->value->data), argument_list, environment);
	}
    
	else { 
	    List* evaluated_list = make_list(); 
	    Node* current = list->first;
	    for (int i = 0; i < list->length; i++) {
		Value* element = eval(current->value, environment);
		if (element->type == TYPE_ERROR) return element;
		list_append(evaluated_list, element);
		current = current->next;
	    }
	    return eval(make_value(TYPE_LIST, evaluated_list), environment);
	}
    }
}

void test_repl()
{
    char expression[1000];
    List* env = setup_environment();

    Value* result;
    while(1) {
	printf("LISP> ");
	if (!strcmp(fgets(expression, sizeof(expression), stdin), "QUIT\n")) break;
	expression[strlen(expression) - 1] = 0;
	result = eval(parse_string(expression), env);
	value_print(result);
	printf("\n");
    }
}

int main()
{
    test_repl();
    return 0;	
}
