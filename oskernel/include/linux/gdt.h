#ifndef _LINUX_GDT_H_
#define _LINUX_GDT_H_

#pragma pack(2)  // limit 是 16bit，按 16bit 对齐
typedef struct {
    short limit;
    int base;
} gdtr_data_t;
#pragma pack()

// Vol.3A 3-10 Figure 3-8
typedef struct {
    unsigned short limit_low;  // Segment Limit 15:00
    unsigned int base_low : 24;  // Base Address 23:00
    unsigned char seg_type : 4;  // Segment Type
    unsigned char d_type: 1;  // Descriptor type (0 = system; 1 = code or data)
    unsigned char dpl : 2;  // Descriptor privilege level
    unsigned char present : 1;  // Segment present
    unsigned char limit_high : 4;  // Segment Limit 19:16
    unsigned char available : 1;  // Available for use by system software
    unsigned char long_mode : 1;  // 64-bit code segment
    unsigned char db : 1;  // Default operation size (0 = 16-bit; 1 = 32-bit)
    unsigned char granularity : 1;  // Granularity
    unsigned char base_high;  // Base Address 31:24
} __attribute__((packed)) gdt_item_t;

void install_x64_descriptor(void);

#endif  // _LINUX_GDT_H_

