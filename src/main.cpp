#include <iostream>
#include <SDL2/SDL.h>
#include "initclose.h"
#include "renderer.h"

typedef struct gameT
{
    SDL_Event event;
    int loopDone;
}gameT;

gameT game;

void init()
{
    game.loopDone = 0;
}

void draw()
{
    SDL_SetRenderDrawColor(getRenderer(), 0x3b, 0x3b, 0x3b, 0x00);
    SDL_RenderPresent(getRenderer());

    //Dont be a CPU HOG :D
    SDL_Delay(10);
}

void update()
{

}

void updateInput()
{
    while(SDL_PollEvent(&game.event) != 0)
    {
        switch (game.event.type)
        {
        case SDL_QUIT:
            game.loopDone=1;
            break;

        case SDL_KEYUP:

            switch (game.event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                game.loopDone=1;
                break;
            
            default:
                break;
            }

            break;
        default:
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    
    initSDL((char*)"isometric");
    
    while (!game.loopDone)
    {
        update();
        updateInput();
        draw();
    }
    

    closeDownSDL();
    return 0;
}