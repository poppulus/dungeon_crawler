#include "sdl_template.h"

#define c_anim_idx(a, b) ((a * 3) + b)

enum PlayerFacing
{
    DOWN, 
    LEFT,
    RIGHT, 
    UP
};

typedef struct p_input
{
    int last, current;
} p_input;

typedef struct player
{
    int w, h, 
        x, y,
        face, dir,
        xvel, yvel;

    unsigned char input[4];

    unsigned char   acounter, aindex, 
                    i_queue[4];

    bool moving;
} player;

typedef struct game
{
    SDL_Window **window;
    SDL_Renderer **renderer;
    Texture *texture;

    player *p;

    bool running;
} game;

void initClips(SDL_Rect *);

void playInput(SDL_Event, game *);
void updatePlayer(player *);

void renderPlayer(game, SDL_Rect *);

void enqueue(unsigned char *q, unsigned char val);
void dequeue(unsigned char *q, unsigned char val);
