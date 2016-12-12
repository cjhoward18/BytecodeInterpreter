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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "vm.h"
#include "loader.h"

VM_INSTRUCTION vm_instructions[] = {
        {"HALT",   HALT,   {},     0},

        {"IADD",   IADD,   {},     2},
        {"ISUB",   ISUB,   {},     2},
        {"IMUL",   IMUL,   {},     2},
        {"IDIV",   IDIV,   {},     2},
        {"SADD",   SADD,   {},     2},

        {"OR",     OR,     {},     2},
        {"AND",    AND,    {},     2},
        {"INEG",   INEG,   {},     1},
        {"NOT",    NOT,    {},     1},

        {"I2S",    I2S,    {},     1},

        {"IEQ",    IEQ,    {},     2},
        {"INEQ",   INEQ,   {},     2},
        {"ILT",    ILT,    {},     2},
        {"ILE",    ILE,    {},     2},
        {"IGT",    IGT,    {},     2},
        {"IGE",    IGE,    {},     2},
        {"SEQ",    SEQ,    {},     2},
        {"SNEQ",   SNEQ,   {},     2},
        {"SGT",    SGT,    {},     2},
        {"SGE",    SGE,    {},     2},
        {"SLT",    SLT,    {},     2},
        {"SLE",    SLE,    {},     2},

        {"BR",     BR,     {4},    0}, // branch to absolute address
        {"BRF",    BRF,    {4},    0},

        {"ICONST", ICONST, {4},    0},
        {"SCONST", SCONST, {2},    0},

        {"LOAD",   LOAD,   {2},    0},
        {"STORE",  STORE,  {2},    1},
        {"SINDEX", SINDEX, {},     2},

        {"POP",    POP,    {},     0},
        {"CALL",   CALL,   {4, 2}, 0}, // num stack operands is actually a variable and 2nd operand
        {"LOCALS", LOCALS, {2},    0},
        {"RET",    RET,    {},     1},

        {"PRINT",  PRINT,  {},     1},
        {"SLEN",   SLEN,   {},     1},
        {"SFREE",  SFREE,  {2},    0} // free a str in a local
};

static void vm_print_instr(VM *vm, addr32 ip);

static void vm_print_stack(VM *vm);

static char *vm_print_element(char *buffer, element el);

static inline int32_t int32(const byte *data, addr32 ip);

static inline int16_t int16(const byte *data, addr32 ip);

static void vm_trace_print_element(VM *vm, element el);

VM *vm_alloc() {

    VM *vm = calloc(sizeof(VM), 1);
    vm->output = calloc(sizeof(char[MAX_OUTPUT]), 1);
    vm->trace = calloc(sizeof(char[MAX_OUTPUT]), 1);
    return vm;

}

void vm_init(VM *vm, byte *code, int code_size) {
    vm->code = code;
    vm->code_size = code_size;
    vm->sp = -1; // grow upwards, stack[sp] is top of stack and valid
    vm->callsp = -1;
}

void vm_free(VM *vm) {
    free(vm->output);
    free(vm->trace);
    free(vm->code);

    int i;

    for (i = 0; i <= vm->max_func_addr; i++) {
        free(vm->func_names[i]);
    }

    for (i = 0; i < vm->num_strings; i++) {
        free(vm->strings[i]);
    }

    free(vm->func_names);
    free(vm->strings);
    free(vm);
}

// You don't need to use/create these functions but I used them
// to help me when there are bugs in my code.
static void inline validate_stack_address(VM *vm, int a) { }

static void inline validate_stack(VM *vm, byte opcode, int sp) { }

