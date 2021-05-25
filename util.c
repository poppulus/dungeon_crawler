#include "util.h"

// sdl and rendering stuff
//
bool initSdl(SDL_Window **window, SDL_Renderer **renderer)
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        // Create window
        *window = SDL_CreateWindow(
            "Dungeon Crawler", 
            SDL_WINDOWPOS_UNDEFINED, 
            SDL_WINDOWPOS_UNDEFINED, 
            W_WIDTH, W_HEIGHT, 
            SDL_WINDOW_SHOWN);

        if( *window == NULL )
        {
            printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            // create the renderer
            *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
            if (*renderer == NULL) 
            {
                printf("Could not create renderer! %s\n", SDL_GetError());
                success = false;
            }
            else
            {
                //Initialize PNG loading
                if( !( IMG_Init( IMG_INIT_PNG ) & IMG_INIT_PNG ) )
                {
                    printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
                    success = false;
                }
            }
        }
    }
    return success;
}

bool initTextureMap(SDL_Renderer **renderer, Texture *sheet, char *str)
{
    bool success = true;
    Uint32 format;
    int access, w, h;
    
    sheet->mTexture = loadTexture(*renderer, str);

    if (sheet->mTexture == NULL) 
    { 
        success = false;
        printf("Image could not initialize! SDL Error: %s\n", SDL_GetError());
    }
    else 
    {
        if (SDL_QueryTexture(sheet->mTexture, &format, &access, &w, &h) != 0) 
        {
            success = false;
            printf("Texture invalid!\n");
        }
        
        sheet->mWidth = w;
        sheet->mHeight = h;
    }

    return success;
}

SDL_Texture *loadTexture(SDL_Renderer *renderer, char *path)
{
    SDL_Texture *newTexture = NULL;

    SDL_Surface *loadedSurface = IMG_Load(path);

    if (loadedSurface == NULL) printf("could not load image! %s\n", IMG_GetError());
    else 
    {
        newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        
        if (newTexture == NULL) printf("could not load optimised surface! %s\n", IMG_GetError());

        SDL_FreeSurface(loadedSurface);
    }

    return newTexture;
}

void renderTexture(SDL_Renderer *r, Texture *text, int x, int y, SDL_Rect *clip, const SDL_RendererFlip flip)
{
    SDL_Rect renderQuad = {x, y, text->mWidth, text->mHeight};

    if (clip != NULL) 
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }

    SDL_RenderCopyEx(r, text->mTexture, clip, &renderQuad, 0, NULL, flip);
}

void freeTexture(Texture *text)
{
    if (text->mTexture != NULL)
    {
        SDL_DestroyTexture(text->mTexture);
        text->mTexture = NULL;

        text->mWidth = 0;
        text->mHeight = 0;
    }
}

// game stuff
//
void c_initClips(SDL_Rect *clips)
{
    int n = 0;
    for (int i = 0; i < 21; i++)
    {
        clips[i].w = 32;
        clips[i].h = 32;
        clips[i].x = (i % 7) * 32;
        clips[i].y = n * 32;

        if ((i % 7) == 6) n++;
    }
}

void e_initClips(SDL_Rect *clips)
{
    int n = 0;
    for (int i = 0; i < 37; i++)
    {
        clips[i].w = 8;
        clips[i].h = 8;
        clips[i].x = (i % 5) * 8;
        clips[i].y = n * 8;

        if ((i % 5) == 4) n++;
    }
}

bool collision(int x, int y, int x2, int y2)
{
    int leftA = x, 
        leftB = x2, 
        rightA = x + 20, 
        rightB = x2 + 1, 
        topA = y, 
        topB = y2, 
        bottomA = y + 20, 
        bottomB = y2 + 1;

    if(bottomA <= topB) return false;

    if(topA >= bottomB) return false;

    if(rightA <= leftB) return false;

    if(leftA >= rightB) return false;

    return true;
}

