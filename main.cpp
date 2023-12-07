#include <SDL2/SDL.h>
#include "asd.h"
#include <iostream>
#include "vector.h"
#include "simulation.h"
#include <math.h>
//#include "simulation_grid.h"
#include <vector>
#include "simulation_simd.h"
//#include "simulation_batched.h"

#define USE_SIMD 1


int main()
{
    int ammount = 100;
    std::cin >> ammount;
    #if USE_SIMD
        Gravity_simulation_simd simulation;
    #else
        Gravity_simulation simulation;
    #endif
    simulation.initial_mip_resolution = VectorInt(20,20);
    simulation.mip_depth = 2;

    simulation.init(ammount);
    float rotation_ammount = 0.1f*pow(ammount,0.6666f);
    simulation.random_particles(1,rotation_ammount);

    Vector view_position(0,0);
    float view_scale = WINDOW_HEIGHT/2;
    SDL_Window* window;
    SDL_Surface* window_surface;
    OnInit(&window);
    window_surface = SDL_GetWindowSurface(window);
    std::vector<float> window_values(WINDOW_HEIGHT*WINDOW_WIDTH);
    std::fill(window_values.begin(), window_values.end(),0.0f);
    std::vector<char> window_chars(WINDOW_HEIGHT*WINDOW_WIDTH);
    SDL_Event Event;
    bool running=true;
    int nr_updates = 0;
    float dt = 0.00001f;
    float brightness = 10000.0f/ammount;
    bool simulate = true;
    bool record = false;
    int img_nr = 0;
    VectorInt mouse_pos(0,0);
    while(running)
    {
        //std::cout << "loop" << std::endl;
        SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
        while(SDL_PollEvent(&Event))
        {
            //std::cout << "event" << std::endl;
            if(Event.type == SDL_QUIT) running = false;
            else if (Event.type == SDL_MOUSEWHEEL)
            {
                if (Event.wheel.y < 0){
                    VectorInt changeA = (mouse_pos-VectorInt(WINDOW_WIDTH/2,WINDOW_HEIGHT/2));
                    view_position = view_position + Vector((float)(changeA.x), (float)(changeA.y))/view_scale;
                    view_scale *= 0.95f;
                    VectorInt changeB = (mouse_pos-VectorInt(WINDOW_WIDTH/2,WINDOW_HEIGHT/2));
                    view_position = view_position - Vector((float)(changeB.x), (float)(changeB.y))/view_scale;
                }
                if (Event.wheel.y > 0){
                    VectorInt changeA = (mouse_pos-VectorInt(WINDOW_WIDTH/2,WINDOW_HEIGHT/2));
                    view_position = view_position + Vector((float)(changeA.x), (float)(changeA.y))/view_scale;
                    view_scale *= 1.05f;
                    VectorInt changeB = (mouse_pos-VectorInt(WINDOW_WIDTH/2,WINDOW_HEIGHT/2));
                    view_position = view_position - Vector((float)(changeB.x), (float)(changeB.y))/view_scale;
                }
            }
            else if (Event.type == SDL_KEYDOWN)
            {
                switch (Event.key.keysym.sym)
                {
                    default:
                        break;
                    case SDLK_ESCAPE:
                        running = false;
                        std::cout << "exit" << std::endl;
                        break;
                    case SDLK_r:
                        simulation.random_particles(1,rotation_ammount);
                        break;
                    case SDLK_SPACE:
                        simulate = !simulate;
                        break;
                    case SDLK_s:
                        simulate = true;
                        record = true;
                        break;
                    case SDLK_LEFT:
                        view_position.x -= WINDOW_HEIGHT / view_scale * 0.01f;
                        //std::cout << "left" << std::endl;
                        break;
                    case SDLK_RIGHT:
                        view_position.x += WINDOW_HEIGHT / view_scale * 0.01f;
                        //std::cout << "right" << std::endl;
                        break;
                    case SDLK_UP:
                        view_position.y -= WINDOW_HEIGHT / view_scale * 0.01f;
                        //std::cout << "up" << std::endl;
                        break;
                    case SDLK_DOWN:
                        view_position.y += WINDOW_HEIGHT / view_scale * 0.01f;
                        //std::cout << "down" << std::endl;
                        break;
                    case SDLK_o:
                        view_scale *= 0.98;
                        //std::cout << "out" << std::endl;
                        break;
                    case SDLK_i:
                        view_scale *= 1.02;
                        //std::cout << "in" << std::endl;
                        break;
                    case SDLK_f:
                        nr_updates++;
                        break;
                    case SDLK_g:
                        nr_updates--;
                        break;
                    case SDLK_v:
                        dt *= 1.1f;
                        break;
                    case SDLK_b:
                        dt *= 1/1.1f;
                        break;
                    case SDLK_e:
                        brightness *= 1.05f;
                        break;
                    case SDLK_w:
                        brightness *= 0.95f;
                        break;
                }
                //std::cout << view_position.x << " " << view_position.y << " " << view_scale << std::endl;
            }
        }
        //std::cout << "after event" << std::endl;
        uint64_t start_time = SDL_GetTicks64();

        for (int i = 0; i < nr_updates && simulate; i++){
            //std::cout << "before update" << std::endl;
            simulation.Update_particles(dt);
        }
        std::cout << SDL_GetTicks64() - start_time << " ms/ " << nr_updates << " updates  dt: " << dt;
        start_time = SDL_GetTicks64();
        SDL_FillRect(window_surface, 0, 0);
        for (int i = 0; i < simulation.nr_particles; i++){
            int x, y;
            #if USE_SIMD
            x = (int)((simulation.pos_x[i] - view_position.x)*view_scale);
            y = (int)((simulation.pos_y[i] - view_position.y)*view_scale);
            #else
            x = (int)((simulation.particles[i].position.x - view_position.x)*view_scale);
            y = (int)((simulation.particles[i].position.y - view_position.y)*view_scale);
            #endif
            x = clamp(x+WINDOW_WIDTH/2, 0, WINDOW_WIDTH-1);
            y = clamp(y+WINDOW_HEIGHT/2, 0, WINDOW_HEIGHT-1);
            //std::cout << x << " " << y << std::endl;
            //set_pixel(window_surface, x, y, 0xffffffffu);
            window_values[x+y*WINDOW_WIDTH] += 1;
        }
        blur5x5(window_values.data(), WINDOW_WIDTH, WINDOW_HEIGHT);
        float view_brightness = view_scale*brightness;
        
        for (int i = 0; i < WINDOW_HEIGHT*WINDOW_WIDTH-1; i++){
            float value = window_values[i]*1.0f * view_brightness;
            window_values[i] = 0;
            value = (value < 0xff) ? value : 0xff;
            set_pixel(window_surface, i, 0, 0x01010101u * (int)value);
            window_chars[i] = (char)value;
        }
        //draw_rect_centered(window_surface, mouse_pos.x,mouse_pos.y, (int)view_scale, (int)view_scale);
        SDL_UpdateWindowSurface(window);
        std::cout << " " << SDL_GetTicks64() - start_time << " ms/render" << std::endl;
        if (record){
            bool success = save_image(window_chars.data(), WINDOW_WIDTH, WINDOW_HEIGHT, img_nr);
            if (!success){std::cout << "save_image failed";}
            img_nr++;
        }
        SDL_Delay(30);
    }
    if (SDL_WasInit(SDL_INIT_VIDEO))
    {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }
    SDL_Quit();
    return 0;
}

