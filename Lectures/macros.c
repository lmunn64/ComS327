#define FOO 42
// function type macros
#define min(x,y) (x < y ? x : y) // Always parens around function type macros

#define foo(x) { dddd;ffff; } //Or curly braces as fits context

#define min3(x, y, z) (min (x, min(y,z)))

//Stringification - A macro parameter prefixed with a hash will be turned into a string
#define stringify(x) #x

struct lutable_entry{
    char *name;
    int (*func)(int);
};

struct lutable_entry lookup_table[] ={
    {"foo", foo_lu_entry},
    {"baz", baz_lu_entry},
    {"bar", bar_lu_entry}
};

#define luentry(x) {#x, x_lu_entry}

struct lutable_entry lookup_table[] = {
    luentry(bar),
    luentry(baz),
    luentry(foo),
};

// Concatenation - Double hash concatenates left and right hand sides into a new symbol. One of these must be a macro parameter which is expanded before concatenation. The other can be any string of symbols.
#define concat_pre(x) foo_ ## x
#define concat_suf(x) x ## _bar

int main(int argc, char *argv[]){

    printf("%d", FOO);

    min3(3, 5, 1);

    stringify(foo);

    //This construction depends on the programmer being sure that the search key is valid
    bsearch(key, lookup_table, s, n, compare);

    concat_pre(bar);
    concat_suf(bar);

    return 0;
}