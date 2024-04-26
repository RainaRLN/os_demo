#ifndef _LINUX_TTY_H_
#define _LINUX_TTY_H_

#include "types.h"

void console_init(void);
void console_write(char *buf, u32 count);

#endif  // _LINUX_TTY_H_

