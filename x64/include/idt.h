#ifndef _IDT_H_
#define _IDT_H_

#include "linux/types.h"

// Vol.3A 2-12 Figure 2-6
typedef struct {
    ushort limit;
    int64 base;
} __attribute__((packed)) idtr_data_t;

// Vol.3A 6-11 Figure 6-2
// Vol.3A 6-20 Figure 6-8
typedef struct {
    short offset0;  // Offset to procedure entry point
    short selector;  // Segment selector for destination code segment
    char ist;  // Interrupt stack table Vol.3A 6-22
    char type : 4;  // Vol.3A 3-14 Table 3-2
    char d_type : 1;  // Descriptor type, 0 = sysetm
    char dpl : 2;  // Descriptor privilege level
    char present : 1;  // Segment present flag
    short offset1;
    int offset2;
    int reserved;
} __attribute__((packed)) idt_item_t;

void idt_init(void);

#endif  // _IDT_H_

