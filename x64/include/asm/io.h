#ifndef _ASM_IO_H_
#define _ASM_IO_H_

char in_byte(int port);
void out_byte(int port, int val);

short in_word(int port);
void out_word(int port, int v);

#endif  // _ASM_IO_H_

