#include <stdio.h>

// A pointer is a variable that holds an address.


//Records in C are called 'struct'
struct foo{
    int i;
    float f;
    char *s; //string or pointer
};

int swap(int *x, int *y){ //x is pointer to int, y is pointer to int
    int tmp;

    // * is the dereference operator. It fetches the value addressed by the pointer
    printf("x = %d, y = %d\n", *x, *y);

    tmp = *x;
    *x = *y;
    *y = tmp;

    printf("x = %d, y = %d\n", *x, *y);

}
int swapWrong(int x, int y){ //x is pointer to int, y is pointer to int
    int tmp;

    // * is the dereference operator. It fetches the value addressed by the pointer
    printf("x = %d, y = %d\n", x, y);

    tmp = x;
    x = y;
    y = tmp;

    printf("x = %d, y = %d\n", x, y);

}

// We have three different operators in C:
// The dereference operator: *
// The arrow operator: -> (dereferences and then gets the field of a struct (only applies to structs))
// The array index operator: [] (applies an offset and then dereferences)
void populateFoo(struct foo *f, int i, float d, char *s){
    f->i = i;
    f->f = d;
    f->s = s;
}

//How to read variable declarations in C: Start with the variable name.
//                                        Go right when we can.
//                                        Go left when we must.
int main(int argc, char *argv[]){
    int i, j;

    struct foo f;

    i = 0;
    j = 1;

    printf("i = %d, j = %d\n", i, j);

    // & is the address operator. It takes the address of the variable
    // it is applied to. 
    swap(&i,&j);

    populateFoo(&f, 0, 3.14, "Hello World"); //Broken

    printf("i = %d, j = %d\n", i, j);
    //  f.i = 0;
    //  f.f = 3.14;
    //  f.s = "Hello World";
     printf("%d %f %s\n", f.i, f.f, f.s);
}