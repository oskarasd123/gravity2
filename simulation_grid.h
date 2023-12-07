#pragma once
#include "vector.h"
#include "particle.h"
#include "asd.h"
#include <iostream>
#include <vector>


class Gravity_grid_simulation{
    public:
        std::vector<Particle> particles;
        int array_length;
        int nr_particles;
        Vector bounds = {0,0};
        VectorInt initial_mip_resolution;
        int nr_cells;
        VectorInt* mip_resolution;
        float grid_cell_size;
        float grid_cell_size_inverse;
        Vector grid_top_left_position;
        int mip_depth = 3;
        Particle** mips;

        VectorInt draw_resolution;
        float* draw_surface;

        bool init(int ammount)
        {
            nr_particles = ammount;
            particles.reserve(ammount);
            
            if (mip_depth < 1) return false;

            mips = (Particle**)calloc(mip_depth, sizeof(Vector*));

            mip_resolution = (VectorInt*)calloc(mip_depth, sizeof(Vector));

            mip_resolution[0] = initial_mip_resolution;
            mips[0] = (Particle*)calloc(initial_mip_resolution.x*initial_mip_resolution.y, sizeof(Vector));

            for (int i = 1; i < mip_depth; i++){
                mip_resolution[i] = VectorInt(mip_resolution[i-1].x/2+1, mip_resolution[i-1].y/2+1);
                mips[i] = (Particle*)calloc(mip_resolution[i].x*mip_resolution[i].y, sizeof(Vector));
            }
            return true;
        }

        
        void random_particles(float range, float rotation)
        {
            const float inverse_max = 1 / (float)__UINT64_MAX__;
            for (int i = 0; i < array_length; i++){
                float x = (float)hash(i*2) * inverse_max * 2 - 1;
                float y = (float)hash(i*2+1) * inverse_max * 2 - 1;
                particles[i] = Particle(Vector(x*range,y*range)+bounds*0.5f, Vector(y*rotation,-x*rotation), 1);
            }
            nr_particles = array_length;
        }

        void Move_particles(float dt){
            for (int i = 0; i < nr_particles; i++){
                particles[i].position = particles[i].position + particles[i].velocity * dt;
            }
        }
        int particle_cell_index(Vector pos){
            return ((int)((pos.x-grid_top_left_position.x)*grid_cell_size_inverse)+(int)((pos.y-grid_top_left_position.y)*grid_cell_size_inverse)*initial_mip_resolution.x)%nr_cells;
        }

        int Attract_particles(float dt)
        {
            
        }
        
        void Find_grid(){
            Vector min_position;
            Vector max_position;
            for (int i = 0; i < nr_particles; i++){
                min_position.x = (min_position.x < particles[i].position.x) ? min_position.x : particles[i].position.x;
                min_position.y = (min_position.y < particles[i].position.y) ? min_position.y : particles[i].position.y;
                max_position.x = (max_position.x > particles[i].position.x) ? max_position.x : particles[i].position.x;
                max_position.y = (max_position.y > particles[i].position.y) ? max_position.y : particles[i].position.y;
            }
            Vector offset = max_position-min_position;
            float size = (offset.x > offset.y) ? offset.x : offset.y;
            // the bounds should be a sqare
            grid_cell_size = size/(initial_mip_resolution.x-5);
            grid_top_left_position = min_position - Vector(grid_cell_size,grid_cell_size)*2;
            grid_cell_size_inverse = 1/ grid_cell_size;

        }

        void Constrain_particles(){  
            for (int i = 0; i < nr_particles; i++){
                particles[i].position = (particles[i].position+bounds*0.5f)%bounds-bounds*0.5f;
            }
        }


        void Update_particles(float dt)
        {
            Find_grid();
            nr_cells = initial_mip_resolution.x*initial_mip_resolution.y;
            Attract_particles(dt);
            Move_particles(dt);
            std::cout << "update complete" << std::endl;
            //Constrain_particles();
            
        }
};