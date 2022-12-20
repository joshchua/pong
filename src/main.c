#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#define LOG_CAT_GENERAL 0

#define WINDOW_TITLE "Pong"
#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 300

#define BALL_LENGTH 10
#define BALL_INIT_SPEED 3
#define BALL_ACCELERATION 0.005
#define BALL_ANGLES { \
        5*M_PI/3, 7*M_PI/4, 11*M_PI/6, 0,       \
        0,        M_PI/6,   M_PI_4,    M_PI/3,  \
        4*M_PI/3, 5*M_PI_4, 7*M_PI/6,  M_PI,    \
        M_PI,     5*M_PI/6, 3*M_PI_4,  2*M_PI/3 }
#define BALL_ANGLES_SIZE 16

#define PADDLE_WIDTH 10
#define PADDLE_HEIGHT 40
#define PADDLE_DY 5
#define PADDLE_HORIZONTAL_OFFSET 0
#define PADDLE_VERTICAL_OFFSET 20

/**
 * \struct game
 * \brief The root game structure.
 */
struct game
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool is_running;

    SDL_FRect ball;
    float ball_dy;
    float ball_dx;
    float ball_speed;
    
    SDL_FRect paddle1;
    float paddle1_dy;
    
    SDL_FRect paddle2;
    float paddle2_dy;
};

/**
 * \fn void reset_ball(struct game *g)
 * \brief Resets the ball position and randomizes its velocity.
 * \param g [in] A pointer to the game structure.
 */
void reset_ball(struct game *g)
{
    int window_width;
    int window_height;
    SDL_GL_GetDrawableSize(g->window, &window_width,
                           &window_height);
    
    g->ball.x = (window_width / 2) - (g->ball.w / 2);
    g->ball.y = (window_height / 2) - (g->ball.h / 2);

    static float angles[BALL_ANGLES_SIZE] = BALL_ANGLES;
    int i = rand() % (BALL_ANGLES_SIZE - 1);
    g->ball_dx = cosf(angles[i]);
    g->ball_dy = sinf(angles[i]);
    g->ball_speed = BALL_INIT_SPEED;
}

/**
 * \fn struct game *init_game()
 * \brief Initializes the game, including SDL2 and game state.
 * \return A pointer to the game structure.
 */
struct game *init_game()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogCritical(LOG_CAT_GENERAL,
                        "Failed to initialize SDL2.");
        exit(1);
    }
    
    SDL_Window *window = SDL_CreateWindow(WINDOW_TITLE,
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH,
                                          WINDOW_HEIGHT,
                                          0);
    if (!window) {
        SDL_LogCritical(LOG_CAT_GENERAL, "Failed to create window.");
        exit(1);
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        SDL_LogCritical(LOG_CAT_GENERAL,
                        "Failed to create renderer.");
        exit(1);
    }
    
    struct game *game = (struct game *)malloc(sizeof(struct game));
    game->window = window;
    game->renderer = renderer;

    
    game->ball.h = BALL_LENGTH;
    game->ball.w = BALL_LENGTH;

    reset_ball(game);

    int window_width;
    int window_height;
    SDL_GL_GetDrawableSize(game->window, &window_width,
                           &window_height);

    game->paddle1.w = PADDLE_WIDTH;
    game->paddle1.h = PADDLE_HEIGHT;
    game->paddle1.x = PADDLE_HORIZONTAL_OFFSET;
    game->paddle1.y = (window_height / 2) - (PADDLE_HEIGHT / 2);

    game->paddle1_dy = 0;

    game->paddle2.w = PADDLE_WIDTH;
    game->paddle2.h = PADDLE_HEIGHT;
    game->paddle2.x = window_width - PADDLE_HORIZONTAL_OFFSET
        - PADDLE_WIDTH;
    game->paddle2.y = (window_height / 2) - (PADDLE_HEIGHT / 2);

    game->paddle2_dy = 0;
    
    return game;
}

/**
 * \fn void free_game(struct game *g)
 * \brief Deinitializes the game structure.
 * \param [in] g A pointer to the game structure.
 */
void free_game(struct game *g)
{
    SDL_DestroyRenderer(g->renderer);
    SDL_DestroyWindow(g->window);
    SDL_Quit();
    free(g);
}

/**
 * \fn void handle_keydown(struct game *g, SDL_Keysym *k)
 * \brief Handles the keydown event for the given key.
 * \param g [in] A pointer to the game structure.
 * \param k [in] A pointer to the keysym of the event.
 */
void handle_keydown(struct game *g, SDL_Keysym *k)
{
    switch (k->sym) {
    case SDLK_w:
        g->paddle1_dy = -PADDLE_DY;
        break;
    case SDLK_s:
        g->paddle1_dy = +PADDLE_DY;
        break;
    case SDLK_UP:
        g->paddle2_dy = -PADDLE_DY;
        break;
    case SDLK_DOWN:
        g->paddle2_dy = +PADDLE_DY;
        break;
    default:
        break;
    }
}

/**
 * \fn void handle_keyup(struct game *g, SDL_Keysym *k)
 * \brief Handles the keyup event for the given key.
 * \param g [in] A pointer to the game structure.
 * \param k [in] A pointer to the keysym of the event.
 */
void handle_keyup(struct game *g, SDL_Keysym *k)
{
     switch (k->sym) {
     case SDLK_w:
         g->paddle1_dy = 0;
         break;
     case SDLK_s:
         g->paddle1_dy = 0;
         break;
     case SDLK_UP:
         g->paddle2_dy = 0;
         break;
     case SDLK_DOWN:
         g->paddle2_dy = 0;
         break;
     default:
         break;
     }
}

