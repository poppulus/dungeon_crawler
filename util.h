#include "sdl_template.h"
#include <unistd.h>
#include <threads.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

#define W_WIDTH 640
#define W_HEIGHT 480

#define FPS 60
#define TICKS 1000/FPS

#define c_anim_idx(a, b) ((a * 7) + b)
#define Q_SIZE 5
#define ATTACK 4

#define T_SIZE 20

enum TileColor
{
    BLUE,
    GREEN,
    RED,
    YELLOW
};

enum PlayerFacing
{
    DOWN, 
    UP,
    RIGHT, 
    LEFT
};

enum PlayerIndex
{
    PLAYER1,
    PLAYER2,
    PLAYER3,
    PLAYER4
};

enum N_PINDEX
{
    N_P1,
    N_P2 = 5,
    N_P3 = 10,
    N_P4 = 15
};

enum G_STATE
{
    MENU,
    HOST,
    JOIN,
    PLAY
};

typedef struct p_input
{
    int last, current;
} p_input;

typedef struct network
{
    struct pollfd *pfds;
    struct sockaddr_in servaddr, cli;
    int sockfd, connfd, addrlen, n;
} network;

typedef struct p_next
{
    int x, y, atk_counter;
    bool attacking;
} p_next;

typedef struct player
{
    SDL_Rect *clips, a_hitBox;
    p_next nextmove;

    int w, h, 
        x, y,
        face, dir,
        xvel, yvel,
        rx, ry,
        nid;

    int color;

    unsigned char input[Q_SIZE];

    unsigned char   acounter, aindex, 
                    atk_counter, 
                    i_queue[Q_SIZE];

    bool moving:1, attacking:1, a_hold:1, sprint:1, spawned:1;
} player;

typedef struct game
{
    SDL_Window      **window;
    SDL_Renderer    **renderer;
    Texture         *c_texture, *e_texture;
    network         nw;
    SDL_Rect        *c_clips;
    player          *players;
    player          *c_player;
    const char      *ip;
    unsigned char   state;
    bool            running, host, client, kill;
} game;

void char_initClips(SDL_Rect *);
void e_initClips(SDL_Rect *);

bool collision(SDL_Rect a, SDL_Rect b);
void checkMapCollision(game, SDL_Rect *block, unsigned char (*map_blocks)[]);
void checkPlayerAtkCol(player *);

void menuInput(SDL_Event, game *, thrd_t *nw_thread);
void playInput(SDL_Event, game *);

void initPlayers(player [4]);
void setPlayerState(player *);

void selectPlayerSlot(player *players, int cfd);
void removePlayerSlot(player *players, int cfd);

void updateLocalPlayer(player *);
void updateOtherPlayer(player *);

void setMapTColor(SDL_Renderer *r, player p);

void animatePlayer(player *);
void playerWalking();
void playerAttacking();
void renderPlayers(game);

void enqueue(unsigned char *q, unsigned char val);
void dequeue(unsigned char *q, unsigned char val);

int setup_server(void *ptr);
int connect_to_server(void *ptr);

void host_loop(game *G);
void client_loop(game *G);

void c_player_update(game *G, int i, short *buffer);
void h_player_update(short *buffer, player *players, int sfd);
