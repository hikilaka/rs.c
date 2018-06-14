#ifndef GAME_H
#define GAME_H

#pragma once

#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

enum game_mouse_state {
    GAME_MOUSE_LEFT = 1 << 0,
    GAME_MOUSE_RIGHT = 1 << 8,
    GAME_MOUSE_WHEEL = 1 << 16
};

enum game_state {
    GAME_STATE_IDLE,
    GAME_STATE_RUNNING,
    GAME_STATE_CLOSING,
};

struct game_input {
    char *buffer;
    char *entered;
    size_t buffer_sz;
    size_t entered_sz;
    size_t max_sz;
};

struct game {
    SDL_Window *window;
    size_t width;
    size_t height;

    int state;
    uint8_t period;
    uint16_t fps;
    bool reduce_lag;

    struct game_input input_short;
    struct game_input input_long;

    size_t mouse_x;
    size_t mouse_y;
    size_t mouse_click;
};

void game_init(struct game *game, const char *title, size_t width,
               size_t height);

void game_deinit(struct game *game);

void game_start(struct game *game);

void game_stop(struct game *game);

void game_set_rate(struct game *game, uint8_t rate);

void game_load_resources(struct game *game);

void game_release_resources(struct game *game);

void game_update(struct game *game);

void game_render(struct game *game);

#endif // GAME_H
