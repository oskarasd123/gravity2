#pragma once
#include <math.h>

struct Vector{
    float x, y;
    Vector() : x(0), y(0){}
    Vector(float x, float y) : x(x), y(y){}
    Vector right(){
        return Vector(y,-x);
    }
    Vector operator+(const Vector& other)const{
        return Vector(x+other.x, y+other.y);
    }
    Vector operator-(const Vector& other)const{
        return Vector(x-other.x, y-other.y);
    }
    Vector operator*(const float& other)const{
        return Vector(x*other, y*other);
    }
    float operator*(const Vector& other)const{// dot product
        return (x*other.x + y*other.y);
    }
    Vector operator/(const float& other)const{
        return Vector(x/other, y/other);
    }

    Vector operator%(const Vector& other)const{
        return Vector(fmod(x,other.x), fmod(y,other.y));
    }
    
    float magnitude(){
        return sqrt(x*x+y*y);
    }

};


struct VectorInt{
    int x, y;
    VectorInt() : x(0), y(0){}
    VectorInt(int x, int y) : x(x), y(y){}
    VectorInt right(){
        return VectorInt(y,-x);
    }
    VectorInt operator+(const VectorInt& other)const{
        return VectorInt(x+other.x, y+other.y);
    }
    VectorInt operator-(const VectorInt& other)const{
        return VectorInt(x-other.x, y-other.y);
    }
    VectorInt operator*(const int& other)const{
        return VectorInt(x*other, y*other);
    }
    int operator*(const VectorInt& other)const{// dot product
        return (x*other.x + y*other.y);
    }
    VectorInt operator/(const int& other)const{
        return VectorInt(x/other, y/other);
    }

    VectorInt operator%(const VectorInt& other)const{
        return VectorInt((x%other.x), (y%other.y));
    }
    
    float magnitude(){
        return sqrt(x*x+y*y);
    }
};

