#pragma once
#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>

#define WINDOW_HEIGHT 800
#define WINDOW_WIDTH 1600

inline bool OnInit(SDL_Window** window){
    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        return false;
    }

    if((*window = SDL_CreateWindow("gravity2", 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI)) == NULL) {
        return false;
    }

    return true;
}

inline void OnCleanup(){
    SDL_Quit();
}

int clamp(int v, int min, int max){
    return (v < min) ? min : ((v > max) ? max : v);
}

void set_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
      Uint8 *target_pixel = (Uint8 *)surface->pixels + y * surface->pitch + x * 4;
      *(Uint32 *)target_pixel = pixel;
}

void draw_rect(SDL_Surface* surface, int x, int y, int w, int h, Uint32 color = 0xffffffffu){
    int x1 = clamp(x, 0, WINDOW_WIDTH-1);
    int y1 = clamp(y, 0, WINDOW_HEIGHT-1);
    int x2 = clamp(x+w, 0, WINDOW_WIDTH-1);
    int y2 = clamp(y+h, 0, WINDOW_HEIGHT-1);
    for(int i = x1; i <= x2; i++){
        set_pixel(surface, i, y1, color);
    }
    for(int i = x1; i <= x2; i++){
        set_pixel(surface, i, y2, color);
    }
    for(int i = y1; i <= y2; i++){
        set_pixel(surface, x1, i, color);
    }
    for(int i = y1; i <= y2; i++){
        set_pixel(surface, x2, i, color);
    }

}

void draw_rect_centered(SDL_Surface* surface, int x, int y, int w, int h, Uint32 color = 0xffffffffu){
    draw_rect(surface, x-w/2, y-h/2, w, h, color);
}

/// @brief 3x3 gaussian blur
void blur(float* values, int width, int height){
    float* new_values = (float*)calloc(width*height, sizeof(float));
    //std::cout << "before vertical blur" << std::endl;
    for (int x = 0; x < width; x++){
        for (int y = 1; y < height-1; y++){
            int index = x+y*width; 
            new_values[index] = 0;
            new_values[index] += values[index+width];
            new_values[index] += values[index]*2;
            new_values[index] += values[index-width];
            new_values[index] *= 0.25f;
        }
    }
    //std::cout << "after vertical blur" << std::endl;
    for (int x = 1; x < width-1; x++){
        for (int y = 0; y < height; y++){
            int index = x+y*width; 
            values[index] = 0;
            values[index] += new_values[index+1];
            values[index] += new_values[index]*2;
            values[index] += new_values[index-1];
            values[index] *= 0.25f;
        }
    }
    free(new_values);
}
/// @brief 5x5 gaussian blur
void blur5x5(float* values, int width, int height){
    const float multiplier = 1/16.0f;
    float* new_values = (float*)calloc(width*height, sizeof(float));
    //std::cout << "before vertical blur" << std::endl;
    for (int x = 0; x < width; x++){
        for (int y = 2; y < height-2; y++){
            int index = x+y*width; 
            new_values[index] = 0;
            new_values[index] += values[index+width*2];
            new_values[index] += values[index+width]*4;
            new_values[index] += values[index]*6;
            new_values[index] += values[index-width]*4;
            new_values[index] += values[index-width*2];
            new_values[index] *= multiplier;
        }
    }
    //std::cout << "after vertical blur" << std::endl;
    for (int x = 2; x < width-2; x++){
        for (int y = 0; y < height; y++){
            int index = x+y*width; 
            values[index] = 0;
             values[index] += new_values[index+2];
            values[index] += new_values[index+1]*4;
            values[index] += new_values[index]*6;
            values[index] += new_values[index-1]*4;
             values[index] += new_values[index-2];
            values[index] *= multiplier;
        }
    }
    free(new_values);
}

bool save_image(const char* pixels, int width, int height, int name_index) {
    // Create the subdirectory if it doesn't exist
    const std::string subdirectory = "./img/";
    mkdir(subdirectory.c_str(), 0777); // Using POSIX mkdir function

    // Construct the full file path
    char name[20];
    sprintf(name, "%06i", name_index);

    std::string filePath = subdirectory + std::string(name) + ".pgm";

    // Open the file in binary mode
    std::ofstream file(filePath, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Failed to open the file: " << filePath << std::endl;
        return false;
    }

    // Write the PGM file header
    // P5 is the magic number for PGM binary format
    // 255 is the maximum value for each pixel
    file << "P5\n" << width << " " << height << "\n255\n";

    // Write the pixel data
    file.write(pixels, width * height);

    // Close the file
    file.close();
    return true;
}


unsigned long hash(unsigned long a){
    a *= 0xabc197e889a92f18ul;
    a ^= 0x2f69a1b24d316bc2ul;
    a *= 0x1271765825487697ul;
    a ^= 0x7e16afb261762949ul;
    a *= 0xa827c1f2b3e78d47ul;
    a ^= 0x8b7a915712291ffaul;
    return a;
}
