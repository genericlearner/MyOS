#ifndef KERNEL_H
#define KERNEL_H

#define MYOS_MAX_PATH 108
#define VGA_WIDTH 80
#define VGA_HEIGHT 20
void kernel_main();
void print(const char* str);


#define ERROR(value) (void*) value
#define ERROR_I(value) (int)(value)
#define ISERR(value) ((int)value < 0)

#endif