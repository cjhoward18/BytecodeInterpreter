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
#include <sys/stat.h>
#include "vm.h"
#include "loader.h"

static void inline vm_write32(byte *data, int n)   { *((int32_t *)data) = (int32_t)n; }
static void inline vm_write16(byte *data, int n) { *((int16_t *)data) = (int16_t)n; }

/*
Create a VM from a bytecode object/asm file, .bytecode; files look like:

2 strings
	0: 2/hi
	1: 3/bye
2 functions maxaddr=112
    0: 4/main
   73: 3/foo
40 instr, 112 bytes
	ICONST 1
	ICONST 0
	OR
    CALL 20,0   ; CALL addr32, nargs16
    ...
 */
VM *vm_load(FILE *f)
{
    VM *vm = vm_alloc();

    fscanf(f, "%d strings\n", &vm->num_strings);
	if ( vm->num_strings>0 ) {
		vm->strings = (String **)calloc((size_t) vm->num_strings, sizeof(String *));
		for (int i = 0; i < vm->num_strings; i++) {
			int index, name_size;
			fscanf(f, "%d: %d/", &index, &name_size);
			String *s = String_alloc((size_t) name_size);
			fgets(s->str, name_size + 1, f);
			vm->strings[index] = s;
		}
	}

    fscanf(f, "%d functions maxaddr=%d\n", &vm->num_functions, &vm->max_func_addr);
	if ( vm->num_functions>0 ) {
		vm->func_names = calloc((size_t) vm->max_func_addr + 1, sizeof(char *));
		for (int i = 1; i <= vm->num_functions; i++) {
			int name_size;
			addr32 addr;
			fscanf(f, "%d: %d/", &addr, &name_size);
			char name[name_size + 1];
			fgets(name, name_size + 1, f);
			// we want a map from byte addr to name of func at that addr
			vm->func_names[addr] = strdup(name);
		}
	}

    int ninstr, nbytes;
    fscanf(f, "%d instr, %d bytes\n", &ninstr, &nbytes);
    byte *code = calloc((size_t)nbytes, sizeof(byte));
    addr32 ip = 0;
    for (int i=1; i<=ninstr; i++) {
        char instr[80+1];
        fgets(instr, 80+1, f);
        char name[80];
        int opnd1, opnd2;
        int n = sscanf(instr, "%s %d, %d", name, &opnd1, &opnd2);
        VM_INSTRUCTION *I = vm_instr(name);
		if ( I==NULL ) {
			fprintf(stderr, "unknown bytecode %s; ignoring\n", name);
			continue;
		}
        code[ip] = I->opcode;
        ip++;
		int num_required_opnds = 0;
		if ( I->opnd_sizes[0]>0 ) num_required_opnds++;
		if ( I->opnd_sizes[1]>0 ) num_required_opnds++;
		if ( n-1 != num_required_opnds ) {
			char *output = print(vm->output, "operand mismatch; expecting %d, found %d\n", num_required_opnds, n-1);
			fprintf(stderr, "%s", output);
			continue;
		}
		// deal with operands
        if ( n>=2 ) { // got an instruction and 1 operand
            if ( I->opnd_sizes[0]==2 ) {
                vm_write16(&code[ip], opnd1);
            }
            else { // must be 4 bytes
                vm_write32(&code[ip], opnd1);
            }
            ip += I->opnd_sizes[0];
        }
        if ( n==3 ) { // got an instruction with 2 operands; fill in 2nd operand
			if ( I->opnd_sizes[1]==2 ) {
				vm_write16(&code[ip], opnd2);
			}
			else { // must be 4 bytes
				vm_write32(&code[ip], opnd2);
			}
			ip += I->opnd_sizes[1];
        }
    }
    vm_init(vm, code, nbytes);
    return vm;
}

VM_INSTRUCTION *vm_instr(char *name) {
    for (int i = 0; i < NUM_INSTRS; ++i) {
        if ( strcmp(name, vm_instructions[i].name)==0 ) {
            return &vm_instructions[i];
        }
    }
    return NULL;
}

addr32 vm_function(VM *vm, char *name) {
    for (addr32 a = 0; a <= vm->max_func_addr; ++a) {
		char *fname = vm->func_names[a];
        if ( fname!=NULL && strcmp(name, fname)==0 ) {
            return a;
        }
    }
    return 0xFFFFFFFF;
}