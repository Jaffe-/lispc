
List* make_list();
void list_copy_new(List*, List*);
void list_append(List*, Value*);
int list_find(List*, Value*);
void list_destruct(List*);
