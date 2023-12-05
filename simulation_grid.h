#pragma once
#include "vector.h"
#include "particle.h"
#include "asd.h"
#include <iostream>
#include <SDL2/SDL.h>


class Gravity_grid_simulation{
    public:
        Particle* particles;
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

        SDL_Surface* draw_surface;

        bool init(int ammount)
        {
            nr_particles = 0;
            this->array_length = ammount;
            particles = (Particle*)calloc(ammount, sizeof(Particle));
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
            std::cout << "attract_particles" << std::endl;
            float min_dist = dt * 8;
            int grid_ammounts[nr_cells];
            for (int i = 0; i < nr_cells; i++) {grid_ammounts[i] = 0;}
            for (int i = 0; i < mip_depth; i++){
                for (int n = 0; n < mip_resolution[i].x*mip_resolution[i].y; n++){
                    mips[i][n] = Particle(Vector(),Vector(),0);
                }
            }
            for (int i = 0; i < nr_particles; i++)
            {
                int cell_index = particle_cell_index(particles[i].position);
                mips[0][cell_index].position = mips[0][cell_index].position + particles[i].position*particles[i].mass;
                mips[0][cell_index].mass = mips[0][cell_index].mass + particles[i].mass;
                grid_ammounts[cell_index]++;
            }
            for (int i = 0; i < nr_cells; i++){
                mips[0][i].position = mips[0][i].position / mips[0][i].mass; 
            }
            // mips[0] particles are now averages of the particles in their cells. 
            // there is no need to know their velocities
            std::cout << "before coarse mipmaps" << std::endl;
            // create the coarser mips
            for (int i = 0; i < mip_depth-1; i++){
                for (int x = 0; x < mip_resolution[i].x; x++){
                    for (int y = 0; y < mip_resolution[i].y; y++){
                        int in_index = x/2+(y/2)*mip_resolution[i+1].x;
                        int from_index = x + y*mip_resolution[i].x;
                        mips[i+1][in_index].position = mips[i+1][in_index].position + mips[i][from_index].position*mips[i][from_index].mass;
                        mips[i+1][in_index].mass = mips[i+1][in_index].mass + mips[i][from_index].mass;
                    }
                }
            }
            std::cout << "after mipmaps\n";
            int** cell_particle_indexes = (int**)calloc(nr_cells, sizeof(int*));
            for (int i = 0; i < nr_cells; i++){
                cell_particle_indexes[i] = (int*)calloc(grid_ammounts[i], sizeof(int));
            }
            int cell_filled_ammount[nr_cells];
            for (int i = 0; i < nr_cells; i++) {cell_filled_ammount[i]=0;}


            for (int i = 0; i < nr_particles; i++)
            {
                int cell_index = particle_cell_index(particles[i].position);
                std::cout << cell_filled_ammount[cell_index] << "/" << grid_ammounts[cell_index] << " " << cell_index << std::endl;
                cell_particle_indexes[cell_index][cell_filled_ammount[cell_index]] = i;
                cell_filled_ammount[cell_index]++;
            }
            std::cout << "after cell population" << std::endl;

            // actual gravity force application
            // there is an error before here
            for (int i = 0; i < nr_particles; i++){
                int initial_cell_index = particle_cell_index(particles[i].position);
                if (initial_cell_index >= nr_cells || initial_cell_index < 0){
                    continue;
                }
                for (int x = -2; x <= 2; x++){
                    for (int y = -2; y <= 2; y++){
                        int other_cell_index = initial_cell_index + x + y*initial_mip_resolution.x;
                        for (int b = 0; b < grid_ammounts[other_cell_index]; b++){
                            int other_particle_index = cell_particle_indexes[other_cell_index][b];
                            Particle* particleA = &particles[i];
                            Particle* particleB = &particles[other_particle_index];
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
                
            }
            std::cout << "after attraction" << std::endl;

            for (int i = 0; i < nr_cells; i++){
                free(cell_particle_indexes[i]);
            }
            free(cell_particle_indexes);
            return 0;
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