#pragma once
#include "vector.h"
#include "particle.h"
#include "asd.h"
#include <thread>
#include <immintrin.h>
#include <array>
#include <vector>

#define NR_THREADS 1




class Gravity_simulation{
    public:
        std::vector<Particle> particles;
        int array_length;
        int nr_particles;
        VectorInt initial_mip_resolution;
        int mip_depth = 3;

        void init(int ammount)
        {
            nr_particles = 0;
            this->array_length = ammount;
            particles.reserve(ammount);
        }
        void random_particles(float range, float rotation)
        {
            const float inverse_max = 1 / (float)__UINT64_MAX__;
            for (int i = 0; i < array_length; i++){
                float x = (float)hash(i*2) * inverse_max * 2 - 1;
                float y = (float)hash(i*2+1) * inverse_max * 2 - 1;
                particles.emplace_back(Vector(x*range,y*range), Vector(y*rotation,-x*rotation), 1);
            }
            nr_particles = array_length;
        }

        void Move_particles(float dt){
            for (int i = 0; i < nr_particles; i++){
                particles[i].position = particles[i].position + particles[i].velocity * dt;
            }
        }

        static void Attract_particles(std::vector<Particle>& particles, int nr_particles, float dt, int core)
        {
            float min_dist = dt * 8;
            
            for (int i = core; i < nr_particles; i+=NR_THREADS){
                for (int n = i+1; n < nr_particles; n++){
                    Particle* particleA = &particles[i];
                    Particle* particleB = &particles[n];
                    Vector offset = (particleB->position) - particleA->position;
                    float distance = offset.magnitude();
                    distance = std::max(min_dist,distance);
                    offset = offset / (distance*distance*distance);
                    offset = offset * dt;
                    particleA->velocity = particleA->velocity + offset*particleB->mass*dt;
                    particleB->velocity = particleB->velocity - offset*particleA->mass*dt;
                }
            }
        }




        void Update_particles(float dt)
        {
            std::array<std::thread, NR_THREADS> threads;
            for (int i = 0; i < NR_THREADS; i++){
                threads[i] = std::thread(Attract_particles, std::ref(particles), nr_particles, dt, i);
            }
            for (int i = 0; i < NR_THREADS; i++){
                threads[i].join();
            }
            Move_particles(dt);
        }
};