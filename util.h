#include "sdl_template.h"

#define c_anim_idx(a, b) ((a * 3) + b)
#define Q_SIZE 5
#define ATTACK 4

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
    SDL_Rect *clips;

    int w, h, 
        x, y,
        face, dir,
        xvel, yvel;

    unsigned char input[Q_SIZE];

    unsigned char   acounter, aindex, 
                    atk_counter, 
                    i_queue[Q_SIZE];

    bool moving, attacking, a_hold;
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

bool collision(SDL_Rect, int x, int y);

void playerInput(SDL_Event, game *);
void updatePlayer(player *);

void animatePlayer(player *);
void playerWalking();
void playerAttacking();
void renderPlayer(game);

void enqueue(unsigned char *q, unsigned char val);
void dequeue(unsigned char *q, unsigned char val);
