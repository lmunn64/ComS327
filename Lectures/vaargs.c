#include <stdio.h>
#include <stdarg.h>
#include <math.h>

// A simplified printf that prints only char, int, float and string
void printf327(const char *format, ...){
    va_list ap;
    int i;
    char c;
    char *s;
    float f;
    va_start(ap, format);

    while(*format){
        switch(*format){
            case 'c':
                c = va_arg(ap, int);
                printf("%c ", c);
                break;
            case 'd':
                i = va_arg(ap, int);
                printf("%d ", i);
                break;
            case 'f':
                f = va_arg(ap, double);
                printf("%f  ", f);
                break;
            case 's':
                s = va_arg(ap, char *);
                printf("'%s' ", s);
                break;
            default:
                fprintf(stderr, "Invalid conversion specifier in format string: %c\n", *format);
        }
        format++;
    }
    printf("\n");
}
int main(int argc, char *argv[]){
    
    printf327("sdsddcf", "Hello", 0, "foo", 7, 23, 'f', 3.14);
    return 0;
}