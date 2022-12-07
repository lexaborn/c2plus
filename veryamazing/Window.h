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
#include "texture.h"

class Window final {
private:
    static constexpr int CELL_SIZE = 15;
    static constexpr int MAP_OFFSET = 5;
    static constexpr int DOT_SIZE = 3;
    static constexpr double WALK_DIST = 0.03;
    static constexpr double TURN_ANGLE = 0.05;
    static constexpr double WALL_HEIGHT = 0.5;
    static constexpr double FOV = 45.0;

    std::shared_ptr<Map> _map;
    std::shared_ptr<Player> _player;
    std::shared_ptr<texture> _wall_tex;
    std::shared_ptr<texture> _road_tex;
    std::shared_ptr<texture> _sky_tex;

    struct {
        bool turn_left {false};
        bool turn_right {false};
        bool walk_fwd {false};
        bool walk_back {false};
        bool shift_left {false};
        bool shift_right {false};
    } _controls;

private:
    int _width, _height;
    std::shared_ptr<SDL_Window> _window;
    std::shared_ptr<void> _gl_context;

    void create_window();
    void create_gl_context();
    static void setup_gl();

    void draw_minimap();
    void set_perspective_mode() const;
    void set_ortho_mode() const;
    void draw_view();
    void draw_hall(int my, int mx);
    void draw_wall(int my, int mx);

public:
    Window(int w, int h);

    void main_loop();
    void render();
    void event(const SDL_Event & e);
    void update();
};


#endif
