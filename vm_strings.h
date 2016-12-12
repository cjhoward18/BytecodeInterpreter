#ifndef VM_STRINGS_H_
#define VM_STRINGS_H_

#include <stddef.h>
#include <stdbool.h>

typedef struct string {
	size_t length; // does not count the '\0' on end
	char str[];
	/* the string starts at the end of fixed fields; this field
	 * does not take any room in the structure; it's really just a
	 * label for the element beyond the length field. So, there is no
	 * need set this field. You must, however, copy strings into it.
	 * You cannot set p->str = "foo";
	 * Must terminate with '\0';
	 */
} String;

static String* NIL_STRING = NULL;

// You need to implement this function 
String *String_alloc(size_t length);

// You don't have to, but I have defined some useful functions
// used by my VM:
String *String_new(char *s);
String *String_dup(String *orig);
String *String_from_char(char c);
String *String_add(String *s, String *t);
String *String_from_int(int value);

bool String_eq(String *s, String *t);
bool String_neq(String *s, String *t);
bool String_gt(String *s, String *t);
bool String_ge(String *s, String *t);
bool String_lt(String *s, String *t);
bool String_le(String *s, String *t);
int String_len(String *s);

#endif