void playerInput(SDL_Event e, game *g, thrd_t *nw_thread)
{
    while (SDL_PollEvent(&e) != 0)
    {
        switch (e.type)
        {
            case SDL_QUIT: g->running = false; break;
            case SDL_KEYDOWN:
                switch (e.key.keysym.sym)
                {
                    case SDLK_h: 
                        if (!g->host && !g->client)
                        {
                            g->host = true;
                            thrd_create(nw_thread, setup_server, g);
                        }
                    break;
                    case SDLK_j:
                        if (!g->host && !g->client)
                        {
                            g->client = true;
                            thrd_create(nw_thread, connect_to_server, g);
                        }
                    break;
                    case SDLK_k:
                        g->kill = true;
                    break;
                    case SDLK_ESCAPE: g->running = false; break;
                    case SDLK_SPACE:
                        if (!e.key.repeat 
                        && !g->players[PLAYER1].attacking 
                        && !g->players[PLAYER1].a_hold) 
                        {
                            g->players[PLAYER1].aindex = 3;
                            g->players[PLAYER1].a_hold = true;
                            enqueue(g->players[PLAYER1].i_queue, ATTACK);
                            g->players[PLAYER1].attacking = true;
                        }
                    break;
                    case SDLK_LSHIFT: 
                        if (!e.key.repeat) 
                            g->players[PLAYER1].sprint = true; 
                    break;
                    case SDLK_w:
                        if (!e.key.repeat) 
                            enqueue(g->players[PLAYER1].i_queue, UP);
                    break;
                    case SDLK_s:
                        if (!e.key.repeat) 
                            enqueue(g->players[PLAYER1].i_queue, DOWN);
                    break;
                    case SDLK_a:
                        if (!e.key.repeat) 
                            enqueue(g->players[PLAYER1].i_queue, LEFT);
                    break;
                    case SDLK_d:
                        if (!e.key.repeat) 
                            enqueue(g->players[PLAYER1].i_queue, RIGHT);
                    break;
                }
            break;
            case SDL_KEYUP:
                switch (e.key.keysym.sym)
                {
                    case SDLK_SPACE:
                        g->players[PLAYER1].a_hold = false; 
                    break;
                    case SDLK_LSHIFT: 
                        g->players[PLAYER1].sprint = false; 
                    break;
                    case SDLK_w:
                        dequeue(g->players[PLAYER1].i_queue, UP);
                    break;
                    case SDLK_a:
                        dequeue(g->players[PLAYER1].i_queue, LEFT);
                    break;
                    case SDLK_s:
                        dequeue(g->players[PLAYER1].i_queue, DOWN);
                    break;
                    case SDLK_d:
                        dequeue(g->players[PLAYER1].i_queue, RIGHT);
                    break;
                }
            break;
        }
    }
}

void setGamePlayers(player p[4])
{
    for (int i = 0; i < 4; i++)
    {
        setPlayerState(&p[i]);
    }
}

void setPlayerState(player *p)
{
    /*
    p->w = 14;
    p->h = 30;
    p->x = 320 - p->w;
    p->y = 240 - p->h;
    p->rx = (320 - p->w) / T_SIZE;
    p->ry = (240 - p->h) / T_SIZE;
    p->dir = -1;
    p->face = DOWN;
    p->xvel = 0;
    p->yvel = 0;
    p->moving = false;
    p->attacking = false;
    p->a_hold = false;
    p->sprint = false;
    p->spawned = false;
    p->acounter = 0;
    p->aindex = 1;
    p->atk_counter = 0;
    */

    memset(p, 0, sizeof(*p));
    memset(p->i_queue, 255, sizeof(p->i_queue));
}

