//
// Created by me on 99.99.2099.
//

#ifndef GAME_WINDOW_H
#define GAME_WINDOW_H

#include <SDL2/SDL.h>
#include <memory>
#include <utility>
#include "Map.h"
#include "Player.h"

class Window final {
private:
    static constexpr int CELL_SIZE = 15;
    static constexpr int MAP_OFFSET = 5;
    static constexpr int DOT_SIZE = 3;
    static constexpr double WALK_DIST = 0.03;
    static constexpr double TURN_ANGLE = 0.05;
    static constexpr double WALL_HEIGHT = 0.5;
    static constexpr double FOV = 90.0;

    std::shared_ptr<Map> _map;
    std::shared_ptr<Player> _player;

    struct {
        bool turn_left { false };
        bool turn_right { false };
        bool walk_fwd { false };
        bool walk_back { false };
        bool shift_left { false };
        bool shift_right { false };
    } _controls;

private:
    int _width, _height;
    std::shared_ptr<SDL_Window> _window;
    std::shared_ptr<SDL_Renderer> _renderer;
    std::shared_ptr<SDL_Texture> _wall_tex;
    int _wall_tex_width, _wall_tex_height;

    void create_window();
    void create_renderer();
    void draw_minimap(SDL_Renderer *r) const;
    void cast_ray(double & rx, double & ry, double dx, double dy);

    std::tuple<std::shared_ptr<SDL_Texture>, int, int>
    load_texture(const char *filename);
public:
    Window(int w, int h);

    void main_loop();
    void render(SDL_Renderer * r);
    void event(const SDL_Event & e);
    void update();
};


#endif
