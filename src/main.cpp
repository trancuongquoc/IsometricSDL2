#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include "initclose.h"
#include "renderer.h"
#include "texture.h"
#include "isoEngine.h"

#define NUM_ISOMETRIC_TILES 5
#define MAP_HEIGHT 16
#define MAP_WIDTH 16

int worldTest[MAP_HEIGHT][MAP_WIDTH] = {
    {1,1,2,2,2,2,2,2,1,1,2,2,2,2,2,1},
    {1,1,1,1,2,1,1,2,1,1,2,2,2,2,2,1},
    {2,1,1,1,2,2,2,2,1,1,2,2,2,2,2,1},
    {2,1,1,2,2,1,1,2,1,1,2,2,2,2,2,1},
    {2,1,1,4,4,4,1,2,1,1,2,2,2,2,4,1},
    {2,1,1,4,4,4,1,2,1,1,2,2,2,2,2,1},
    {2,1,1,4,4,4,1,2,1,1,2,2,4,2,2,1},
    {2,2,2,4,4,4,2,1,2,3,3,3,4,2,2,1},
    {1,1,2,2,2,2,2,3,4,3,3,3,4,2,2,2},
    {1,1,1,1,2,1,1,2,1,3,3,3,2,2,2,3},
    {2,1,1,1,2,2,2,2,1,1,2,2,2,2,2,1},
    {2,1,1,2,2,1,1,2,1,1,3,2,2,2,4,4},
    {2,1,1,4,2,1,1,2,1,1,3,2,2,2,2,4},
    {2,1,1,1,2,1,1,2,1,1,3,3,3,3,3,4},
    {2,1,1,1,1,1,1,2,1,1,2,2,2,2,4,4},
    {2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,1}
};

typedef struct gameT
{
    SDL_Event event;
    int loopDone;
    SDL_Rect mouseRect;
    point2DT mousePoint;
    point2DT mapScroll2Dpos;
    int mapScrollSpeed;
    isoEngineT isoEngine;
    int lastTileClicked;
    float zoomLevel;
}gameT;

gameT game;
textureT tilesTex;
SDL_Rect tilesRects[NUM_ISOMETRIC_TILES];

void initTileClip()
{
    int x=0, y=0;
    int i;

    textureInit(&tilesTex, 0, 0, 0, NULL, NULL, SDL_FLIP_NONE);

    for (i = 0; i < NUM_ISOMETRIC_TILES; ++i)
    {
        setupRect(&tilesRects[i], x,y,64,80);
        x+=64;
    }
    
}

void writeCoords()
{
    fprintf(stdout,"\rmap x:%d,map y:%d, iso x:%d, iso y:%d\n",
            (int)game.mapScroll2Dpos.x,(int)game.mapScroll2Dpos.y,(int)game.isoEngine.scrollX,(int)game.isoEngine.scrollY);
}

void init()
{
    int tileSize = 32;
    game.loopDone = 0;
    initTileClip();
    InitIsoEngine(&game.isoEngine, tileSize);
    IsoEngineSetMapSize(&game.isoEngine, 16, 16);

    game.isoEngine.scrollX = 77;
    game.isoEngine.scrollY = -77;
    game.mapScroll2Dpos.x = -77;
    game.mapScroll2Dpos.y = 0;
    game.mapScrollSpeed = 3;
    game.lastTileClicked = -1;
    game.zoomLevel = 1.25;

    if (loadTexture(&tilesTex, "data/isotiles.png") == 0)
    {
        fprintf(stderr, "Error, could not load texture: data/isotiles.png\n");
        exit(1);
    }    
}

void drawIsoMouse()
{
    int modulusX = TILESIZE*game.zoomLevel;
    int modulusY = TILESIZE*game.zoomLevel;
    int correctX =(((int)game.mapScroll2Dpos.x)%modulusX)*2;
    int correctY = ((int)game.mapScroll2Dpos.y)%modulusY;

    game.mousePoint.x = (game.mouseRect.x/TILESIZE) * TILESIZE;
    game.mousePoint.y = (game.mouseRect.y/TILESIZE) * TILESIZE;

    //For every other x position on the map
    if(((int)game.mousePoint.x/TILESIZE)%2){
        //Move the mouse down by half a tile so we can
        //pick isometric tiles on that row as well.
        game.mousePoint.y+=TILESIZE*0.5;
    }
    textureRenderXYClipScale(&tilesTex,(game.zoomLevel*game.mousePoint.x)-correctX,
                             (game.zoomLevel*game.mousePoint.y)+correctY,&tilesRects[0],game.zoomLevel);
}

void drawIsoMap(isoEngineT *isoEngine)
{
    int i,j;
    int tile;

    point2DT point;

    //loop through the map
    for(i=0;i<isoEngine->mapHeight;++i)
    {
        for(j=0;j<isoEngine->mapWidth;++j)
        {
            point.x = (j *game.zoomLevel *TILESIZE) + isoEngine->scrollX;
            point.y = (i *game.zoomLevel *TILESIZE) + isoEngine->scrollY;

            tile = worldTest[i][j];

            Convert2dToIso(&point);

            textureRenderXYClipScale(&tilesTex,point.x,point.y,&tilesRects[tile],game.zoomLevel);
        }
    }
}

