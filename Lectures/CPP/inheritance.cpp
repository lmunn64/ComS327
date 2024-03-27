#include <iostream>

using namespace std;

class shape{
public:
    virtual ~shape() {}
    virtual double perimeter() = 0; // = 0 TELLS COMPILER THAT IT IS ABSTRACT
    virtual double area() = 0;
    virtual ostream &print(ostream &o) = 0; //polymorphic print method
};

ostream &operator<<(ostream &o, const shape &s){
    return s.print(o);
}

class rectangel : public shape{
    private: 
        double w,h;
    public:
        rectangle(double width, double height){
            w = width;
            h = height;
        }

        double perimeter() const{
            return w*2 + h*2;
        }

        double area() const {
            return w*h;
        }

        ostream &print(ostream &o) const{
            return o << "rectangle["  << w << "," << h << "]";
        }
};

int main(){
    shape *s;
    
    rectangle r(7,4);

    cout << r << endl;

    s = &r;

    cout << *s << endl;

    return 0;
}