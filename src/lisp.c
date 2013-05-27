#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lisp.h"
#include "list.h"
#include "operators.h"

const char* type_names[] = {"INTEGER", "SYMBOL", "LIST", "PROCEDURE", "BINDING", "ERROR", "OPERATOR"};

int compare_values(Value* a, Value* b) 
{
    // First of all, two values are identical if they point to the same data
    if (a->data == b->data) return 1;

    // Secondly, types have to match
    if (a->type == b->type)
	switch (a->type) {
	    
	case TYPE_INTEGER:
	    return *(int*)a->data == *(int*)b->data;
	case TYPE_SYMBOL:
	    return (strcmp((char*)a->data, (char*)b->data) == 0);
	case TYPE_BINDING:
	    // Bindings are equal when bound values are equal
	    return !strcmp(((Binding*)a->data)->symbol, ((Binding*)b->data)->symbol);
	case TYPE_PROCEDURE:
	    // Procedures are generally not equal
	    return 0;
	case TYPE_LIST:
	{
	    List* lst_a = (List*)a->data;
	    List* lst_b = (List*)b->data;
	    if (lst_a->length != lst_b->length) return 0;
	    Node* node_a = lst_a->first;
	    Node* node_b = lst_b->first;
	    for (int i = 0; i < lst_a->length; i++) {
		if (!compare_values(node_a->value, node_b->value))
		    return 0;
		node_a = node_a->next;
		node_b = node_b->next;
	    }
	    return 1;
	}
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
    return alloc_value(TYPE_ERROR, errorstring);
}

int* allocate_integer(int value)
{
    int* integer = (int*)malloc(sizeof(int));
    *integer = value;
    return integer;
}

Value* alloc_value(int type, void* data)
{
    Value* value = (Value*)malloc(sizeof(Value));
    value->type = type;
    value->data = data;
    return value;
}

Binding* alloc_binding(Value* variable, Value* value)
{
    Binding* binding = (Binding*)malloc(sizeof(Binding));
    binding->symbol = variable->data;
    binding->value = (Value*)malloc(sizeof(Value));
    binding->value->type = value->type;
    binding->value->data = value->data;
    return binding;
}

List* alloc_binding_list(List* variables, List* values)
{
    List* binding_list = alloc_list();
    Node* current_variable = variables->first;
    Node* current_value = values->first;
    for (int i = 0; i < variables->length; i++) {
	list_append(binding_list, alloc_value(TYPE_BINDING, alloc_binding(current_variable->value, current_value->value)));
	current_variable = current_variable->next;
	current_value = current_value->next;
    }
    return binding_list;
}

Procedure* alloc_procedure(List* variables, Value* code, List* parent_environment, int type)
{
    Procedure* procedure = (Procedure*)malloc(sizeof(Procedure));
    procedure->type = (unsigned char)type;
    procedure->free_variables = variables;
    procedure->code = code;
    procedure->environment = parent_environment;
    return procedure;
}

Value* apply(Procedure* procedure, List* arguments, List* environment)
{
    // If procedure is a LAMBDA or PRIMITIVE, argument and variable list lengths must agree
    if (procedure->type == PROCEDURE_LAMBDA || (procedure->type == PROCEDURE_PRIMITIVE && procedure->free_variables->length != 0)) 
	if (procedure->free_variables->length != arguments->length) 
	    return alloc_value(TYPE_ERROR, "procedure given wrong number of arguments");

    // Primitive proceudres are executed by calling the appropriate C function through a function pointer
    if (procedure->type == PROCEDURE_PRIMITIVE) 
	return ((Value* (*)(List*))procedure->code)(arguments);
  
    // If procedure is a VARLAMBDA (.. in parameter list),
    if (procedure->type == PROCEDURE_VARLAMBDA) {
	int required_args = procedure->free_variables->length - 1;
	// Enough arguments supplied?
	if (arguments->length < required_args)
	    return alloc_value(TYPE_ERROR, "procedure given too few arguments");
	else {
	    // Basically we wrap the rest of the arguments into a new list (destructively): 
	    List* rest_args = alloc_list();
	    rest_args->length = arguments->length - required_args;
	    if (arguments->length != required_args) {
		rest_args->first = list_nth_node(arguments, required_args);
		rest_args->last = arguments->last;
	    }
	    Node* rest_arg_node = (Node*)malloc(sizeof(Node));
	    rest_arg_node->value = alloc_value(TYPE_LIST, rest_args);
	    arguments->length = procedure->free_variables->length;
	    list_nth_node(arguments, required_args - 1)->next = rest_arg_node;
	}
    }

    // Make a new environment where the new bindings are added to the parent environment
    List* new_environment  = alloc_binding_list(procedure->free_variables, arguments);
    list_copy_new(new_environment, procedure->environment);
    list_copy_new(new_environment, environment);
 
    // Finally, evaluate the procedure
    return eval(procedure->code, new_environment);
}

Value* eval(Value* expression, List* environment)
{
    //printf("EVAL: "); value_print(expression); printf("\n");
    if (expression->type == TYPE_INTEGER || expression->type == TYPE_ERROR || expression->type == TYPE_PROCEDURE) 
	return expression;
    if (expression->type == TYPE_SYMBOL && !strcmp(expression->data, "NIL")) 
	return expression;
    if (expression->type == TYPE_SYMBOL) 
	return environment_lookup(environment, expression->data);
    
    if (expression->type == TYPE_LIST) {
	List* list = (List*)expression->data;
	if (list->length == 0) 
	    return expression;

	// If the first element is a symbol, it might refer to an operator
	if (list->first->value->type == TYPE_SYMBOL) { 
	    char* symbol = (char*)list->first->value->data;
	   
	    // Check for operator match	    
	    for (int i = 0; i < num_operators; i++) {
		if (!strcmp(operators[i].name, symbol)) {

		    // Call the operator
		    List arguments = list_pop(list);
		    return apply_operator(&operators[i], &arguments, environment);
		}
	    }
	}
       
	// If first element is not an operator, the list is a function call, and each element is evaluated:
	List evaluated_list = {0, NULL, NULL}; 
	Node* current = list->first;
	for (int i = 0; i < list->length; i++) {
	    Value* element = eval(current->value, environment);
	    if (element->type == TYPE_ERROR) return element;
	    list_append(&evaluated_list, element);
	    current = current->next;
	}

	// If the first element evaluated to a procedure, apply the procedure to the arguments:
	if (evaluated_list.first->value->type == TYPE_PROCEDURE) {
	    List arguments = list_pop(&evaluated_list);
	    return apply((Procedure*)evaluated_list.first->value->data, &arguments, environment);
	}
	else 
	    return alloc_value(TYPE_ERROR, "first element of list not operator or procedure");
    }
}

void test_repl()
{
    char expression[8000];
    List* env = setup_environment();
    Value* result;

    FILE* file = fopen("fibmemo.lisp", "r");
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);
    fread(expression, 1, size, file);
    expression[size] = 0;
    strip_spaces(expression);
    printf("%s", expression);
    result = eval(parse_string(expression), env);
    value_print(result);
    printf("\n");
    
    while(1) {
	printf("LISP> ");
	if (!strcmp(fgets(expression, sizeof(expression), stdin), "QUIT\n")) break;
	expression[strlen(expression) - 1] = 0;
	strip_spaces(expression);
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