void vm_exec(VM *vm, bool trace_to_stderr) {
    bool trace = true; // always store trace in vm->trace
    int x, y, g, opcode;
    size_t size;
    short n;
    bool t, f;
    String *o;
    String *k, *p;
    char z;
    char *q, *w;
    Activation_Record *m;
    vm->call_stack[++vm->callsp].name = "main";
    vm->ip = vm_function(vm, "main");
    opcode = vm->code[vm->ip];
    while (opcode != HALT && vm->ip < vm->code_size) {

        if (trace) {
            vm_print_instr(vm, vm->ip);
            if (trace_to_stderr) fprintf(stderr, "%s", vm->trace);
        }
        vm->ip++;
        switch (opcode) {
            case IADD:
                x = vm->stack[vm->sp--].i;
                y = vm->stack[vm->sp--].i;
                vm->stack[++vm->sp].i = y + x;
                break;
            case ISUB:
                x = vm->stack[vm->sp--].i;
                y = vm->stack[vm->sp--].i;
                vm->stack[++vm->sp].i = y - x;
                break;
            case IDIV:
                x = vm->stack[vm->sp--].i;
                y = vm->stack[vm->sp--].i;
                vm->stack[++vm->sp].i = y / x;
                break;
            case IMUL:
                x = vm->stack[vm->sp--].i;
                y = vm->stack[vm->sp--].i;
                vm->stack[++vm->sp].i = y * x;
                break;
            case ICONST:
                x = int32(vm->code, vm->ip);
                vm->ip += 4;
                vm->stack[++vm->sp].i = x;
                vm->stack[vm->sp].type = INT;
                break;
            case PRINT:
                vm_print_element(vm->output, vm->stack[vm->sp--]);
                print(vm->output, "\n");
                break;
            case SADD:
                k = vm->stack[vm->sp--].s;
                p = vm->stack[vm->sp--].s;
                o = String_add(p, k);
                vm->stack[++vm->sp].s = o;
                break;
            case LOCALS:
                vm->call_stack[vm->callsp].nlocals = int16(vm->code, vm->ip);
                vm->ip += 2;;
                break;
            case SCONST:
                n = int16(vm->code, vm->ip);
                vm->ip += 2;
                vm->stack[++vm->sp].s = String_new(vm->strings[n]->str);
                vm->stack[vm->sp].type = STRING;
                break;
            case STORE:
                n = int16(vm->code, vm->ip);
                vm->ip += 2;
                vm->call_stack[vm->callsp].locals[n] = vm->stack[vm->sp--];
                break;
            case LOAD:
                n = int16(vm->code, vm->ip);
                vm->ip += 2;
                vm->stack[++vm->sp] = vm->call_stack[vm->callsp].locals[n];
                break;
            case SFREE:
                n = int16(vm->code, vm->ip);
                vm->ip += 2;
                free(vm->call_stack[vm->callsp].locals[n].s);
                vm->call_stack[vm->callsp].locals[n].type = NULL;
                vm->call_stack[vm->callsp].locals[n].s = NULL;
                break;
            case SLEN:
                size = strlen(vm->stack[vm->sp--].s->str);
                vm->stack[++vm->sp].i = (int)size;
                vm->stack[vm->sp].type = INT;
                break;
            case IEQ:
                y = vm->stack[vm->sp--].i;
                x = vm->stack[vm->sp--].i;
                vm->stack[++vm->sp].b = (x == y);
                vm->stack[vm->sp].type = BOOLEAN;
                break;
            case INEQ:
                y = vm->stack[vm->sp--].i;
                x = vm->stack[vm->sp--].i;
                vm->stack[++vm->sp].b = (x != y);
                vm->stack[vm->sp].type = BOOLEAN;
                break;
            case ILT:
                y = vm->stack[vm->sp--].i;
                x = vm->stack[vm->sp--].i;
                vm->stack[++vm->sp].b = (x < y) ? true : false;
                vm->stack[vm->sp].type = BOOLEAN;
                break;
            case IGT:
                y = vm->stack[vm->sp--].i;
                x = vm->stack[vm->sp--].i;
                vm->stack[++vm->sp].b = (x > y) ? true : false;
                vm->stack[vm->sp].type = BOOLEAN;
                break;
            case IGE:
                y = vm->stack[vm->sp--].i;
                x = vm->stack[vm->sp--].i;
                vm->stack[++vm->sp].b = (x >= y) ? true : false;
                vm->stack[vm->sp].type = BOOLEAN;
                break;
            case ILE:
                y = vm->stack[vm->sp--].i;
                x = vm->stack[vm->sp--].i;
                vm->stack[++vm->sp].b = (x <= y) ? true : false;
                vm->stack[vm->sp].type = BOOLEAN;
                break;
            case I2S:
                x = vm->stack[vm->sp--].i;
                vm->stack[++vm->sp].s = String_from_int(x);
                vm->stack[vm->sp].type = STRING;
                break;
            case SEQ:
                q = vm->stack[vm->sp--].s->str;
                w = vm->stack[vm->sp--].s->str;
                vm->stack[++vm->sp].b = (strcmp(w, q) == 0) ? true : false;
                vm->stack[vm->sp].type = BOOLEAN;
                break;
            case SNEQ:
                q = vm->stack[vm->sp--].s->str;
                w = vm->stack[vm->sp--].s->str;
                vm->stack[++vm->sp].b = (strcmp(w, q) != 0) ? true : false;
                vm->stack[vm->sp].type = BOOLEAN;
                break;
            case SGT:
                q = vm->stack[vm->sp--].s->str;
                w = vm->stack[vm->sp--].s->str;
                vm->stack[++vm->sp].b = (strcmp(w, q) > 0) ? true : false;
                vm->stack[vm->sp].type = BOOLEAN;
                break;
            case SGE:
                q = vm->stack[vm->sp--].s->str;
                w = vm->stack[vm->sp--].s->str;
                vm->stack[++vm->sp].b = (strcmp(w, q) >= 0) ? true : false;
                vm->stack[vm->sp].type = BOOLEAN;
                break;
            case SLT:
                q = vm->stack[vm->sp--].s->str;
                w = vm->stack[vm->sp--].s->str;
                vm->stack[++vm->sp].b = (strcmp(w, q) < 0) ? true : false;
                vm->stack[vm->sp].type = BOOLEAN;
                break;
            case SLE:
                q = vm->stack[vm->sp--].s->str;
                w = vm->stack[vm->sp--].s->str;
                vm->stack[++vm->sp].b = (strcmp(w, q) <= 0) ? true : false;
                vm->stack[vm->sp].type = BOOLEAN;
                break;
            case SINDEX:
                x = vm->stack[vm->sp--].i;
                w = vm->stack[vm->sp--].s->str;
                z = w[x - 1];
                vm->stack[++vm->sp].s = String_from_char(z);
                vm->stack[vm->sp].type = STRING;
                break;
            case BR:
                x = int32(vm->code, vm->ip);
                vm->ip = x;
                break;
            case BRF:
                x = int32(vm->code, vm->ip);
                if (vm->stack[vm->sp--].b == false) {
                    vm->ip = (addr32)x;
                } else {
                    vm->ip += 4;
                }
                break;
            case POP:
                vm->sp--;
                break;
            case CALL:
                x = int32(vm->code, vm->ip);
                vm->ip += 4;
                y = int16(vm->code, vm->ip);
                vm->ip += 2;
                m = &vm->call_stack[++vm->callsp];
                m->nargs = y;
                m->retaddr = vm->ip;
                for (g = y - 1; g > -1; g--) {
                    m->locals[g] = vm->stack[vm->sp--];
                }
                m->name = vm->func_names[x];
                vm->ip = (addr32)x;
                break;
            case OR:
                t = vm->stack[vm->sp--].b;
                f = vm->stack[vm->sp--].b;
                vm->stack[++vm->sp].b = (t | f);
                vm->stack[vm->sp].type = BOOLEAN;
                break;
            case AND:
                t = vm->stack[vm->sp--].b;
                f = vm->stack[vm->sp--].b;
                vm->stack[++vm->sp].b = (t & f);
                vm->stack[vm->sp].type = BOOLEAN;
                break;
            case INEG:
                x = vm->stack[vm->sp--].i;
                vm->stack[++vm->sp].i = -x;
                vm->stack[vm->sp].type = INT;
                break;
            case NOT:
                t = vm->stack[vm->sp--].b;
                vm->stack[++vm->sp].b = !x;
                vm->stack[vm->sp].type = BOOLEAN;
                break;
            case RET:
                m = &vm->call_stack[vm->callsp--];
                vm->ip = m->retaddr;
                break;
            default:
                printf("invalid opcode: %d at ip=%d\n", opcode, (vm->ip - 1));
                exit(1);
        }
        if (trace) {
            vm_print_stack(vm);
            if (trace_to_stderr) fprintf(stderr, "%s", vm->trace);
        }
        opcode = vm->code[vm->ip];
    }
    if (trace) {
        vm_print_instr(vm, vm->ip);
        vm_print_stack(vm);
        if (trace_to_stderr) fprintf(stderr, "%s", vm->trace);
    }

}

