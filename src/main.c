#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint8_t *input_stack;
static uint16_t *instruct_stack;
static int input_stack_max = 0;
static int instruct_stack_max = 0;
static int input_cur = 0;
static int instruct_cur = 0;
static uint64_t regs[16] = {
    /*  0    1    2    3    4    5    6    7    */
    0x0, 0x0f, 0x04, 0x00, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,
    /*  8    9    10   11   12   13   14   15   */
};

void inp_stack_init(char *path) {
    FILE *asm_input = fopen(path, "r");
    if (!asm_input) {
        printf("fnf\n");
        exit(1);
    }

    input_stack = malloc(sizeof(uint8_t) * 1024);

    while (fscanf(asm_input, " %x", &input_stack[input_stack_max]) == 1)
        input_stack_max++;
    fclose(asm_input);
    input_stack = realloc(input_stack, sizeof(uint8_t) * (input_stack_max + 1));
}

void instruct_stack_init(char *path) {
    FILE *asm_instr = fopen(path, "r");
    if (!asm_instr) {
        printf("fnf2\n");
        exit(2);
    }

    instruct_stack = malloc(sizeof(uint16_t) * 1024);

    while (fscanf(asm_instr, " %x", &instruct_stack[instruct_stack_max]) == 1)
        instruct_stack_max++;
    fclose(asm_instr);
    instruct_stack =
        realloc(instruct_stack, sizeof(uint16_t) * (instruct_stack_max + 1));
}

void to_binary16(uint16_t v, uint8_t *out) {
    for (int bit = 15; bit >= 0; bit--) {
        out[15 - bit] = (v & (1U << bit)) ? '1' : '0';
    }
}

void print_bin(const uint16_t val) {
    uint8_t test[16];
    to_binary16(val, test);

    for (int i = 0; i < 16; i++) {
        if (i % 4 == 0 && i != 0)
            printf(" ");
        printf("%c", test[i]);
    }
    puts("");
}
void uint8_print_array_hex(const uint8_t *arr, int len) {
    printf("\n");
    for (int i = 0; i < len; i++) {
        printf("0x%02X -> ", arr[i]);
        print_bin(arr[i]);
    }
}
void uint16_print_array_hex(const uint16_t *arr, int len) {
    printf("\n");
    for (uint16_t i = 0; i < len; i++) {
        printf("0x%x -> ", arr[i]);
        print_bin(arr[i]);
    }
}

void print_mem() {
    puts("");
    for (int i = 0; i < 16; i++) {
        printf("0x%02x ", regs[i]);
        if ((i + 1) % 4 == 0)
            puts("");
    }
    puts("");
}

/*
 * 0b00 = R-Type
 * 0b01 = B-Type
 * 0b10 = I-type
 * 0b11 = L-Type
 */

static void inline add(uint8_t x3, uint8_t x2, uint8_t x1) {
    if (x3)
        regs[x3] = regs[x2] + regs[x1];
}

static void inline and (uint8_t x3, uint8_t x2, uint8_t x1) {
    if (x3)
        regs[x3] = regs[x2] & regs[x1];
}

static void inline or (uint8_t x3, uint8_t x2, uint8_t x1) {
    if (x3)
        regs[x3] = regs[x2] | regs[x1];
}

static void inline sll(uint8_t x3, uint8_t x2, uint8_t x1) {
    if (x3)
        regs[x3] = regs[x2] << regs[x1];
}

static inline uint8_t hex_nth(uint16_t value, int n) {
    return (value >> (n * 4)) & 0xF;
}

static inline void ld(uint64_t *x1, uint8_t *x2) {
}
static inline void lb(uint8_t *x1, uint8_t *x2) {
}
static inline void sd(uint64_t x1, uint64_t x2) {
}
static inline void sb() {
}
void read(uint16_t cur_instr) {
    uint8_t x3 = hex_nth(cur_instr, 3);
    uint8_t x2 = hex_nth(cur_instr, 2);
    uint8_t x1 = hex_nth(cur_instr, 1);

    printf("0x%x\n", cur_instr);
    print_bin(cur_instr);
    puts("\nHex Breakdown:");
    printf("Regs: 0x%x 0x%x 0x%x\n", x3, x2, x1);
    printf("Vals: 0x%x 0x%x 0x%x\n\n", regs[x3], regs[x2], regs[x1]);
    uint8_t instr_bin[16];
    to_binary16(cur_instr, instr_bin);

    uint8_t hi = ((instr_bin[15] - '0') << 1) | (instr_bin[14] - '0');
    uint8_t lo = ((instr_bin[13] - '0') << 1) | (instr_bin[12] - '0');

    printf("up: 0b%02d\n", hi);
    printf("lo: 0b%02d\n", lo);

    switch (hi) {
        case 0b00:
            switch (lo) {
                case 0b00:
                    puts("adding");
                    add(x3, x2, x1);
                    break;
                case 0b01:
                    and(x3, x2, x1);
                    break;
                case 0b10:
                    or (x3, x2, x1);
                    break;
                case 0b11:
                    sll(x3, x2, x1);
                    break;
            }
            break;

        case 0b01:
            // beq
            break;

        case 0b10:
            switch (lo) {
                case 0b00:
                    // ld
                    break;
                case 0b01:
                    // lb
                    break;
                case 0b10:
                    // sd
                    break;
                case 0b11:
                    // sb
                    break;
            }
            break;

        case 0b11:
            // li
            break;
    }
}

int main(int count, char **argv) {
    inp_stack_init(argv[1]);
    instruct_stack_init(argv[2]);

    uint8_print_array_hex(input_stack, input_stack_max);
    uint16_print_array_hex(instruct_stack, instruct_stack_max);

    print_mem();
    uint16_t ins = 0x3120;
    read(ins);
    print_mem();

    free(input_stack);
    free(instruct_stack);
}
