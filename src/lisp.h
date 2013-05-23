#define TYPE_NUMBER 0
#define TYPE_SYMBOL 1
#define TYPE_LIST 2
#define TYPE_PROCEDURE 3
#define TYPE_BINDING 4
#define TYPE_ERROR 5
#define TYPE_CONS 6
#define TYPE_OPERATOR 7

#define PROCEDURE_PRIMITIVE 0
#define PROCEDURE_LAMBDA 1

#define EVAL 0
#define NO_EVAL 1

#define OPERATION_PLUS 0
#define OPERATION_MINUS 1
#define OPERATION_MULTIPLY 2

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
    Value* car;
    Value* cdr;
} Cons;


// for operator handling

typedef struct {
    char* name;
    int num_arguments;
    char argument_flags[3];
    Value* (*function) (List*, List*);
} Operator;

int compare_values(Value*, Value*); 
Value* environment_lookup(List*, char*);
Cons* make_cons(Value*, Value*);
Value* make_value(int, void*);
Binding* make_binding(Value*, Value*);
List* make_binding_list(List*, List*);
Procedure* make_procedure(List*, Value*, List*);
Value* apply(Procedure*, List*, List*);
Value* eval(Value*, List*);
List* setup_environment();
void value_print(Value*);
void list_print(Cons*);
int check_number(char*);
int check_symbol(char*);
int check_list(char*);
int strcount(char*, char);
char* string_uppercase(char*);
Value* parse_string(char*);
void list_append_parsed_string(List*, char*, int, int);
List* parse_list_string(char*);
int* allocate_integer(int);

#define num_operators 9

extern Operator operators[];