/* return a 32-bit integer at data[ip] */
static inline int32_t int32(const byte *data, addr32 ip) {
    return *((int32_t *) &data[ip]);
}

/* return a 16-bit integer at data[ip] */
static inline int16_t int16(const byte *data, addr32 ip) {
    return *((int16_t *) &data[ip]); // could be negative value
}

void vm_print_instr_opnd0(const VM *vm, addr32 ip) {
    int op_code = vm->code[ip];
    VM_INSTRUCTION *inst = &vm_instructions[op_code];
    print(vm->trace, "%04d:  %-25s", ip, inst->name);
}

void vm_print_instr_opnd1(const VM *vm, addr32 ip) {
    int op_code = vm->code[ip];
    VM_INSTRUCTION *inst = &vm_instructions[op_code];
    int sz = inst->opnd_sizes[0];
    switch (sz) {
        case 2:
            print(vm->trace, "%04d:  %-15s%-10d", ip, inst->name, int16(vm->code, ip + 1));
            break;
        case 4:
            print(vm->trace, "%04d:  %-15s%-10d", ip, inst->name, int32(vm->code, ip + 1));
            break;
        default:
            break;
    }
}

/* currently only a CALL instr */
void vm_print_instr_opnd2(const VM *vm, addr32 ip) {
    int op_code = vm->code[ip];
    VM_INSTRUCTION *inst = &vm_instructions[op_code];
    char buf[100];
    sprintf(buf, "%d, %d", int32(vm->code, ip + 1), int16(vm->code, ip + 5));
    print(vm->trace, "%04d:  %-15s%-10s", ip, inst->name, buf);
}