/**
 * \fn void poll_events(struct game *g)
 * \brief Polls events and handles QUIT, keydown, and keyup.
 * \param g [in] A pointer to the game structure.
 */
void poll_events(struct game *g)
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            g->is_running = false;
        } else if (e.type == SDL_KEYDOWN) {
            handle_keydown(g, &e.key.keysym);
        } else if (e.type == SDL_KEYUP) {
            handle_keyup(g, &e.key.keysym);
        }
    }        
}

/**
 * \fn void prepare_scene(struct game *g)
 * \brief Clears the screen with a black color.
 * \param g [in] A pointer to the game structure.
 */
void prepare_scene(struct game *g)
{
    SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, 255);
    SDL_RenderClear(g->renderer);
}

/**
 * \fn void handle_paddle1_collision(struct game *g)
 * \brief Reflects the ball when the it hits paddle1.
 * \param g [in] A pointer to the game structure.
 */
void handle_paddle1_collision(struct game *g)
{
    SDL_FRect *b = &g->ball;
    SDL_FRect *p = &g->paddle1;

    bool in_range_x = (b->x <= p->x + p->w + 1);
    bool in_range_y = (b->y + b->h >= p->y && b->y <= p->y + p->h);
    if (!in_range_x || !in_range_y) {
        return;
    }

    float bucket_length = p->h / (BALL_ANGLES_SIZE / 2);
    float test = b->y + (b->h / 2) - p->y;
    int bucket = (int)(test / bucket_length);
    static float angles[BALL_ANGLES_SIZE] = BALL_ANGLES;
    float angle = angles[bucket];
    g->ball_dx = cosf(angle);
    g->ball_dy = sinf(angle);
}


/**
 * \fn void handle_paddle2_collision(struct game *g)
 * \brief Reflects the ball when the it hits paddle2.
 * \param g [in] A pointer to the game structure.
 */
void handle_paddle2_collision(struct game *g)
{
    SDL_FRect *b = &g->ball;
    SDL_FRect *p = &g->paddle2;

    bool in_range_x = (b->x + b->w >= p->x - 1);
    bool in_range_y = (b->y + b->h >= p->y && b->y <= p->y + p->h);
    if (!in_range_x || !in_range_y) {
        return;
    }

    float bucket_length = p->h / (BALL_ANGLES_SIZE / 2);
    float test = b->y + (b->h / 2) - p->y;
    int bucket = (int)(test / bucket_length);
    static float angles[BALL_ANGLES_SIZE] = BALL_ANGLES;
    float angle = angles[bucket + (BALL_ANGLES_SIZE / 2)];
    g->ball_dx = cosf(angle);
    g->ball_dy = sinf(angle);
}

/**
 * \fn void move_ball(struct game *g)
 * \brief Moves the ball and reflects the ball against the window and
 *        paddles as needed.
 * \pram g [in] A pointer to the game structure.
 */
void move_ball(struct game *g)
{
    g->ball_speed += BALL_ACCELERATION;
    g->ball.x += g->ball_dx * g->ball_speed;
    g->ball.y += g->ball_dy * g->ball_speed;

    int window_width;
    int window_height;
    SDL_GL_GetDrawableSize(g->window, &window_width,
                           &window_height);

    if (g->ball.x < 0 ||
        g->ball.x + g->ball.w > window_width) {
        reset_ball(g);
    }

    if (g->ball.y < 0 || g->ball.y + g->ball.h > window_height) {
        g->ball_dy *= -1;
    }

    handle_paddle1_collision(g);
    handle_paddle2_collision(g);
}

/**
 * \fn void move_paddle(struct game *g, SDL_FRect *paddle, float *dy)
 * \brief Changes a given paddle's position given its velocity.
 * \param g [in] A pointer to the game structure.
 * \param paddle [in] A pointer to the paddle to move.
 * \param dy The velocity of the paddle.
 */
void move_paddle(struct game *g, SDL_FRect *paddle, float dy)
{
    int window_height;
    SDL_GL_GetDrawableSize(g->window, NULL, &window_height);

    paddle->y += dy;

    if (paddle->y < PADDLE_VERTICAL_OFFSET) {
        paddle->y = PADDLE_VERTICAL_OFFSET;
    } else if (paddle->y + paddle->h >
               window_height - PADDLE_VERTICAL_OFFSET) {
        paddle->y = window_height - paddle->h
            - PADDLE_VERTICAL_OFFSET;
    }
}

/**
 * \fn void draw_white_rect(SDL_Renderer *renderer, SDL_FRect *rect)
 * \brief Draws a white rectangle on the window.
 * \param renderer [in] A pointer to the renderer.
 * \param rect [in] A pointer to the rect to draw.
 */
void draw_white_rect(SDL_Renderer *renderer, SDL_FRect *rect)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRectF(renderer, rect);
}

/**
 * \fn void game_loop(struct game *g)
 * \brief The game's main loop.
 * \param [in] game The game structure.
 */
void game_loop(struct game *g)
{
    g->is_running = true;
    while (g->is_running) {
        prepare_scene(g);
        poll_events(g);
        move_paddle(g, &g->paddle1, g->paddle1_dy);
        move_paddle(g, &g->paddle2, g->paddle2_dy);
        move_ball(g);        
        draw_white_rect(g->renderer, &g->ball);
        draw_white_rect(g->renderer, &g->paddle1);
        draw_white_rect(g->renderer, &g->paddle2);
        SDL_RenderPresent(g->renderer);;
        SDL_Delay(16);
    }
}

/**
 * The program's entry point.
 */
int main()
{
    struct game *game = init_game();
    game_loop(game);
    free_game(game);
    return 0;
}
