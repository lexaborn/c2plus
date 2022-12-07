#include <iostream>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>
#include "Window.h"

int main(int, char **) {

    SDL_Init(SDL_INIT_EVERYTHING);

    Window w { 1440, 720 };

    w.main_loop();

    return 0;
}
