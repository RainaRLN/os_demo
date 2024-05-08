#include "enter_x64.h"
#include "linux/kernel.h"

// Vol.2A 3-198
// 如果可以改变 EFLAGS 的 bit21，表示支持 CPUID 指令
int check_cpuid(void)
{
    int ret = 0;

    __asm__ __volatile__(
        "pushfd \n\t"  // EFLAGS 入栈
        "pop eax \n\t"  // eax = EFLAGS
        "mov ebx, eax \n\t"  // ebx = eax
        "xor eax, 0x00200000 \n\t"  // 尝试改变 bit21
        "push eax \n\t"
        "popfd \n\t"  // EFLAGS = eax
        "pushfd \n\t"
        "pop eax \n\t"  // 重新读取 EFLAGS
        "xor eax, ebx \n\t"  // 判断 EFLAGS 是否改变
        "jz not_supported \n\t"  // 相同表示不支持
        "mov %0, 1 \n\t"  // ret = 1
        "jmp return \n\t"
        "not_supported: \n\t"
        "mov %0, 0 \n\t"  // ret = 0
        "return: \n\t"
        : "=c"(ret)
        :
        :
    );

    return ret;
}

// Vol.2A 3-215
int get_max_cpuid(void)
{
    int ret = 0;

    __asm__ __volatile__(
        "mov eax, 0x80000000 \n\t"
        "cpuid \n\t"
        : "=a"(ret)  // ret = eax
        :
        :
    );

    return ret;
}

// Vol.2A 3-215
// 0x80000001 function
// EDX bit29: Intel 64 Architecture available if 1
int check_longmode(void)
{
    int ret = 0;
    __asm__ __volatile__(
        "mov eax, 0x80000001 \n\t"
        "cpuid \n\t"
        : "=d"(ret)  // ret = edx
        :
        :
    );

    if (ret & (1 << 29)) {
        return 1;
    } else {
        return 0;
    }
    return ret;
}

// Vol.2A 3-216
// Get processor brand string
void get_brand_info(void)
{
    int info[13] = {0};

    __asm__ __volatile__(
        "mov eax, 0x80000002 \n\t"
        "cpuid \n\t"
        "mov [%[info] + 0 * 4], eax \n\t"
        "mov [%[info] + 1 * 4], ebx \n\t"
        "mov [%[info] + 2 * 4], ecx \n\t"
        "mov [%[info] + 3 * 4], edx \n\t"
        "mov eax, 0x80000003 \n\t"
        "cpuid \n\t"
        "mov [%[info] + 4 * 4], eax \n\t"
        "mov [%[info] + 5 * 4], ebx \n\t"
        "mov [%[info] + 6 * 4], ecx \n\t"
        "mov [%[info] + 7 * 4], edx \n\t"
        "mov eax, 0x80000004 \n\t"
        "cpuid \n\t"
        "mov [%[info] + 8 * 4], eax \n\t"
        "mov [%[info] + 9 * 4], ebx \n\t"
        "mov [%[info] + 10 * 4], ecx \n\t"
        "mov [%[info] + 11 * 4], edx \n\t"
        :
        : [info]"m"(info)
        :
    );
    printk("Brand Info: %s\n", info);
    return;
}

// Vol.2A 3-216
// Linear/Physical address size
void get_mem_info(void)
{
    int physical_address_bits = 0;
    int linear_address_bits = 0;

    __asm__ __volatile__(
        "mov eax, 0x80000008 \n\t"
        "cpuid \n\t"
        "movzx %0, al \n\t"
        "movzx %1, ah \n\t"
        : "=r"(physical_address_bits), "=r"(linear_address_bits)
        :
        : "eax"
    );
    printk("Physical address bits: %d\n", physical_address_bits);
    printk("Linear address bits: %d\n", linear_address_bits);
    return;
}

int x64_cpu_check(void)
{
    int ret = 0;

    if (!check_cpuid()) {
        printk("Your CPU doesn't support the CPUID instruction.\n");
        return 0;
    }
    printk("Support for CPUID instruction check OK.\n");

    ret = get_max_cpuid();
    printk("Maximum input value for extended function CPUID information is 0x%x\n", ret);
    if (ret < 0x80000008) {
        printk("We need 0x80000008 CPUID function.\n");
        return 0;
    }
    printk("Support for maximum CPUID function check OK.\n");

    if (!check_longmode()) {
        printk("Unsupport Intel 64 Architecture.\n");
        return 0;
    }
    printk("Support for Intel 64 Architecture check OK.\n");

    get_brand_info();

    get_mem_info();
    return 1;
}

