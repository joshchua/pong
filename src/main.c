#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#define LOG_CAT_GENERAL 0

/**
 * The root game structure.
 */
struct game {
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool is_running;
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
    int w = 500;
    int h = 300;
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
        SDL_LogCritical(LOG_CAT_GENERAL, "Failed to create renderer");
        exit(1);
    }

    struct game *game = (struct game *)malloc(sizeof(struct game));
    game->window = window;
    game->renderer = renderer;
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
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            game->is_running = false;
            break;
        default:
            break;
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
    SDL_SetRenderDrawColor(game->renderer, 96, 128, 255, 255);
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
