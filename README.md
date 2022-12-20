# Pong
This is a Pong game created in C99 using the SDL2 game library.

Like the original game...
- The paddle is divided into eighths allowing for returning the ball at various angles
- As the round goes on the ball moves faster
- The paddles cannot reach the ends of the screen

TODO: Add scoring and a reset button.

## Requirements
SDL2 needs to be linked to build the game. A simple way to install SDL2 on a Mac is via Homebrew
```shell
brew install sdl2
```

## Building and Running
```shell
make
./a.out
```
