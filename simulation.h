#pragma once
#include "vector.h"
#include "particle.h"
#include "asd.h"
#include <thread>
#include <immintrin.h>

#define NR_THREADS 2
class m256_f{
    public:
        __m256 reg;
        
        m256_f(__m256 a){
            reg = a;
        }
        m256_f();
        m256_f(float value){
            reg =_mm256_set1_ps(value);
        }
        m256_f operator+(const m256_f& other)const{
            return m256_f(_mm256_add_ps(reg,other.reg));
        }
        m256_f operator-(const m256_f& other)const{
            return m256_f(_mm256_sub_ps(reg,other.reg));
        }
        m256_f operator-()const{
            return m256_f(_mm256_sub_ps(m256_f(0).reg,reg));
        }
        m256_f operator*(const m256_f& other)const{
            return m256_f(_mm256_mul_ps(reg, other.reg));
        }
        m256_f operator/(const m256_f& other)const{
            return m256_f(_mm256_div_ps(reg, other.reg));
        }
        m256_f inv_sqr_dist(m256_f y){
            return m256_f(_mm256_rsqrt_ps((*this * *this+y*y).reg));
        }
        m256_f max(m256_f other)const{
            return m256_f(_mm256_max_ps(reg, other.reg));
        }
        
};



class Gravity_simulation{
    public:
        Particle* particles;
        int array_length;
        int nr_particles;
        VectorInt initial_mip_resolution;
        int mip_depth = 3;

        void init(int ammount)
        {
            nr_particles = 0;
            this->array_length = ammount;
            particles = (Particle*)calloc(ammount, sizeof(Particle));
        }
        void random_particles(float range, float rotation)
        {
            const float inverse_max = 1 / (float)__UINT64_MAX__;
            for (int i = 0; i < array_length; i++){
                float x = (float)hash(i*2) * inverse_max * 2 - 1;
                float y = (float)hash(i*2+1) * inverse_max * 2 - 1;
                particles[i] = Particle(Vector(x*range,y*range), Vector(y*rotation,-x*rotation), 1);
            }
            nr_particles = array_length;
        }

        void Move_particles(float dt){
            for (int i = 0; i < nr_particles; i++){
                particles[i].position = particles[i].position + particles[i].velocity * dt;
            }
        }

        static void Attract_particles(Particle* particles, int nr_particles, float dt, int core)
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