void updateLocalPlayer(player *p)
{
    p->xvel = 0;
    p->yvel = 0;

    p->dir = p->i_queue[0];

    if ((p->i_queue[0] != 255) 
    && (p->i_queue[0] != ATTACK) && !p->attacking) 
        p->face = p->i_queue[0];

    if (!p->attacking)
    {
        switch (p->dir)
        {
            case UP: 
                p->yvel = p->sprint ? -2 : -1;
                p->a_hitBox.x = p->x + 7; 
                p->a_hitBox.y = p->y;
            break;
            case LEFT: 
                p->xvel = p->sprint ? -2 : -1;
                p->a_hitBox.x = p->x; 
                p->a_hitBox.y = p->y + 15;
            break;
            case DOWN: 
                p->yvel = p->sprint ? 2 : 1; 
                p->a_hitBox.x = p->x + 7; 
                p->a_hitBox.y = p->y + p->h;
            break;
            case RIGHT: 
                p->xvel = p->sprint ? 2 : 1; 
                p->a_hitBox.x = p->x + p->w; 
                p->a_hitBox.y = p->y + 15;
            break;
        }
    }

    animatePlayer(p);

    if (p->x + p->xvel <= 0 || p->x + p->xvel >= 640)
    {
        p->xvel = 0;
    }
    if (p->y + p->yvel <= 0 || p->y + p->yvel >= 480)
    {
        p->yvel = 0;
    }

    p->x += p->xvel;
    p->y += p->yvel;
}

void updateOtherPlayer(player *p)
{
    if (p->spawned)
    {
        // animation
        if (p->attacking)
        {
            p->acounter = 0;
            p->aindex = 4;

            if (p->atk_counter < 5) p->aindex = 4;

            else if (p->atk_counter >= 5 && p->atk_counter < 10)
                p->aindex = 5;
            
            else if (p->atk_counter >= 10) 
                p->aindex = 6;

            if (p->atk_counter + 1 == 15) p->aindex = 0;
        }
        else
        {
            p->acounter++;

            if (p->nx != p->x && p->ny == p->y)
            {
                if (p->nx > p->x)
                {
                    p->face = RIGHT;
                }
                else if (p->nx < p->x)
                {
                    p->face = LEFT;
                }

                if (((p->nx == p->x + 1) || (p->nx == p->x - 1)) 
                && (p->acounter % 8 == 0))
                {
                    p->aindex++;
                    p->aindex = (p->aindex % 2) + 2;
                }
                else if (((p->nx == p->x + 2) || (p->nx == p->x - 2)) 
                && (p->acounter % 6 == 0))
                {
                    p->aindex++;
                    p->aindex = (p->aindex % 2) + 2;
                }
            }
            else if (p->ny != p->y && p->nx == p->x)
            {
                if (p->ny > p->y)
                {
                    p->face = DOWN;
                }
                else if (p->ny < p->y)
                {
                    p->face = UP;
                }

                if (((p->ny == p->y + 1) || (p->ny == p->y - 1)) 
                && (p->acounter % 8 == 0))
                {
                    p->aindex++;
                    p->aindex = (p->aindex % 2) + 2;
                }
                else if (((p->ny == p->y + 2) || (p->ny == p->y - 2)) 
                && (p->acounter % 6 == 0))
                {
                    p->aindex++;
                    p->aindex = (p->aindex % 2) + 2;
                }
            }
            else
            {
                if (p->acounter % 16 == 0)
                {
                    p->aindex++;
                    p->aindex %= 2;
                }
            }
        }

        p->x = p->nx;
        p->y = p->ny;
    }
}

void setMapTColor(SDL_Renderer *r, player p)
{
    switch (p.color)
    {
        case BLUE: 
            SDL_SetRenderDrawColor(r, 0x00, 0x00, 0xff, 0xff); 
        break;
        case GREEN:
            SDL_SetRenderDrawColor(r, 0x00, 0xff, 0x00, 0xff); 
        break;
        case RED:
            SDL_SetRenderDrawColor(r, 0xff, 0x00, 0x00, 0xff); 
        break;
        case YELLOW:
            SDL_SetRenderDrawColor(r, 0xff, 0xff, 0x00, 0xff); 
        break;
    }
}

