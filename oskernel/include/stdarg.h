#ifndef _STDARG_H_
#define _STDARG_H_

typedef char * va_list;

// 参数 push 进堆栈占 32bit 大小
// p = (char *)&v + sizeof(指针大小);
// p 指向下一个参数首地址
#define va_start(p, v) (p = (va_list)&v + sizeof(char *))
// p += sizeof(char *);  // p 指向下一个参数
// return *(t *)(p - sizeof(char *));  // 返回当前参数的值
#define va_arg(p, t) (*(t*)((p += sizeof(char *)) - sizeof(char *)))
// p = NULL;
#define va_end(p) (p = 0)

// FIXME: 实测第一个参数类型需要是 32bit 大小
// 不然 GCC 汇编后会重新在堆栈中申请一块内存存放转换后的值
// 导致拿到的首个参数的地址不是push时候的地址

#endif  // _STDARG_H_

