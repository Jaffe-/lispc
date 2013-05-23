#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "lisp.h"
#include "list.h"
#include "operators.h"

Operator operators[] = {
    {"IF", 3, {EVAL, NO_EVAL, NO_EVAL}, &operator_if},
    {"QUOTE", 1, {NO_EVAL}, &operator_quote},
    {"DEFINE", 2, {NO_EVAL, EVAL}, &operator_define},
    {"\\", 2, {NO_EVAL, NO_EVAL}, &operator_lambda},
    {"SET", 2, {NO_EVAL, EVAL}, &operator_set},
    {"LET", 2, {NO_EVAL, NO_EVAL}, &operator_let},
    {"PROG", 0, {}, &operator_prog},
    {"COND", 0, {}, &operator_cond},
    {"EVAL", 1, {EVAL}, &operator_eval}
};

Value* apply_operator(Operator* operator, List* arguments, List* environment)
{
    Node* current_arg = arguments->first;
    List* evaluated_args = make_list();
    if (operator->num_arguments != 0 && operator->num_arguments != arguments->length) 
	return make_value(TYPE_ERROR, "wrong number of arguments for operator");
    for (int i = 0; i < arguments->length; i++) {
	Value* result = current_arg->value;
	if (operator->num_arguments != 0) { 
	    if ((operator->argument_flags)[i] == EVAL) {	
		result = eval(current_arg->value, environment);
		if (result->type == TYPE_ERROR) return result;
	    }
	}	
	list_append(evaluated_args, result);
	current_arg = current_arg->next;
    }
    return operator->function(evaluated_args, environment);
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
	return make_value(TYPE_ERROR, "DEFINE: variable name must be symbolic");
    list_append(environment, make_value(TYPE_BINDING, make_binding(symbol, value)));
    return symbol;
}

Value* operator_lambda(List* arguments, List* environment)
{
    Value* arglist = arguments->first->value;
    Value* code = arguments->first->next->value;
	
    if (arglist->type != TYPE_LIST)
	return make_value(TYPE_ERROR, "LAMBDA: malformed argument list");
    return make_value(TYPE_PROCEDURE, make_procedure(arglist->data, code, environment));
}

Value* operator_set(List* arguments, List* environment)
{
    Value* symbol = arguments->first->value;
    Value* value = arguments->first->next->value;

    if (!(symbol->type == TYPE_SYMBOL))
	return make_value(TYPE_ERROR, "SET: variable name must be symbolic");
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
	return make_value(TYPE_ERROR, "LET: no variable definition list");
    List* var_def_list = (List*)var_defs->data;
    List* variable_names = make_list();
    List* initial_values = make_list();
    Node* current = ((List*)var_defs->data)->first;

    for (int i = 0; i < var_def_list->length; i++) {
	if (current->value->type == TYPE_LIST) {
	    List* def = (List*)current->value->data;
	    if (def->length != 2 || def->first->value->type != TYPE_SYMBOL)
		return make_value(TYPE_ERROR, "LET: malformed binding list");
	    list_append(variable_names, make_value(TYPE_SYMBOL, def->first->value->data));
	    list_append(initial_values, eval(def->first->next->value, environment));
	}
	else if (current->value->type == TYPE_SYMBOL) {
	    list_append(variable_names, current->value);
	    list_append(initial_values, make_value(TYPE_SYMBOL, "NIL"));
	}
	else return make_value(TYPE_ERROR, "LET: expected symbol or list in binding list");
	current = current->next;
    }
    Procedure* pseudo_proc = make_procedure(variable_names, body, environment);
    List* pseudo_call = make_list();
    list_append(pseudo_call, make_value(TYPE_PROCEDURE, pseudo_proc));
    pseudo_call->length += initial_values->length;
    pseudo_call->first->next = initial_values->first;	
    return eval(make_value(TYPE_LIST, pseudo_call), environment);
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
	    return make_value(TYPE_ERROR, "COND: expects lists of form (test e1 e2 ... en)");
	List* clause = (List*)current_clause->value->data;
	if (clause->length < 2) 
	    return make_value(TYPE_ERROR, "COND: malformed clause");
	result = eval(clause->first->value, environment);
	if (result->type == TYPE_ERROR) return result;
	if (!(result->type == TYPE_SYMBOL && !strcmp(result->data, "NIL"))) {
	    List* prog_exp = make_list();
	    list_append(prog_exp, make_value(TYPE_SYMBOL, "PROG"));
	    prog_exp->first->next = clause->first->next;
	    prog_exp->length = clause->length;	
	    return eval(make_value(TYPE_LIST, prog_exp), environment);
	}
	current_clause = current_clause->next;
    }
    return result;
}

Value* operator_eval(List* arguments, List* environment)
{
    return arguments->first->value;	
}

