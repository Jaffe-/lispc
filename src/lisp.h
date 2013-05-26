#define TYPE_INTEGER 0
#define TYPE_SYMBOL 1
#define TYPE_LIST 2
#define TYPE_PROCEDURE 3
#define TYPE_BINDING 4
#define TYPE_ERROR 5
#define TYPE_OPERATOR 6

#define PROCEDURE_PRIMITIVE 0
#define PROCEDURE_LAMBDA 1

typedef struct {
    int type;
    void* data;
} Value;

typedef struct Node
{
    Value* value;
    struct Node* next;
} Node;

typedef struct
{
    int length;
    struct Node* first;
    struct Node* last;
} List;

typedef struct {
    int type;
    List* environment;
    List* free_variables;
    void* code;
} Procedure;

typedef struct {
    char* symbol;
    Value* value;
} Binding;

typedef struct {
    char* name;
    int num_arguments;
    char argument_flags[3];
    Value* (*function) (List*, List*);
} Operator;

int compare_values(Value*, Value*); 
Value* environment_lookup(List*, char*);
Value* alloc_value(int, void*);
Binding* alloc_binding(Value*, Value*);
List* alloc_binding_list(List*, List*);
Procedure* alloc_procedure(List*, Value*, List*);
Value* apply(Procedure*, List*, List*);
Value* eval(Value*, List*);
List* setup_environment();
void value_print(Value*);
int check_number(char*);
int check_symbol(char*);
int check_list(char*);
int strcount(char*, char);
char* string_uppercase(char*);
Value* parse_string(char*);
void list_append_parsed_string(List*, char*, int, int);
List* parse_list_string(char*);
int* allocate_integer(int);

extern Operator operators[];
