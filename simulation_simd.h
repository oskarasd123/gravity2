#pragma once
#include "vector.h"
#include "particle.h"
#include "asd.h"
#include <thread>
#include <immintrin.h>
#include <array>
#include <vector>




class Gravity_simulation_simd{
    public:
        std::vector<float> pos_x;
        std::vector<float> pos_y;
        std::vector<float> vel_x;
        std::vector<float> vel_y;
        std::vector<float> mass;
        int array_length;
        int nr_particles;
        VectorInt initial_mip_resolution;
        int mip_depth = 3;

        void init(int ammount)
        {
            nr_particles = ammount;
            array_length = ammount + 8;
            pos_x.reserve(array_length);
            pos_y.reserve(array_length);
            vel_x.reserve(array_length);
            vel_y.reserve(array_length);
            mass.reserve(array_length);
        }
        void random_particles(float range, float rotation)
        {
            const float inverse_max = 1 / (float)__UINT64_MAX__;
            pos_x.clear();
            pos_y.clear();
            vel_x.clear();
            vel_y.clear();
            mass.clear();
            for (int i = 0; i < nr_particles; i++){
                float x = (float)hash(i*2) * inverse_max * 2 - 1;
                float y = (float)hash(i*2+1) * inverse_max * 2 - 1;
                pos_x.push_back(x*range);
                pos_y.push_back(y*range);
                vel_x.push_back(y*rotation);
                vel_y.push_back(-x*rotation);
                mass.push_back(1);
            }
            for (int i = 0; i < 8; i++)
            {
                mass.push_back(0);
                pos_x.push_back(0);
                pos_y.push_back(0);
                vel_x.push_back(0);
                vel_y.push_back(0);
            }
        }

        void Move_particles(float dt){
            for (int i = 0; i < nr_particles; i++){
                pos_x[i] = pos_x[i] + vel_x[i] * dt;
                pos_y[i] = pos_y[i] + vel_y[i] * dt;
            }
        }

        void Attract_particles(float dt)
        {
            float min_dist = dt*8;
            float min_dist_inverse = sqrt(1/min_dist);

            __m256 min_dist_f = _mm256_set1_ps(min_dist_inverse);
            __m256 dt_f = _mm256_set1_ps(dt);
            int use_particles = 8*(nr_particles/8);
            
            for (int hop = 8; hop < use_particles; hop++)
            {
                for (int n = 0; n < use_particles/8+1; n++)
                {
                    int start_a = n*8;
                    int start_b = (start_a + hop)%(use_particles);

                    __m256 pos_x_a = _mm256_loadu_ps(&pos_x.data()[start_a]);
                    __m256 pos_y_a = _mm256_loadu_ps(&pos_y.data()[start_a]);

                    __m256 pos_x_b = _mm256_loadu_ps(&pos_x.data()[start_b]);
                    __m256 pos_y_b = _mm256_loadu_ps(&pos_y.data()[start_b]);

                    __m256 vel_x_a = _mm256_loadu_ps(&vel_x.data()[start_a]);
                    __m256 vel_y_a = _mm256_loadu_ps(&vel_y.data()[start_a]);

                    __m256 vel_x_b = _mm256_loadu_ps(&vel_x.data()[start_b]);
                    __m256 vel_y_b = _mm256_loadu_ps(&vel_y.data()[start_b]);

                    __m256 mass_b = _mm256_loadu_ps(&mass.data()[start_a]);

                    __m256 mass_a = _mm256_loadu_ps(&mass.data()[start_b]);


                    __m256 offset_x = _mm256_sub_ps(pos_x_b,pos_x_a);
                    __m256 offset_y = _mm256_sub_ps(pos_y_b,pos_y_a);
                    
                    __m256 dist_inverse = _mm256_rsqrt_ps(_mm256_add_ps(_mm256_mul_ps(offset_x, offset_x),_mm256_mul_ps(offset_y, offset_y)));
                    dist_inverse = _mm256_min_ps(dist_inverse, min_dist_f);

                    dist_inverse = _mm256_mul_ps(dist_inverse, _mm256_mul_ps(dist_inverse,dist_inverse));

                    offset_x = _mm256_mul_ps(_mm256_mul_ps(offset_x, dist_inverse), dt_f);
                    offset_y = _mm256_mul_ps(_mm256_mul_ps(offset_y, dist_inverse), dt_f);

                    vel_x_a = _mm256_add_ps(vel_x_a, _mm256_mul_ps(offset_x, mass_b));
                    vel_y_a = _mm256_add_ps(vel_y_a, _mm256_mul_ps(offset_y, mass_b));

                    vel_x_b = _mm256_sub_ps(vel_x_b, _mm256_mul_ps(offset_x, mass_a));
                    vel_y_b = _mm256_sub_ps(vel_y_b, _mm256_mul_ps(offset_y, mass_a));

                    _mm256_storeu_ps(&vel_x.data()[start_a], vel_x_a);
                    _mm256_storeu_ps(&vel_y.data()[start_a], vel_y_a);

                    _mm256_storeu_ps(&vel_x.data()[start_b], vel_x_b);
                    _mm256_storeu_ps(&vel_y.data()[start_b], vel_y_b);   
                }
            }

            for (int i = 0; i < 8; i++)
            {
                for (int n = 0; n < nr_particles; n++)
                {
                    int particle_a = n;
                    int particle_b = (n+i)%nr_particles;
                    float offset_x = pos_x[particle_a] - pos_x[particle_b];
                    float offset_y = pos_y[particle_a] - pos_y[particle_b];

                    float dist = sqrtf(offset_x*offset_x + offset_y*offset_y);

                    dist = (dist < min_dist) ? min_dist : dist;

                    float scaler = dt / (dist*dist*dist);

                    offset_x *= scaler;
                    offset_y *= scaler;

                    vel_x[particle_a] += offset_x * mass[particle_b];
                    vel_x[particle_a] += offset_y * mass[particle_b];

                    vel_x[particle_b] -= offset_x * mass[particle_a];
                    vel_x[particle_b] -= offset_y * mass[particle_a];
                }
            }
            
        }

        void expand_universe(float ammount){
            for (int i = 0; i < nr_particles; i++){
                pos_x[i] *= ammount;
                pos_y[i] *= ammount;
            }
        }

        float calculate_standard_distribution(){
            float x_avg = 0;
            float y_avg = 0;
            for (int i = 0; i < nr_particles; i++){
                x_avg += pos_x[i];
                y_avg += pos_y[i];
            }
            x_avg /= nr_particles;
            y_avg /= nr_particles;

            float x_distribution = 0;
            float y_distribution = 0;
            for (int i = 0; i < nr_particles; i++){
                x_distribution += (pos_x[i] - x_avg)*(pos_x[i] - x_avg);
                y_distribution += (pos_y[i] - y_avg)*(pos_y[i] - y_avg);
            }
            x_distribution /= nr_particles;
            y_distribution /= nr_particles;
            x_distribution = sqrt(x_distribution);
            y_distribution = sqrt(y_distribution);

            return sqrt(x_distribution*x_distribution + y_distribution*y_distribution);
        }



        void Update_particles(float dt)
        {
            expand_universe(0.7f/calculate_standard_distribution());
            Attract_particles(dt);
            Move_particles(dt);
        }
};