void animatePlayer(player *p)
{
    if (p->attacking)
    {
        if (p->atk_counter % 5 == 0)
        {
            p->aindex++;
            switch (p->face)
            {
                case DOWN:
                    p->rx = p->x;
                    p->ry = p->y + ((p->aindex - 3) * 20);
                break;
                case UP:
                    p->rx = p->x;
                    p->ry = p->y - ((p->aindex - 3) * 20);
                break;
                case LEFT:
                    p->rx = p->x - ((p->aindex - 3) * 20);
                    p->ry = p->y;
                break;
                case RIGHT:
                    p->rx = p->x + ((p->aindex - 3) * 20);
                    p->ry = p->y;    
                break;
            }
        }

        p->atk_counter++;

        if (p->atk_counter >= 15)
        {
            p->atk_counter = 0;
            p->aindex = 0;
            dequeue(p->i_queue, ATTACK);
            p->attacking = false;

            p->rx = p->x;
            p->ry = p->y;
        }
    }
    else 
    {
        p->acounter++;

        if ((p->xvel != 0) || (p->yvel != 0)) 
        {
            if (p->sprint && (p->acounter % 6 == 0))
            {
                p->aindex++;
                p->aindex = (p->aindex % 2) + 2;
            }
            else if (!p->sprint && (p->acounter % 8 == 0))
            {
                p->aindex++;
                p->aindex = (p->aindex % 2) + 2;
            }
        }
        else 
        {
            if (p->acounter % 16 == 0)
            {
                p->aindex++;
                p->aindex %= 2;
            }
        }
    }
}

void renderPlayers(game G)
{
    player p;

    for (int i = 0; i < 4; i++)
    {
        if (G.players[i].spawned)
        {
            p = G.players[i];
            bool flip = (p.face == 3 ? true : false);
            int c_index;
        
            if (p.face == 3) c_index = (2 * 7) + p.aindex;
            else c_index = (p.face * 7) + p.aindex;

            renderTexture(*G.renderer, G.c_texture, 
                p.x - 16, p.y - 28, 
                &G.c_clips[c_index], flip);
        }
    }
}

void enqueue(unsigned char *q, unsigned char val)
{
    for (int i = Q_SIZE - 1; i > 0; i--)
        q[i] = q[i - 1];
    
    q[0] = val;
}

void dequeue(unsigned char *q, unsigned char val)
{
    bool found = false;

    for (int i = 0; i < Q_SIZE; i++)
    {
        if (q[i] == val) 
            found = true;

        if (found && ((i + 1) < Q_SIZE)) 
            q[i] = q[i + 1];
    }

    q[Q_SIZE - 1] = 255;
}

