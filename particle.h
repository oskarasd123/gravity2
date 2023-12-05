#pragma once
#include "vector.h"


struct Particle{
    Vector position, velocity;
    float mass;
    Particle() : position(), velocity(), mass(1) {}
    Particle(Vector pos, Vector vel, float mass) : position(pos), velocity(vel), mass(mass) {}

    Particle operator+(const Particle& other){
        Particle new_ = Particle();
        float mass_sum_inverse = 1/(mass + other.mass);
        new_.mass = mass + other.mass;
        new_.position = (position * mass + other.position * other.mass)*mass_sum_inverse;
        new_.velocity = (velocity * mass + other.velocity * other.mass)*mass_sum_inverse;
        return new_;
    }
};