void getMouseTilePos(isoEngineT *isoEngine, point2DT *mouseTilePos)
{
    point2DT point;
    point2DT tileShift, mouse2IsoPOint;

    if(isoEngine == NULL || mouseTilePos == NULL){
        return;
    }

    int modulusX = TILESIZE*game.zoomLevel;
    int modulusY = TILESIZE*game.zoomLevel;
    int correctX =(((int)game.mapScroll2Dpos.x)%modulusX)*2;
    int correctY = ((int)game.mapScroll2Dpos.y)%modulusY;

    //copy mouse point
    mouse2IsoPOint = game.mousePoint;
    ConvertIsoTo2D(&mouse2IsoPOint);

    //get tile coordinates
    GetTileCoordinates(&mouse2IsoPOint,&point);

    tileShift.x = correctX;
    tileShift.y = correctY;
    Convert2dToIso(&tileShift);

    //check for fixing tile position when the y position is larger than 0
    if(game.mapScroll2Dpos.y>0){
        point.y -= (((float)isoEngine->scrollY-tileShift.y)/(float)TILESIZE)/game.zoomLevel;
        point.y+=1;
    }
    else{
        point.y -= (((float)isoEngine->scrollY-tileShift.y)/(float)TILESIZE)/game.zoomLevel;
    }

    //check for fixing tile position when the x position is larger than 0
    if(game.mapScroll2Dpos.x>0)
    {
        point.x -= (((float)isoEngine->scrollX+(float)tileShift.x)/(float)TILESIZE)/game.zoomLevel;
        point.x+=1;
    }
    else{
        point.x -= (((float)isoEngine->scrollX+(float)tileShift.x)/(float)TILESIZE)/game.zoomLevel;
    }
    mouseTilePos->x = (int)point.x;
    mouseTilePos->y = (int)point.y;
}

void getMouseTileClick(isoEngineT *isoEngine)
{
    point2DT point;
    getMouseTilePos(isoEngine,&point);
    if(point.x>=0 && point.y>=0 && point.x<MAP_WIDTH && point.y<MAP_HEIGHT)
    {
        game.lastTileClicked = worldTest[(int)point.y][(int)point.x];
    }
}

void draw()
{
    SDL_SetRenderDrawColor(getRenderer(), 0x3b, 0x3b, 0x3b, 0x00);
    SDL_RenderClear(getRenderer());
    // textureRenderXYClip(&tilesTex, game.mouseRect.x, game.mouseRect.y, &tilesRects[0]);

    drawIsoMap(&game.isoEngine);
    drawIsoMouse();

    if(game.lastTileClicked != -1) {
        textureRenderXYClip(&tilesTex, 0, 0, &tilesRects[game.lastTileClicked]);

    }

    SDL_RenderPresent(getRenderer());

    //Dont be a CPU HOG :D
    SDL_Delay(10);
}

void update()
{
    SDL_GetMouseState(&game.mouseRect.x, &game.mouseRect.y);
    game.mouseRect.x = game.mouseRect.x/game.zoomLevel;
    game.mouseRect.y = game.mouseRect.y/game.zoomLevel;
}

void updateInput()
{
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);

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

        case SDL_MOUSEBUTTONDOWN:
            if(game.event.button.button == SDL_BUTTON_LEFT)
            {
                getMouseTileClick(&game.isoEngine);
            }
            break;

        case SDL_MOUSEWHEEL:
            //if the user scrolled the mouse wheel up
            if (game.event.wheel.y >= 1)
            {
                if (game.zoomLevel < 3.0)
                {
                    game.zoomLevel += 0.25;
                }
            //if the user scrolled the mouse wheel up
            } else {
                if (game.zoomLevel > 1.0)
                {
                    game.zoomLevel -= 0.25;
                }
                
            }
            break;
            
        default:
            break;
        }
    }

    if (keystate[SDL_SCANCODE_W])
    {
        game.mapScroll2Dpos.y += game.mapScrollSpeed;

        if (game.mapScroll2Dpos.y > 0)
        {
            game.mapScroll2Dpos.y = 0;
            game.isoEngine.scrollX -= game.mapScrollSpeed;
            game.isoEngine.scrollY -= game.mapScrollSpeed;
        }
        convertCartesianCameraToIsometric(&game.isoEngine, &game.mapScroll2Dpos);
        writeCoords();
    }
    if (keystate[SDL_SCANCODE_A])
    {
        game.mapScroll2Dpos.x -= game.mapScrollSpeed;
        
        convertCartesianCameraToIsometric(&game.isoEngine, &game.mapScroll2Dpos);
        writeCoords();
    }
    if (keystate[SDL_SCANCODE_S])
    {
        game.mapScroll2Dpos.y -= game.mapScrollSpeed;

        convertCartesianCameraToIsometric(&game.isoEngine, &game.mapScroll2Dpos);
        writeCoords();
    }
    if (keystate[SDL_SCANCODE_D])
    {
        game.mapScroll2Dpos.x += game.mapScrollSpeed;

        convertCartesianCameraToIsometric(&game.isoEngine, &game.mapScroll2Dpos);
        writeCoords();
    }
    
}

int main(int argc, char *argv[])
{
    
    initSDL((char*)"isometric");
    init();

    while (!game.loopDone)
    {
        update();
        updateInput();
        draw();
    }
    

    closeDownSDL();
    return 0;
}