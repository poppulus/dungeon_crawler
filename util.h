#include "sdl_template.h"

enum PlayerFacing
{
    UP, 
    LEFT,
    DOWN, 
    RIGHT
};

typedef struct player
{
    int w, h, 
        x, y,
        face, dir,
        xvel, yvel;
} player;

typedef struct game
{
    player *p;

    bool running;
} game;

void initClips(SDL_Rect *);

void playInput(SDL_Event, game *);
void updtPlyrPos(player *);
