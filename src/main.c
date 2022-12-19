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
#define BALL_SPEED 3
#define BALL_ANGLES { 0, 30, 45, 60, 300, 315, 330, \
                      120, 135, 150, 180, 210, 225, 240 }
#define BALL_ANGLES_SIZE 14

#define PADDLE_WIDTH 10
#define PADDLE_HEIGHT 40
#define PADDLE_DY 5
#define PADDLE_HORIZONTAL_OFFSET 0
#define PADDLE_VERTICAL_OFFSET 20

/**
 * The root game structure.
 */
struct game
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool is_running;

    SDL_FRect ball;
    float ball_dy;
    float ball_dx;
    
    SDL_FRect paddle1;
    float paddle1_dy;
    
    SDL_FRect paddle2;
    float paddle2_dy;
};

/**
 * Creates and configures a window.
 *
 * @return The window handle.
 */
SDL_Window *create_window()
{
    const char *title = WINDOW_TITLE;
    int x = SDL_WINDOWPOS_CENTERED;
    int y = SDL_WINDOWPOS_CENTERED;
    int w = WINDOW_WIDTH;
    int h = WINDOW_HEIGHT;
    uint32_t flags = 0;
    return SDL_CreateWindow(title, x, y, w, h, flags);
}

/**
 * Creates and configures a renderer.
 *
 * @param [in] window The window to render to.
 *
 * @return The renderer handle.
 */
SDL_Renderer *create_renderer(SDL_Window *window)
{
    int index = -1;
    uint32_t flags = 0;
    return SDL_CreateRenderer(window, index, flags);
}


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
}

/**
 * Initializes the game, including SDL2 and game state.
 *
 * @return A pointer to the game structure.
 */
struct game *init_game()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogCritical(LOG_CAT_GENERAL, "Failed to initialize SDL2.");
        exit(1);
    }
    
    SDL_Window *window = create_window();
    if (!window) {
        SDL_LogCritical(LOG_CAT_GENERAL, "Failed to create window.");
        exit(1);
    }

    SDL_Renderer *renderer = create_renderer(window);
    if (!renderer) {
        SDL_LogCritical(LOG_CAT_GENERAL, "Failed to create renderer.");
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
 * Deinitializes the game structure.
 *
 * @param [in] game The game structure.
 */
void free_game(struct game *g)
{
    SDL_DestroyRenderer(g->renderer);
    SDL_DestroyWindow(g->window);
    SDL_Quit();
    free(g);
}

/**
 * Handler for game input.
 *
 * @param [in] game The game structure.
 */
void handle_input(struct game *g)
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            g->is_running = false;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
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
        } else if (e.type == SDL_KEYUP) {
            switch (e.key.keysym.sym) {
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
    }        
}

/**
 * Prepares the scene for the frame.
 *
 * @param [in] game The game structure.
 */
void prepare_scene(struct game *g)
{
    SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, 255);
    SDL_RenderClear(g->renderer);
}

/**
 * Presents the scene for the frame.
 *
 * @param [in] game The game structure.
 */
void present_scene(struct game *g)
{
    SDL_RenderPresent(g->renderer);
}


void move_ball(struct game *g)
{
    g->ball.x += g->ball_dx * BALL_SPEED;
    g->ball.y += g->ball_dy * BALL_SPEED;

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
}

void move_paddle(struct game *g, SDL_FRect *paddle, float *dy)
{
    int window_height;
    SDL_GL_GetDrawableSize(g->window, NULL, &window_height);

    paddle->y += *dy;

    if (paddle->y < PADDLE_VERTICAL_OFFSET) {
        paddle->y = PADDLE_VERTICAL_OFFSET;
    } else if (paddle->y + paddle->h >
               window_height - PADDLE_VERTICAL_OFFSET) {
        paddle->y = window_height - paddle->h - PADDLE_VERTICAL_OFFSET;
    }
}

void draw_white_rect(SDL_Renderer *renderer, SDL_FRect *rect)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRectF(renderer, rect);
}

void draw_ball(struct game *g)
{
    SDL_SetRenderDrawColor(g->renderer, 255, 255, 255, 255);
    SDL_RenderFillRectF(g->renderer, &g->ball);
}

/**
 * The game's main loop.
 *
 * @param [in] game The game structure.
 */
void game_loop(struct game *g)
{
    g->is_running = true;
    while (g->is_running) {
        prepare_scene(g);
        handle_input(g);
        move_paddle(g, &g->paddle1, &g->paddle1_dy);
        move_paddle(g, &g->paddle2, &g->paddle2_dy);
        move_ball(g);        
        draw_ball(g);
        draw_white_rect(g->renderer, &g->paddle1);
        draw_white_rect(g->renderer, &g->paddle2);
        present_scene(g);
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
