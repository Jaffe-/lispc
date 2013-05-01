#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lisp.h"

int compare_values(Value* a, Value* b) 
{
	if (a->type == b->type)
		switch (a->type) {
			case TYPE_NUMBER:
				return *(int*)a->data == *(int*)b->data;
			case TYPE_SYMBOL:
				return (strcmp((char*)a->data, (char*)b->data) == 0);
			case TYPE_BINDING:
				return (strcmp(((Binding*)a->data)->symbol, ((Binding*)b->data)->symbol));
			case TYPE_PROCEDURE:
				return 0;
			case TYPE_CONS:
				return ((Cons*)a->data)->car->type == ((Cons*)b->data)->car->type 
					&& ((Cons*)a->data)->cdr->type == ((Cons*)b->data)->cdr->type 
					&& ((Cons*)a->data)->car->data == ((Cons*)b->data)->car->data
					&& ((Cons*)a->data)->cdr->data == ((Cons*)b->data)->cdr->data;
		}
	return 0;
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

List* make_list() 
{
	List* list = (List*)malloc(sizeof(List));
	list->length = 0;
	list->first = NULL;
	list->last = NULL;
	return list;
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
			Node* current = operators->first;
	
			// check for operator match
			for (int i = 0; i < operators->length; i++) {
				Operator* operator = (Operator*)current->value->data;
				if (!strcmp(operator->name, symbol)) {	
					List* arguments = make_list();
					arguments->length = list->length - 1;
					arguments->first = list->first->next;
					arguments->last = list->last;
					return apply_operator(operator, arguments, environment);
				}
				current = current->next;
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
	operators = make_operator_list();

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
