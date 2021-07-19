#include "sdl_template.h"
#include <unistd.h>
#include <threads.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <poll.h>

#define W_WIDTH 640
#define W_HEIGHT 480

#define FPS 60
#define TICKS 1000/FPS

#define c_anim_idx(a, b) ((a * 7) + b)
#define Q_SIZE 5
#define ATTACK 4

#define T_SIZE 20
#define NW_P_SIZE 5

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

enum P_PUSH_D
{
    PUSH_NONE,
    PUSH_UP,
    PUSH_LEFT,
    PUSH_DOWN,
    PUSH_RIGHT
};

enum I_STATE
{
    I_UP = 1,
    I_LEFT = 2,
    I_DOWN = 4,
    I_RIGHT = 8,
    I_ATK = 16,
    I_RUN = 32
};

enum G_STATE
{
    MENU,
    HOST,
    JOIN,
    PLAY
};

typedef struct g_rules
{
    unsigned short  g_timer,  
                    p1_score, 
                    p2_score, 
                    p3_score, 
                    p4_score;
                    
    char    p1_buf[4], 
            p2_buf[4], 
            p3_buf[4], 
            p4_buf[4];
} g_rules;

typedef struct network
{
    struct pollfd *pfds;
    struct sockaddr_in servaddr, cli;
    int sockfd, connfd, addrlen, n;
} network;

typedef struct nw_player
{
    short x, y;
    unsigned char fd, input;
} nw_player;

typedef struct p_next
{
    int x, y, atk_counter;
    bool attacking;
} p_next;

typedef struct p_cube
{
    SDL_Rect rect;
    unsigned char xvel, yvel, plr;
} p_cube;

typedef struct player
{
    SDL_Rect *clips, a_hitBox;
    p_next nextmove;
    p_cube cube;
    int w, h, 
        x, y,
        face, dir, p_dir,
        xvel, yvel,
        rx, ry,
        nid, color;

    unsigned char   input, 
                    acounter, aindex, 
                    atk_counter, 
                    i_queue[Q_SIZE],
                    hurt_counter,
                    push_counter;
                    
    bool blocked:1, attacking:1, a_hold:1, 
         sprint:1, spawned:1, ready:1, hurt:1;
} player;

typedef struct game
{
    SDL_Window      **window;
    SDL_Renderer    **renderer;
    Texture         *c_texture, *e_texture;
    FC_Font         *font;
    network         nw;
    g_rules         rules;
    SDL_Rect        *c_clips;
    player          players[4];
    player          *c_player;
    const char      *ip;
    unsigned char   state;
    char            g_count[3];
    bool            running:1, s_cntdwn:1, 
                    g_cntdwn:1, g_done:1, g_ready:1, 
                    host:1, client:1, kill:1;
    char            s_count;
} game;

int sortfunc(const void *a, const void *b);
int r_sortfunc(const void *a, const void *b);

void char_initClips(SDL_Rect *);
void e_initClips(SDL_Rect *);

bool collision(SDL_Rect a, SDL_Rect b);
void checkMapCollision(game *, SDL_Rect *block, unsigned char (*map_blocks)[]);
void checkPlayerAtkCol(player *);

bool checkPlayerReady(player *);

void menuInput(SDL_Event, game *, thrd_t *nw_thread);
void playInput(SDL_Event, game *);

void initPlayers(player [4]);
void setPlayerState(player *);
void setAttackBox(player *);

void selectPlayerSlot(player *players, int cfd);
void removePlayerSlot(player *players, int cfd);

void updateLocalPlayer(player *);
void updateOtherPlayer(player *);
void updateClient(player *);
void addPlayerScore(char *);
void subPlayerScore(char *);

void setMapTColor(SDL_Renderer *r, player p);

void animatePlayer(player *);
void playerWalking(void);
void playerAttacking(void);
void renderPlayer(game, player *);

void setRenderOrder(game);
void renderScore(game);

int decideWinner(game);

void enqueue(unsigned char *q, unsigned char val);
void dequeue(unsigned char *q, unsigned char val);

int setup_server(void *ptr);
int connect_to_server(void *ptr);

void host_loop(game *G);
void client_loop(game *G);

void c_player_update(game *G, int i, short *buffer);
void h_player_update(short *buffer, player *players);
