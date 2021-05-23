#include "sdl_template.h"
#include <unistd.h>
#include <threads.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define W_WIDTH 640
#define W_HEIGHT 480

#define FPS 60
#define TICKS 1000/FPS

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

typedef struct network
{
    struct sockaddr_in servaddr, cli;
    int sockfd, connfd, addrlen, n;
} network;

typedef struct player
{
    SDL_Rect *clips, a_hitBox;

    int w, h, 
        x, y,
        nx, ny,
        face, dir,
        xvel, yvel,
        rx, ry;

    unsigned char input[Q_SIZE];

    unsigned char   acounter, aindex, 
                    atk_counter, 
                    i_queue[Q_SIZE];

    bool moving, attacking, a_hold, sprint, spawned;
} player;

typedef struct game
{
    SDL_Window **window;
    SDL_Renderer **renderer;
    Texture *c_texture, *e_texture;

    network nw;

    player *p, *p2;

    bool running, host, client, kill;
} game;

void c_initClips(SDL_Rect *);
void e_initClips(SDL_Rect *);

bool collision(int x, int y, int x2, int y2);

void playerInput(SDL_Event, game *, thrd_t *nw_thread);
void setPlayerState(player *);
void updatePlayer(player *);

void updateOnOff(player *);

void animatePlayer(player *);
void playerWalking();
void playerAttacking();
void renderPlayer(game, int nmbr);

void enqueue(unsigned char *q, unsigned char val);
void dequeue(unsigned char *q, unsigned char val);

int setup_server(void *ptr);
int connect_to_server(void *ptr);

void host_loop(game *G);
void client_loop(game *G);

int recv_data(void *ptr);
int send_data(void *ptr);
