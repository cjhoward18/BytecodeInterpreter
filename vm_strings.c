#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "vm_strings.h"
#include <assert.h>

String *String_alloc(size_t length) {
	String *p = (String *)calloc(1, sizeof(String) + (length+1) * sizeof(char));
	p->length = length;
	return p;
}

String *String_new(char *orig) {
	String *s = String_alloc(strlen(orig));
	strcpy(s->str, orig);
	return s;
}

String *String_dup(String *orig) {
	String *s = String_alloc(orig->length);
	strcpy(s->str, orig->str);
	return s;
}

String *String_from_char(char c) {
	char buf[2] = {c, '\0'};
	return String_new(buf);
}

String *String_from_int(int value) {
	char buf[50];
	sprintf(buf,"%d",value);
	return String_new(buf);
}

int String_len(String *s) {
	if (s == NULL) {
		fprintf(stderr, "len() cannot be applied to NULL string object\n");
		return -1;
	}
	return (int)s->length;
}

String *String_add(String *s, String *t) {
	if ( s == NULL && t == NULL) {
		fprintf(stderr, "Addition Operator cannot be applied to two NULL string objects\n");
		return NIL_STRING;
	}
	if ( s == NULL ) return t; // don't REF/DEREF as we might free our return value
	if ( t == NULL ) return s;
	size_t n = strlen(s->str) + strlen(t->str);
	String *u = String_alloc(n);
	strcpy(u->str, s->str);
	strcat(u->str, t->str);
	return u;
}

bool String_eq(String *s, String *t) {
	assert(s);
	assert(t);
	return strcmp(s->str, t->str) == 0;
}

bool String_neq(String *s, String *t) {
	return !String_eq(s,t);
}

bool String_gt(String *s, String *t) {
	assert(s);
	assert(t);
	return strcmp(s->str, t->str) > 0;
}

bool String_ge(String *s, String *t) {
	assert(s);
	assert(t);
	return strcmp(s->str, t->str) >= 0;
}

bool String_lt(String *s, String *t) {
	assert(s);
	assert(t);
	return strcmp(s->str, t->str) < 0;

}
bool String_le(String *s, String *t) {
	assert(s);
	assert(t);
	return strcmp(s->str, t->str) <= 0;
}