int setup_server(void *ptr)
{
    game *G = ptr;
    network *nw = &G->nw;

    // socket create and verification
    nw->sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (nw->sockfd == -1) {
		printf("socket creation failed...\n");
        thrd_exit(1);
	}
	else
		printf("Socket successfully created..\n");

    if (setsockopt(nw->sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
        thrd_exit(1);
    }

    bzero(&nw->servaddr, sizeof(nw->servaddr));

	// assign IP, PORT
	nw->servaddr.sin_family = AF_INET;
	nw->servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	nw->servaddr.sin_port = htons(8008);

	// Binding newly created socket to given IP and verification
	if ((bind(nw->sockfd, (struct sockaddr*)&nw->servaddr, sizeof(nw->servaddr))) != 0) {
		printf("socket bind failed...\n");
        thrd_exit(1);
	}
	else
		printf("Socket successfully binded..\n");

	nw->addrlen = sizeof(nw->cli);

    // Now server is ready to listen and verification
	if ((listen(nw->sockfd, 3)) == -1) {
		perror("listen");
        thrd_exit(1);
	}
	
	printf("Server listening..\n");

    // just for testing !!!
    G->players[PLAYER1].spawned = true;
    G->players[PLAYER1].nid = nw->sockfd;

    host_loop(G);
    G->host = false;
    G->kill = false;

	// Accept the data packet from client and verification
    /*
	nw->connfd = accept(nw->sockfd, (struct sockaddr*)&nw->cli, &nw->addrlen);
	if (nw->connfd < 0) {
		printf("server acccept failed...\n");
	}
	else
    {
        printf("server acccept the client...\n");

        // just for testing !!!
        G->players[PLAYER2].nid = nw->connfd;
        G->players[PLAYER1].spawned = true;
        G->players[PLAYER2].spawned = true;

        host_loop(G);
        G->host = false;
        G->kill = false;
    }
    */

    close(G->nw.sockfd);

    printf("host thread terminated\n");
    thrd_exit(0);
    return 0;
}

int connect_to_server(void *ptr)
{
    game *G = ptr;
    network *nw = &G->nw;
    // socket create and varification
	nw->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (nw->sockfd == -1) {
		printf("socket creation failed...\n");
        thrd_exit(1);
	}
	else
		printf("Socket successfully created..\n");

	bzero(&nw->servaddr, sizeof(nw->servaddr));

	// assign IP, PORT
	nw->servaddr.sin_family = AF_INET;
	nw->servaddr.sin_addr.s_addr = inet_addr(G->ip);
	nw->servaddr.sin_port = htons(8008);

    if (connect(nw->sockfd, (struct sockaddr*)&nw->servaddr, sizeof(nw->servaddr)) != 0) {
		printf("connection with the server failed...\n");
        thrd_exit(1);
	}
	else 
    {
        printf("connected to the server..\n");

        // testing !!!
        G->players[PLAYER1].spawned = true;
        G->players[PLAYER2].spawned = true;

        client_loop(G);
        G->client = false;
        G->kill = false;
    }

    printf("client thread terminated\n");
    thrd_exit(0);
    return 0;
}

void host_loop(game *G)
{
    Uint32 timer, delta;
    short nbytes, buffer[5];
    bool q = false;

    int fd_counter = 1;

    G->nw.pfds[0].fd = G->nw.sockfd;
    G->nw.pfds[0].events = POLLIN;

    printf("host loop\n");

    while (!q && G->running && !G->kill)
    {
        timer = SDL_GetTicks();
        
        int poll_count = poll(G->nw.pfds, fd_counter, -1);

        if (poll_count == -1)
        {
            perror("poll");
            thrd_exit(1);
        }

        for (int p = 0; p < fd_counter; p++)
        {
            if (G->nw.pfds[p].revents & POLLIN)
            {
                if (G->nw.pfds[p].fd == G->nw.sockfd)
                {
                    G->nw.addrlen = sizeof(G->nw.cli);
                    G->nw.connfd = accept(G->nw.sockfd, (struct sockaddr*)&G->nw.cli, &G->nw.addrlen);

                    if (G->nw.connfd == -1) perror("accept");
                    else
                    {
                        printf("server acccepted the client...\n");
                        G->nw.pfds[p].fd = G->nw.connfd;
                        G->nw.pfds[p].events = POLLIN;

                        G->players[fd_counter].nid = G->nw.connfd;
                        G->players[fd_counter].spawned = true;

                        fd_counter++;
                    }
                    
                }
                else 
                {
                    nbytes = recv(G->nw.pfds[p].fd, buffer, sizeof(buffer), 0);

                    if (nbytes <= 0)
                    {
                        if (nbytes == 0) printf("socket %d hung up\n", G->nw.pfds[p].fd);
                        else perror("recv");

                        close(G->nw.pfds[p].fd);

                        G->nw.pfds[p].fd = 0;
                        G->nw.pfds[p].events = 0;

                        G->players[p].nid = 0;
                        G->players[p].spawned = false;

                        fd_counter--;
                    }
                    else
                    {
                        for (int i = 0; i < fd_counter; i++)
                        {
                            if (G->nw.pfds[p].fd == G->players[i].nid 
                            && G->nw.pfds[p].fd != G->nw.sockfd)
                            {
                                G->players[i].nx = buffer[1];
                                G->players[i].ny = buffer[2];
                                G->players[i].attacking = buffer[3];
                                G->players[i].atk_counter = buffer[4];
                            }
                        }

                        bzero(buffer, sizeof(buffer));
                    }
                    
                    buffer[0] = G->nw.sockfd;
                    buffer[1] = G->players[PLAYER1].x;
                    buffer[2] = G->players[PLAYER1].y;
                    buffer[3] = G->players[PLAYER1].attacking;
                    buffer[4] = G->players[PLAYER1].atk_counter;

                    for (int i = 1; i < fd_counter; i++)
                    {
                        if (G->players[i].spawned)
                        {
                            if (send(G->players[i].nid, buffer, sizeof(buffer), 0) < 0) perror("send");
                        }
                    }
                }
            }
        }

        /* udp
        recvfrom(G->nw.sockfd, buffer, sizeof(buffer), MSG_WAITALL, 
            (struct sockaddr*)&G->nw.cli, 
            &G->nw.addrlen);

        sendto(G->nw.sockfd, buffer, sizeof(buffer), MSG_CONFIRM, 
                (struct sockaddr*)&G->nw.cli, 
                G->nw.addrlen);
        */

        bzero(buffer, sizeof(buffer));

        // set tickrate to ~60
        delta = SDL_GetTicks() - timer;
        if (delta < TICKS) SDL_Delay(TICKS - delta);
    }
}

void client_loop(game *G)
{
    int nbytes, timer, delta;
    short buffer[5];
    bool q = false;

    printf("client loop\n");

    while (!q && G->running && !G->kill)
    {
        //timer = SDL_GetTicks();

        /* udp
            recvfrom(G->nw.sockfd, buffer, sizeof(buffer), MSG_WAITALL, 
                (struct sockaddr*)&G->nw.servaddr, 
                &G->nw.addrlen); 

            sendto(G->nw.sockfd, buffer, sizeof(buffer), MSG_CONFIRM, 
                (struct sockaddr*)&G->nw.servaddr, 
                sizeof(G->nw.servaddr));
        */

        buffer[0] = G->nw.sockfd;
        buffer[1] = G->players[PLAYER1].x;
        buffer[2] = G->players[PLAYER1].y;
        buffer[3] = G->players[PLAYER1].attacking;
        buffer[4] = G->players[PLAYER1].atk_counter;

        send(G->nw.sockfd, buffer, sizeof(buffer), 0);

        bzero(buffer, sizeof(buffer));

        nbytes = recv(G->nw.sockfd, buffer, sizeof(buffer), 0);

        if (nbytes <= 0)
        {
            if (nbytes == 0) printf("socket %d hung up\n", G->nw.sockfd);
            else perror("recv");
            close(G->nw.sockfd);
            q = true;
        }
        else if (nbytes > 0)
        {
            G->players[PLAYER2].nid = buffer[0];
            G->players[PLAYER2].nx = buffer[1];
            G->players[PLAYER2].ny = buffer[2];
            G->players[PLAYER2].attacking = buffer[3];
            G->players[PLAYER2].atk_counter = buffer[4];

            bzero(buffer, sizeof(buffer));
        }

        // set tickrate to ~60
        //delta = SDL_GetTicks() - timer;
        //if (delta < TICKS) SDL_Delay(TICKS - delta);
    }
}

int recv_data(void *ptr)
{
    game *G = ptr;
    int nbytes;
    short buffer[2];

    /*
    while (G->running)
    {
        nbytes = recvfrom(G->nw.sockfd, buffer, sizeof(buffer), 0, 
            (struct sockaddr*)&G->nw.servaddr, 
            &G->nw.addrlen);

        if (nbytes > 0)
        {

        }

        bzero(buffer, sizeof(buffer));
    }
    */

    thrd_exit(0);
    return 0;
}

int send_data(void *ptr)
{
    game *G = ptr;
    int nbytes;
    short buffer[2];

    /*
    while (G->running)
    {
        nbytes = sendto(G->nw.sockfd, buffer, sizeof(buffer), 0, 
            (struct sockaddr*)&G->nw.servaddr, 
            &G->nw.addrlen);

        if (nbytes > 0)
        {
            
        }

        bzero(buffer, sizeof(buffer));
    }
    */

    thrd_exit(0);
    return 0;
}
