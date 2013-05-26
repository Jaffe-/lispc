
List* alloc_list();
List* list_copy(List*);
void list_copy_new(List*, List*);
void list_append(List*, Value*);
int list_find(List*, Value*);
void list_destruct(List*);