        static void Attract_particles_simd(Particle* particles, int nr_particles, float dt)
        {
            float min_dist = dt * 8;
            float inv_max_dist = 1/min_dist;
            m256_f inv_max_dist_f = -m256_f(inv_max_dist);
            m256_f dt_f = m256_f(dt);
            
            for(int hop = 8; hop < nr_particles; hop++){
                for (int i = 0; i < (nr_particles+7)/8; i++)
                {
                    int a = i/(hop);
                    int b = i%(hop);
                    int start = a*(hop)+b;
                    Particle particleA[8];
                    Particle particleB[8];
                    for (int b = 0; b < 8 && start+b<nr_particles; b++){
                        particleA[b] = particles[(start+b)%nr_particles];
                        particleB[b] = particles[(start+b+hop)%nr_particles];
                    }
                    
                    
                    
                    
                    m256_f a_pos_x(_mm256_set_ps(particleA[0].position.x,particleA[1].position.x,particleA[2].position.x,particleA[3].position.x,particleA[4].position.x,particleA[5].position.x,particleA[6].position.x,particleA[7].position.x));
                    m256_f b_pos_x(_mm256_set_ps(particleB[0].position.x,particleB[1].position.x,particleB[2].position.x,particleB[3].position.x,particleB[4].position.x,particleB[5].position.x,particleB[6].position.x,particleB[7].position.x));
                    

                    m256_f a_pos_y(_mm256_set_ps(particleA[0].position.y,particleA[1].position.y,particleA[2].position.y,particleA[3].position.y,particleA[4].position.y,particleA[5].position.y,particleA[6].position.y,particleA[7].position.y));
                    m256_f b_pos_y(_mm256_set_ps(particleB[0].position.y,particleB[1].position.y,particleB[2].position.y,particleB[3].position.y,particleB[4].position.y,particleB[5].position.y,particleB[6].position.y,particleB[7].position.y));

                    m256_f a_vel_x(_mm256_set_ps(particleA[0].velocity.x,particleA[1].velocity.x,particleA[2].velocity.x,particleA[3].velocity.x,particleA[4].velocity.x,particleA[5].velocity.x,particleA[6].velocity.x,particleA[7].velocity.x));
                    m256_f b_vel_x(_mm256_set_ps(particleB[0].velocity.x,particleB[1].velocity.x,particleB[2].velocity.x,particleB[3].velocity.x,particleB[4].velocity.x,particleB[5].velocity.x,particleB[6].velocity.x,particleB[7].velocity.x));

                    m256_f a_vel_y(_mm256_set_ps(particleA[0].velocity.y,particleA[1].velocity.y,particleA[2].velocity.y,particleA[3].velocity.y,particleA[4].velocity.y,particleA[5].velocity.y,particleA[6].velocity.y,particleA[7].velocity.y));
                    m256_f b_vel_y(_mm256_set_ps(particleB[0].velocity.y,particleB[1].velocity.y,particleB[2].velocity.y,particleB[3].velocity.y,particleB[4].velocity.y,particleB[5].velocity.y,particleB[6].velocity.y,particleB[7].velocity.y));

                    m256_f a_mass(_mm256_set_ps(particleA[0].mass,particleA[1].mass,particleA[2].mass,particleA[3].mass,particleA[4].mass,particleA[5].mass,particleA[6].mass,particleA[7].mass));
                    m256_f b_mass(_mm256_set_ps(particleB[0].mass,particleB[1].mass,particleB[2].mass,particleB[3].mass,particleB[4].mass,particleB[5].mass,particleB[6].mass,particleB[7].mass));

                    m256_f offset_x = b_pos_x - a_pos_x;
                    m256_f offset_y = b_pos_y - a_pos_y;

                    m256_f inv_dist = offset_x.inv_sqr_dist(offset_y);
                    inv_dist = -((-inv_dist).max(inv_max_dist_f));
                    m256_f multiplier = m256_f(1)/(inv_dist*inv_dist*inv_dist);

                    offset_x = offset_x*multiplier*dt_f;
                    offset_y = offset_y*multiplier*dt_f;
                    
                    a_vel_x = a_vel_x + offset_x*b_mass;
                    a_vel_y = a_vel_y + offset_y*b_mass;
                    
                    b_vel_x = b_vel_x + offset_x*a_mass;
                    b_vel_y = b_vel_y + offset_y*a_mass;

                    float a_vel_x_[8];
                    float a_vel_y_[8];
                    float b_vel_x_[8];
                    float b_vel_y_[8];
                    _mm256_storeu_ps(a_vel_x_, a_vel_x.reg);
                    _mm256_storeu_ps(a_vel_y_, a_vel_y.reg);
                    _mm256_storeu_ps(b_vel_x_, b_vel_x.reg);
                    _mm256_storeu_ps(b_vel_y_, b_vel_y.reg);

                    for (int b = 0; b < 8; b++){
                        particleA[i].velocity.x = a_vel_x_[i];
                        particleA[i].velocity.y = a_vel_y_[i];
                        particleB[i].velocity.x = b_vel_x_[i];
                        particleB[i].velocity.y = b_vel_y_[i];
                    }

                    for (int b = 0; b < 8 && start+b<nr_particles; b++){
                        particles[(start+b)%nr_particles] = particleA[7-b];
                        particles[(start+b+hop)%nr_particles] = particleB[7-b];
                    }

                }
            }
        }




        void Update_particles(float dt)
        {
            std::thread* threads = (std::thread*)calloc(NR_THREADS, sizeof(std::thread));
            for (int i = 0; i < NR_THREADS; i++){
                threads[i] = std::thread(Attract_particles, particles, nr_particles, dt, i);
            }
            for (int i = 0; i < NR_THREADS; i++){
                threads[i].join();
            }
            //Attract_particles_simd(particles, nr_particles, dt);
            
            //Attract_particles(particles, nr_particles, dt, 0);
            //Attract_particles(particles, nr_particles, dt, 1);
            Move_particles(dt);
        }
};