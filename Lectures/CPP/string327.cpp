#include "string327.h"
#include <cstring>
string327::string327(){
    str = strdup("");
}; //default constructor
string327::string327(const char *s){
    str = strdup(s);
}; // initializes from this pointer
string327::string327(const string327 &s){
    str = strdup(s.str);
}; //copier that takes a const reference to a string327 reference
string327::~string327(){
    free(str);
}; //destructor
int string327::length(){
    return strlen(str);
};
bool string327::operator>(const string327 &s) const{
    return strcmp(str, s.str) > 0;
};
bool string327::operator>=(const string327 &s) const{
    return strcmp(str, s.str) >= 0;
};
bool string327::operator<(const string327 &s) const{
    return strcmp(str, s.str) < 0;
};
bool string327::operator<=(const string327 &s) const{
    return strcmp(str, s.str) <= 0;
};
bool string327::operator==(const string327 &s) const{
    return strcmp(str, s.str) == 0;
};
bool string327::operator!=(const string327 &s) const{
    return strcmp(str, s.str) != 0;
};
string327 string327::operator+(const string327 &s) const{
    string327 r;

    free(r.str);

    r.str = (char *) malloc(strlen(str) + strlen(s.str) + 1);

    strcpy(strcpy(r.str, str), s.str);
    return r;
};
string327 string327::operator+(const char *s) const{
    string327 r;

    free(r.str);

    r.str = (char *) malloc(strlen(str) + strlen(s) + 1);

    strcpy(strcpy(r.str, str), s);
    return r;
};
string327 &string327::operator+=(const string327 &s) {
    
    int l;
    // When realloc fails, it return NULL. If the first parameter and 
    // the location for the return value are the same variable, this will
    // lose the pointer in the parameter. In general you should assign
    // return value to a temporary, check for success, and then (if desired)
    // overwrite the pointer.
    str = (char *)realloc(str, (l = strlen(str)) + strlen(s.str) + 1);
    
    strcpy(str + l, s.str);

    return *this;


};
string327 &string327::operator+=(const char *s){
 int l;
    // When realloc fails, it return NULL. If the first parameter and 
    // the location for the return value are the same variable, this will
    // lose the pointer in the parameter. In general you should assign
    // return value to a temporary, check for success, and then (if desired)
    // overwrite the pointer.
    str = (char *)realloc(str, (l = strlen(str)) + strlen(s) + 1);
    
    strcpy(str + l, s);

    return *this;
};
string327 &string327::operator=(const string327 &s){
    free(str);

    str = strdup(s.str);

    return *this;
};

string327 &string327::operator=(const char *s){
    free(str);

    str = (char *)s;

    return *this;
};

char &string327::operator[](int i) const{
    return str[i];
}
const char *string327::c_str() const{
    return str;
}
std::istream &operator>>(std::istream &i, const string327 &s){
    // Correctly implementing this is complicated-ish.
    // You'll do similar things in 1.07.
    // Here we're going to assume that our input fits into s.str.
    // NOT A SAFE ASSUMPTION!

    return i >> s.str;
};
std::ostream &operator<<(std::ostream &o, const string327 &s){
    return o << s.c_str;
};
