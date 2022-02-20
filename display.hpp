#include <iostream>
#include <SDL2/SDL.h>

class Display{
    private:
    SDL_Renderer* renderer = NULL;
    SDL_Window* window = NULL;
    SDL_Texture* texture;
    public:
    SDL_Event event;
    // https://wiki.libsdl.org/SDL_GetKeyboardState
    const uint8_t *state = SDL_GetKeyboardState(NULL);
    Display();
    void destroy();
};
