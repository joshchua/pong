#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#define LOG_CAT_GENERAL 0

#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 300

#define BALL_LENGTH 10
#define PADDLE_WIDTH 10
#define PADDLE_HEIGHT 40
#define PADDLE_DY 5
#define PADDLE_HORIZONTAL_OFFSET 20

/**
 * The root game structure.
 */
struct game
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool is_running;

    SDL_Rect ball;
    int ball_dy;
    int ball_dx;
    
    SDL_Rect paddle1;
    int paddle1_dy;
    
    SDL_Rect paddle2;
    int paddle2_dy;
};

/**
 * Creates and configures a window.
 *
 * @return The window handle.
 */
SDL_Window *create_window()
{
    const char *title = "Pong";
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
    game->ball.x = 100;
    game->ball.y = 100;

    game->ball_dx = -1;
    game->ball_dy = -1;

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
void free_game(struct game *game)
{
    SDL_DestroyRenderer(game->renderer);
    SDL_DestroyWindow(game->window);
    SDL_Quit();
    free(game);
}

/**
 * Handler for game input.
 *
 * @param [in] game The game structure.
 */
void handle_input(struct game *game)
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            game->is_running = false;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
            case SDLK_w:
                game->paddle1_dy = -PADDLE_DY;
                break;
            case SDLK_s:
                game->paddle1_dy = +PADDLE_DY;
                break;
            case SDLK_UP:
                game->paddle2_dy = -PADDLE_DY;
                break;
            case SDLK_DOWN:
                game->paddle2_dy = +PADDLE_DY;
                break;
            default:
                break;
            }
        } else if (e.type == SDL_KEYUP) {
            switch (e.key.keysym.sym) {
            case SDLK_w:
                game->paddle1_dy = 0;
                break;
            case SDLK_s:
                game->paddle1_dy = 0;
                break;
            case SDLK_UP:
                game->paddle2_dy = 0;
                break;
            case SDLK_DOWN:
                game->paddle2_dy = 0;
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
void prepare_scene(struct game *game)
{
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderClear(game->renderer);
}

/**
 * Presents the scene for the frame.
 *
 * @param [in] game The game structure.
 */
void present_scene(struct game *game)
{
    SDL_RenderPresent(game->renderer);
}

void move_ball(struct game *game)
{
    game->ball.x += game->ball_dx;
    game->ball.y += game->ball_dy;

    int window_width;
    int window_height;
    SDL_GL_GetDrawableSize(game->window, &window_width,
                           &window_height);

    if (game->ball.x < 0 ||
        game->ball.x + game->ball.w > window_width) {
        game->ball.x = (window_width / 2) - (game->ball.w / 2);
        game->ball.y = (window_height / 2) - (game->ball.h / 2);
    }

    if (game->ball.y < 0) {
        game->ball_dy = +1;
    } else if (game->ball.y + game->ball.h > window_height) {
        game->ball_dy = -1;
    }
}

void move_paddle(struct game *game, SDL_Rect *paddle, int *dy)
{
    int window_height;
    SDL_GL_GetDrawableSize(game->window, NULL, &window_height);

    paddle->y += *dy;

    if (paddle->y < 0) {
        paddle->y = 0;
    } else if (paddle->y + paddle->h > window_height) {
        paddle->y = window_height - paddle->h;
    }
}

void handle_paddle_ball_collision(struct game *game, SDL_Rect *p,
                                  int *p_dy)
{
    SDL_Rect *b = &game->ball;
    if (p->x < b->x + b->w &&
        p->x + p->w > b->x &&
        p->y < b->y + b->h &&
        p->y + p->h > b->y) {
        game->ball_dx *= -1;
        int dy = 0;
        if (*p_dy < 0) {
            dy = -1;
        } else if (*p_dy > 0) {
            dy = 1;
        }
        game->ball_dy = dy;
    }
}

void draw_white_rect(SDL_Renderer *renderer, SDL_Rect *rect)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, rect);
}

/**
 * The game's main loop.
 *
 * @param [in] game The game structure.
 */
void game_loop(struct game *game)
{
    game->is_running = true;
    while (game->is_running) {
        prepare_scene(game);
        handle_input(game);
        move_ball(game);
        move_paddle(game, &game->paddle1, &game->paddle1_dy);
        move_paddle(game, &game->paddle2, &game->paddle2_dy);
        handle_paddle_ball_collision(game, &game->paddle1,
                                     &game->paddle1_dy);
        handle_paddle_ball_collision(game, &game->paddle2,
                                     &game->paddle2_dy);
        draw_white_rect(game->renderer, &game->ball);
        draw_white_rect(game->renderer, &game->paddle1);
        draw_white_rect(game->renderer, &game->paddle2);
        present_scene(game);
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
