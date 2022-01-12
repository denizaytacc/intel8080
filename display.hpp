#include <iostream>
#include <SDL2/SDL.h>

class Display{
    private:
    SDL_Renderer* renderer = NULL;
    SDL_Window* window = NULL;
    SDL_Texture* texture;
    public:
    Display();
    void destroy();
};
