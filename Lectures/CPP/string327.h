#ifndef STRING327_H
#define STRING327_H

#include <iostream>
// if(s > t) //this will work with operator method // if (s.operator(t))
//     ..
// cout << s+t;
// cout << s << t; //operator<<(operator<<(cout, s), t)
class string327{ //default inside a class you are private
    private:
        char *str;
    public:
        string327(); //default constructor
        string327(const char *); // initializes from this pointer
        string327(const string327 &); //copier that takes a const reference to a string327 reference
        ~string327(); //destructor
        int length();
        bool operator>(const string327 &) const;
        bool operator>=(const string327 &) const;
        bool operator<(const string327 &) const;
        bool operator<=(const string327 &) const;
        bool operator==(const string327 &) const;
        bool operator!=(const string327 &) const;
        string327 operator+(const string327 &) const;
        string327 operator+(const char *) const;
        string327 &operator+=(const string327 &);
        string327 &operator+=(const char *);
        string327 &operator=(const string327 &);
        string327 &operator=(const char *);
        const char *c_str() const;
        char &string327::operator[](int i) const;
        //friends have access to our privates
        friend std::istream &operator>>(std::istream &, const string327 &);
};
std::ostream &operator<<(std::ostream &, const string327 &);

#endif