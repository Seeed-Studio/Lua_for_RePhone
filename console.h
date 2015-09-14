
#ifndef __CONSOLE_H__
#define __CONSOLE_H__

void console_init(void);
void console_putc(char ch);
void console_puts(const char *str);
int  console_getc(void);


#endif // __CONSOLE_H__
