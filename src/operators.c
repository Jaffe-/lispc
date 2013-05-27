#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "lisp.h"
#include "list.h"
#include "operators.h"

#define EVAL 0
#define NO_EVAL 1

/* Internal declarations */
Value* operator_if(List*, List*);
Value* operator_define(List*, List*);
Value* operator_set(List*, List*);
Value* operator_quote(List*, List*);
Value* operator_lambda(List*, List*);
Value* operator_let(List*, List*);
Value* operator_prog(List*, List*);
Value* operator_cond(List*, List*);
Value* operator_eval(List*, List*);

Operator operators[] = {
    {"IF", 3, {EVAL, NO_EVAL, NO_EVAL}, &operator_if},
    {"QUOTE", 1, {NO_EVAL}, &operator_quote},
    {"DEF", 2, {NO_EVAL, EVAL}, &operator_define},
    {"\\", 2, {NO_EVAL, NO_EVAL}, &operator_lambda},
    {"SET!", 2, {NO_EVAL, EVAL}, &operator_set},
    {"LET", 2, {NO_EVAL, NO_EVAL}, &operator_let},
    {"DO", 0, {}, &operator_prog},
    {":", 0, {}, &operator_prog},
    {"COND", 0, {}, &operator_cond},
    //{"EVAL", 1, {EVAL}, &operator_eval}
};

Value* apply_operator(Operator* operator, List* arguments, List* environment)
{
    Node* current_arg = arguments->first;
    List evaluated_args = {};
    if (operator->num_arguments != 0 && operator->num_arguments != arguments->length) 
	return alloc_value(TYPE_ERROR, "wrong number of arguments for operator");
    for (int i = 0; i < arguments->length; i++) {
	Value* result = current_arg->value;
	if (operator->num_arguments != 0) { 
	    if ((operator->argument_flags)[i] == EVAL) {	
		result = eval(current_arg->value, environment);
		if (result->type == TYPE_ERROR) return result;
	    }
	}	
	list_append(&evaluated_args, result);
	current_arg = current_arg->next;
    }
    return operator->function(&evaluated_args, environment);
}

Value* operator_if(List* arguments, List* environment)
{
    Value* test_value = arguments->first->value;
    Value* true_exp = arguments->first->next->value;
    Value* false_exp = arguments->first->next->next->value;

    if (test_value->type == TYPE_SYMBOL && !strcmp(test_value->data, "NIL")) 
	return eval(false_exp, environment);
    return eval(true_exp, environment);
}

Value* operator_quote(List* arguments, List* environment)
{
    return arguments->first->value;
}

Value* operator_define(List* arguments, List* environment)
{
    Value* symbol = arguments->first->value;
    Value* value = arguments->first->next->value;

    if (!symbol->type == TYPE_SYMBOL)
	return alloc_value(TYPE_ERROR, "DEFINE: variable name must be symbolic");
    
    list_append(environment, alloc_value(TYPE_BINDING, alloc_binding(symbol, value)));
    return symbol;
}

Value* operator_lambda(List* arguments, List* environment)
{
    Value* arglist = arguments->first->value;
    Value* code = arguments->first->next->value;
	
    if (arglist->type != TYPE_LIST)
	return alloc_value(TYPE_ERROR, "LAMBDA: no argument list");
    Value ellipsis;
    List* arglst = (List*)arglist->data;
    ellipsis.type = TYPE_SYMBOL;
    ellipsis.data = "..";
    int index; 
    int type = PROCEDURE_LAMBDA;
    // If we find an ellipsis, check if it's correctly placed:
    if ((index = list_find(arglst, &ellipsis)) != -1) {
	if (index == arglst->length - 2) {
	    // if so, it will be a procedure of type TYPE_VARIABLE_LAMBDA, and we simply remove the ..
	    type = PROCEDURE_VARLAMBDA;
	    arglst = list_copy_omit(arglst, index);
	}
	else 
	    return alloc_value(TYPE_ERROR, "LAMBDA: malformed argument list");
    }	
    return alloc_value(TYPE_PROCEDURE, alloc_procedure(arglst, code, environment, type));
}

