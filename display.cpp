#include "display.hpp"
#include <SDL2/SDL.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 320;

const int DISPLAY_HEIGHT = 32;
const int DISPLAY_WIDTH = 64;

Display::Display() {
        if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) std::cout <<  "Failed at SDL_Init" << std::endl;
        window = SDL_CreateWindow("i8080",
                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                        SCREEN_WIDTH, SCREEN_HEIGHT,
                        0);
       	SDL_Delay(1000);
}

void Display::destroy() {
        SDL_DestroyWindow(window);
        SDL_Quit();
}