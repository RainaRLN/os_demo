#ifndef _PIC_H_
#define _PIC_H_

void pic_init(void);
void send_eoi(int idt_index);

#endif  // _PIC_H_