static void vm_print_instr(VM *vm, addr32 ip) {
    int op_code = vm->code[ip];
    VM_INSTRUCTION *inst = &vm_instructions[op_code];
    if (inst->opnd_sizes[1] > 0) {
        vm_print_instr_opnd2(vm, ip);
    }
    else if (inst->opnd_sizes[0] > 0) {
        vm_print_instr_opnd1(vm, ip);
    }
    else {
        vm_print_instr_opnd0(vm, ip);
    }
}

static void vm_print_stack(VM *vm) {
    // stack grows upwards; stack[sp] is top of stack
    print(vm->trace, "calls=[");
    for (int i = 0; i <= vm->callsp; i++) {
        Activation_Record *frame = &vm->call_stack[i];
        print(vm->trace, " %s=[", frame->name);
        for (int j = 0; j < frame->nlocals + frame->nargs; ++j) {
            print(vm->trace, " ");
            vm_trace_print_element(vm, frame->locals[j]);
        }
        print(vm->trace, " ]");
    }
    print(vm->trace, " ]  ");
    print(vm->trace, "stack=[");
    for (int i = 0; i <= vm->sp; i++) {
        print(vm->trace, " ");
        vm_trace_print_element(vm, vm->stack[i]);
    }
    print(vm->trace, " ] sp=%d\n", vm->sp);
}

void vm_trace_print_element(VM *vm, element el) {
    if (el.type == STRING) {
        print(vm->trace, "\"");
        vm_print_element(vm->trace, el);
        print(vm->trace, "\"");
    }
    else {
        vm_print_element(vm->trace, el);
    }
}

char *vm_print_element(char *buffer, element el) {
    switch (el.type) {
        case INT :
            return print(buffer, "%d", el.i);
        case BOOLEAN :
            return print(buffer, "%s", el.b ? "true" : "false");
        case STRING :
            return print(buffer, "%s", el.s->str);
        default:
            return print(buffer, "%s", "?");
    }
}

char *print(char *buffer, char *fmt, ...) {
    va_list args;
    char buf[1000];

    size_t n = strlen(buffer);
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    strcat(buffer, buf);
    va_end(args);
    return &buffer[n];
}
