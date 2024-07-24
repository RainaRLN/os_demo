#include "enter_x64.h"
#include "linux/kernel.h"
#include "linux/gdt.h"
#include "asm/system.h"
#include "string.h"

// Vol.3A 4-23
// 所有表的地址都是 4K 对齐的
#define PML4T_ADDR 0x8000
#define PDPT_ADDR 0x9000
#define PD_ADDR 0xA000
// #define PT_ADDR 0xB000

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

static void load_pml4(void)
{
    // Vol.3A 4-2: CR4.PAE=1, IA32_EFER.LME=1, CR4.LA57=0, 4-level paging is uesd.
    __asm__ __volatile__(
        "mov eax, cr4 \n\t"
        "btr eax, 12 \n\t"
        "mov cr4, eax \n\t"
    );

    // Page map level 4 table
    int *plm4e_ptr = (int *)PML4T_ADDR;

    // Vol.3A 4-6: Every paging structure is 4096 Bytes in size
    memset(plm4e_ptr, 0, 4096);

    // Vol.3A 4-25 Table 4-15
    *plm4e_ptr = PDPT_ADDR | 3;  // R/W | Present
    *(plm4e_ptr + 1) = 0;

    // Page directory pointer table
    int *pdpte_ptr = (int *)PDPT_ADDR;
    memset(pdpte_ptr, 0, 4096);

    // Vol.3A 4-27 Table 4-17
    *pdpte_ptr = PD_ADDR | 3;  // R/W | Present
    *(pdpte_ptr + 1) = 0;

    // Page directory
    int *pde_ptr = (int *)PD_ADDR;
    memset(pde_ptr, 0, 4096);

    // 采用 2M 分页，直接映射 0 - 100M
    for (int i = 0; i < 50; ++i) {
        // Vol.3A 4-27 Table 4-18
        *(pde_ptr + i * 2) = 0x200000 * i | 0x83;  // Page size | R/W | Present
        *(pde_ptr + i * 2 + 1) = 0;
    }

    // Vol.3A 4-20 Table 4-12
    __asm__ __volatile__(
            "mov cr3, ebx \n\t"
            :
            :"b"(PML4T_ADDR)
            :
    );
}

// Vol.3A 9-11 9.8.5
static void init_ia32e_mode(void)
{
    // 1. 禁用分页模式: CR0.PG = 0
    //    Vol.3A 2-14 Figure 2-7. Control Registers
    __asm__ __volatile__(
        "mov eax, cr0 \n\t"
        "btr eax, 31 \n\t"
        "mov cr0, eax \n\t"
    );

    // 2. 开启物理地址扩展功能: CR4.PAE = 1
    __asm__ __volatile__(
        "mov eax, cr4 \n\t"
        "bts eax, 5 \n\t"
        "mov cr4, eax \n\t"
    );

    // 3. 加载四级页表地址到 CR3
    //    此时在保护模式，只有 CR3 的低 32 位被写入
    //    所以 64 位模式的页表必须位于内存的低 4G 内
    //    在激活 IA-32e 模式后，可将页表重新定位到物理内存的任意位置
    load_pml4();

    // 4. 使能 IA-32e 模式: IA32_EFER.LME = 1
    //    Vol.4 2-50: IA32_EFER 的 MSR 地址为 0xC0000080
    //    Vol.2B 4-534: RDMSR Description
    __asm__ __volatile__(
        "mov ecx, 0xc0000080 \n\t"
        "rdmsr \n\t"
        "bts eax, 8 \n\t"
        "wrmsr \n\t"
    );

    // 5. 开启分页: CR0.PG = 1
    __asm__ __volatile__(
        "mov eax, cr0 \n\t"
        "bts eax, 31 \n\t"
        "mov cr0, eax \n\t"
    );
}

void enter_x64(void)
{
    BOCHS_DEBUG();
    init_ia32e_mode();
    install_x64_descriptor();
    BOCHS_DEBUG();
}

