#include "sdl_template.h"

#define c_anim_idx(a, b) ((a * 7) + b)
#define Q_SIZE 5
#define ATTACK 4

enum PlayerFacing
{
    DOWN, 
    UP,
    RIGHT, 
    LEFT
};

typedef struct p_input
{
    int last, current;
} p_input;

typedef struct player
{
    SDL_Rect *clips, a_hitBox;

    int w, h, 
        x, y,
        face, dir,
        xvel, yvel;

    unsigned char input[Q_SIZE];

    unsigned char   acounter, aindex, 
                    atk_counter, 
                    i_queue[Q_SIZE];

    bool moving, attacking, a_hold, sprint;
} player;

typedef struct game
{
    SDL_Window **window;
    SDL_Renderer **renderer;
    Texture *c_texture, *e_texture;

    player *p;

    bool running;
} game;

void c_initClips(SDL_Rect *);
void e_initClips(SDL_Rect *);

bool collision(SDL_Rect, int x, int y);

void playerInput(SDL_Event, game *);
void updatePlayer(player *);

void animatePlayer(player *);
void playerWalking();
void playerAttacking();
void renderPlayer(game);

void enqueue(unsigned char *q, unsigned char val);
void dequeue(unsigned char *q, unsigned char val);
