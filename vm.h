/*
The MIT License (MIT)

Copyright (c) 2015 Terence Parr, Hanzhou Shi, Shuai Yuan, Yuanyuan Zhang

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm_strings.h"

#ifndef VM_H_
#define VM_H_

static const int MAX_OUTPUT	= 1000000;	// max 100k output
static const int MAX_LOCALS		= 10;	// max locals/args in activation record
static const int MAX_CALL_STACK = 1000;
static const int MAX_OPND_STACK = 1000;

typedef unsigned char byte;
typedef uintptr_t word; // has to be big enough to hold a native machine pointer
typedef unsigned int addr32;

// Bytecodes are all 8 bits but operand size can vary
// Explicitly type the operations, even the loads/stores
// for both safety, efficiency, and possible JIT from bytecodes later.
typedef enum {
	HALT=0,

	IADD,
	ISUB,
	IMUL,
	IDIV,
	SADD,

    OR,
    AND,
    INEG,
	NOT,

	I2S,

	IEQ,
	INEQ,
	ILT,
	ILE,
	IGT,
	IGE,
	SEQ,
	SNEQ,
	SGT,
	SGE,
	SLT,
	SLE,

	BR,
	BRF,

	ICONST,
	SCONST,

	LOAD,
	STORE,
	SINDEX,

	POP,
	CALL,
	LOCALS,
	RET,

	PRINT,
	SLEN,
	SFREE,
} BYTECODE;

static const int NUM_INSTRS		= SFREE+1; // last opcode value + 1 is num instructions

typedef struct {
	char *name;
	BYTECODE opcode;
	int opnd_sizes[2]; // array of sizes in bytes; max 2 operands
	int num_stack_opnds;
} VM_INSTRUCTION;

typedef enum { INVALID=0, INT, BOOLEAN, STRING } element_type;

typedef struct {
	element_type type;
	union {
		int i;
		bool b;
		String *s;
	};
} element;

typedef struct activation_record {
	addr32 retaddr;
	char *name;						// set by CALL
	int nargs;						// set by CALL
	int nlocals;					// set by LOCALS
	element locals[MAX_LOCALS]; 	// args + locals go here per func def
} Activation_Record;

typedef struct {
	// registers
	addr32 ip;        	// instruction pointer register
    int sp;             // stack pointer register
	int callsp;			// call stack pointer register

	byte *code;   		// byte-addressable code memory.
	int code_size;
	element stack[MAX_OPND_STACK]; 	// operand stack, grows upwards; word addressable
	Activation_Record call_stack[MAX_CALL_STACK];

	int num_functions;
	int max_func_addr;
	char **func_names;
	int num_strings;
	String **strings;

	char *trace;
	char *output;		// prints strcat on to the end of this buffer
} VM;

extern VM *vm_alloc();
extern void vm_init(VM *vm, byte *code, int code_size);
extern void vm_free(VM *vm);
extern void vm_exec(VM *vm, bool trace_to_stderr);
extern VM_INSTRUCTION vm_instructions[];
extern char *print(char *buffer, char *fmt, ...);

#endif
