#include <stdio.h>

size_t strlen327(const char *s){
    int i;
    for(i = 0; s[i]; i++);
    
    return i;
}

int strcmp_idiom(const char *s1 , const char *s2){
    while(*s1 && *s1++ == *s2++)
        ;
        
    return *s1-*s2;
}

int strcmp327(const char *s1 , const char *s2){
    int i;
    for(i = 0; s1[i] && s2[i] && s1[i] == s2[i]; i++);

    return s1[i]-s2[i];
}
char *strcpy327(char *dest, const char *src){

    int i;

    for(i = 0; src[i]; i++){
        dest[i]= src[i];
    }
    //for null byte
    dest[i] = src[i];
    return dest;
}

int main(int argc, char *argv[]){
    char *s = "Hello World!";
    char a[] = "Hello World!";

    printf("%lu\n", strlen327("Hello World!"));
    printf("%lu\n", strlen327(s));
    printf("%lu\n", strlen327(a));

    s = "Foo!";
    printf("%d\n", strlen327(s));

    a[0] = 'h';
    printf("%s\n", a);

    strcpy327(a, "Goodbye");

    printf("%s\n", a);

}
