#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lisp.h"
#include "list.h"

int check_number(char* str) 
{
    for (int i = 0; i < strlen(str); i++) {
	if (strpbrk(str + i, "0123456789") == NULL)
	    return 0;
    }
    return 1;
}

int check_symbol(char* str)
{
    return strchr(str, '(') == 0 && strchr(str, ')') == 0 && strchr(str, ' ') == 0;
}

int strcount(char* str, char c)
{
    int count = 0;
    for (int i = 0; i < strlen(str); i++) 
	if (str[i] == c) count++;
    return count;
}

int check_list(char* str)
{
    return strcount(str, '(') == strcount(str, ')');
}

char* string_uppercase(char* string)
{
    for (int i = 0; i < strlen(string); i++) 
	string[i] = toupper(string[i]);
    return string;
}

int check_quote(char* str)
{
    return *str == '\'';
}

void strip_spaces(char* string)
{
    int j = 0; int spaces = 0;
    int length = strlen(string);
    for (int i = 0; i < length; i++) {
	if (string[i] != ' ' && string[i] != '\n' && string[i] != '\t') {
	    spaces = 0;
	    string[j++] = string[i];
	}
	else {
	    spaces++;
	    if (spaces == 1 && i > 1 && i < length - 2 && string[i-1] != '(' && string[i+1] != ')') 
		string[j++] = ' ';
	}
    }
    string[j] = 0;
}

Value* parse_string(char* string) 
{
    Value* value;
    if (check_quote(string)) {
	List* quote_exp = alloc_list();
	list_append(quote_exp, alloc_value(TYPE_SYMBOL, "QUOTE"));
	list_append(quote_exp, parse_string(string+1));
	value = alloc_value(TYPE_LIST, quote_exp);
    }
    else if (check_number(string)) 
	value = alloc_value(TYPE_INTEGER, allocate_integer(atoi(string)));	
    else if (check_symbol(string)) 
	value = alloc_value(TYPE_SYMBOL, string_uppercase(string));
    else if (check_list(string)) 
	value = alloc_value(TYPE_LIST, parse_list_string(string));
    else
	value = alloc_value(TYPE_ERROR, "Parse error!");		
    return value;
}

void list_append_parsed_string(List* list, char* string, int start, int end)
{
    int length = end - start;
    char* element = (char*)malloc(sizeof(char) * (length + 1));
    strncpy(element, string + start, length);
    element[length] = 0;
    list_append(list, parse_string(element));
}

List* parse_list_string(char* string)
{
    List* expression = alloc_list();
    int length = strlen(string);
    int level = 0;
    int last_break = 1;
    if (strlen(string) == 2) return expression;
    if (string[0] == '(' && string[length - 1] == ')') {
	for (int i = 1; i < length - 1; i++) {
	    switch (string[i]) {
	    case '(':
		level++; break;
	    case ')':
		level--; break;
	    case ' ':
		if (level == 0) {
		    list_append_parsed_string(expression, string, last_break, i);
		    last_break = i+1;
		}
		break;
	    }
	}
	list_append_parsed_string(expression, string, last_break, length - 1);
    }
    return expression;
}