Value* operator_set(List* arguments, List* environment)
{
    Value* symbol = arguments->first->value;
    Value* value = arguments->first->next->value;

    if (!(symbol->type == TYPE_SYMBOL))
	return alloc_value(TYPE_ERROR, "SET: variable name must be symbolic");
    Value* binding_value = environment_lookup(environment, symbol->data);
    if (binding_value->type == TYPE_ERROR) return binding_value;
    binding_value->type = value->type;
    binding_value->data = value->data;
    return symbol;
}

Value* operator_let(List* arguments, List* environment)
{
    Value* var_defs = arguments->first->value;
    Value* body = arguments->first->next->value;

    if (var_defs->type != TYPE_LIST) 
	return alloc_value(TYPE_ERROR, "LET: no variable definition list");
    List* var_def_list = (List*)var_defs->data;
    List variable_names = {0, NULL, NULL};
    List initial_values = {0, NULL, NULL};
    Node* current = ((List*)var_defs->data)->first;

    for (int i = 0; i < var_def_list->length; i++) {
	if (current->value->type == TYPE_LIST) {
	    List* def = (List*)current->value->data;
	    if (def->length != 2 || def->first->value->type != TYPE_SYMBOL)
		return alloc_value(TYPE_ERROR, "LET: malformed binding list");
	    list_append(&variable_names, alloc_value(TYPE_SYMBOL, def->first->value->data));
	    list_append(&initial_values, def->first->next->value);
	}
	else return alloc_value(TYPE_ERROR, "LET: expected list in binding list");
	current = current->next;
    }
    // Create an anonymous procedure to perform the local bindings
    Procedure pseudo_proc;
    pseudo_proc.type = PROCEDURE_LAMBDA;
    pseudo_proc.environment = environment;
    pseudo_proc.free_variables = &variable_names;
    pseudo_proc.code = body;

    // Construct a call to the procedure
    List pseudo_call = {0, NULL, NULL};
    list_append(&pseudo_call, alloc_value(TYPE_PROCEDURE, &pseudo_proc));
    pseudo_call.length += initial_values.length;
    pseudo_call.first->next = initial_values.first;	

    // ... and call it:
    return eval(alloc_value(TYPE_LIST, &pseudo_call), environment);
}

Value* operator_prog(List* arguments, List* environment)
{
    Value* result;
    Node* current = arguments->first;
    for (int i = 0; i < arguments->length; i++) { 
	result = eval(current->value, environment);
	if (result->type == TYPE_ERROR) return result;
	current = current->next;
    }
    return result;
}

Value* operator_cond(List* arguments, List* environment)
{
    Node* current_clause = arguments->first;
    Value* result;

    for (int i = 0; i < arguments->length; i++) {
	if (current_clause->value->type != TYPE_LIST)
	    return alloc_value(TYPE_ERROR, "COND: expects lists of form (test e1 e2 ... en)");
	List* clause = (List*)current_clause->value->data;
	if (clause->length < 2) 
	    return alloc_value(TYPE_ERROR, "COND: malformed clause");
	result = eval(clause->first->value, environment);
	if (result->type == TYPE_ERROR) return result;
	if (!(result->type == TYPE_SYMBOL && !strcmp(result->data, "NIL"))) {
	    List prog_exp = {0, NULL, NULL};
	    list_append(&prog_exp, alloc_value(TYPE_SYMBOL, "DO"));
	    prog_exp.first->next = clause->first->next;
	    prog_exp.length = clause->length;	
	    return eval(alloc_value(TYPE_LIST, &prog_exp), environment);
	}
	current_clause = current_clause->next;
    }
    return result;
}

Value* operator_eval(List* arguments, List* environment)
{
    return arguments->first->value;	
}

