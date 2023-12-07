#pragma once
#include "vector.h"
#include "asd.h"
#include <thread>
#include <immintrin.h>
#include <array>
#include <vector>

#define NR_THREADS 2




class Gravity_simulation_batched{
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
            nr_particles = 0;
            array_length = ammount;
            pos_x.reserve(ammount);
            pos_y.reserve(ammount);
            vel_x.reserve(ammount);
            vel_y.reserve(ammount);
            mass.reserve(ammount);
        }
        void random_particles(float range, float rotation)
        {
            pos_x.clear();
            pos_y.clear();
            vel_x.clear();
            vel_y.clear();
            mass.clear();
            const float inverse_max = 1 / (float)__UINT64_MAX__;
            for (int i = 0; i < array_length; i++){
                float x = (float)hash(i*2) * inverse_max * 2 - 1;
                float y = (float)hash(i*2+1) * inverse_max * 2 - 1;
                pos_x.emplace_back(x*range);
                pos_y.emplace_back(y*range);
                vel_x.emplace_back(y*rotation);
                vel_y.emplace_back(-x*rotation);
                mass.emplace_back(1);
            }
            nr_particles = array_length;
        }

        void Move_particles(float dt){
            for (int i = 0; i < nr_particles; i++){
                pos_x[i] = pos_x[i] + vel_x[i] * dt;
                pos_y[i] = pos_y[i] + vel_y[i] * dt;
            }
        }

        void Attract_particles_2_batches(int batch1start, int batch2start, int batchsize float dt)
        {
            float min_dist = dt*8;
            float min_dist_inverse = sqrtf(1/min_dist);
            
            float* particleA_pos_x = &pos_x[batch1start];
            float* particleA_pos_y = &pos_y[batch1start];

            float* particleB_pos_x = &pos_x[batch2start];
            float* particleB_pos_y = &pos_y[batch2start];

            float* particleA_vel_x = &vel_x[batch1start];
            float* particleA_vel_y = &vel_y[batch1start];

            float* particleB_vel_x = &vel_x[batch2start];
            float* particleB_vel_y = &vel_y[batch2start];

            float* particleA_mass = &mass[batch1start];
            float* particleB_mass = &mass[batch1start];

            for (int i = 0; i < batchsize; i++)
            {
                for (int n = 0; n < batchsize; n++)
                {
                    float offset_x = particleA_pos_x[n] - particleB_pos_x[i];
                    float offset_y = particleA_pos_y[n] - particleB_pos_y[i];

                    float dist = sqrtf(offset_x*offset_x + offset_y*offset_y);

                    dist = (dist < min_dist) ? min_dist : dist;

                    float scaler = dt / (dist*dist*dist);

                    offset_x *= scaler;
                    offset_y *= scaler;

                    particleA_vel_x[i] += offset_x * particleB_mass[n];
                    particleA_vel_y[i] += offset_y * particleB_mass[n];

                    particleB_vel_x[n] -= offset_x * particleA_mass[i];
                    particleB_vel_y[n] -= offset_y * particleA_mass[i];
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
            x_distribution = sqrtf(x_distribution);
            y_distribution = sqrtf(y_distribution);

            return sqrtf(x_distribution*x_distribution + y_distribution*y_distribution);
        }



        void Update_particles(float dt)
        {
            //expand_universe(0.7f/calculate_standard_distribution());

            Attract_particles_2_batches(0, 100, 100, dt);
            Move_particles(dt);
        }
};