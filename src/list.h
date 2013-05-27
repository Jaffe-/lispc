
List* alloc_list();
Node* list_nth_node(List*, int);
void list_delete(List*, int);
List* list_copy(List*);
List* list_copy_omit(List*, int);
void list_copy_new(List*, List*);
void list_append(List*, Value*);
int list_find(List*, Value*);
void list_node_free(Node*);
void list_destruct(List*);
List list_pop(List*);
