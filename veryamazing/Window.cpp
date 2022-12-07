//
// Created by me on 99.99.2099.
//

#include <stdexcept>
#include <string>
#include <chrono>
#include <thread>
#include <cmath>
#include "Window.h"
#include <SDL2/SDL_image.h>

void Window::create_window() {
    _window = std::shared_ptr<SDL_Window>(
            SDL_CreateWindow("VERY AMAZING WINDOW", SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED, _width, _height,
                             SDL_WINDOW_OPENGL), SDL_DestroyWindow);
    if (_window == nullptr)
        throw std::runtime_error(
                std::string("Не могу создать окно: ") + std::string(SDL_GetError()));
}

void Window::create_gl_context() {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    _gl_context = std::shared_ptr<void>(SDL_GL_CreateContext(_window.get()),
                                        SDL_GL_DeleteContext);
    if(_gl_context == nullptr)
        throw std::runtime_error(
                std::string("Не могу создать контекст OpenGL: ") +
                std::string(SDL_GetError()));
}

void Window::set_perspective_mode() const {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FOV, double(_width) / double(_height), 0.1, 30.0);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
}

void Window::setup_gl() {
    glClearColor(0.1f, 0.2f, 0.35f, 1.0f);
    glEnable(GL_TEXTURE);
}

Window::Window(int w, int h): _width(w), _height(h) {
    create_window();
    create_gl_context();
    setup_gl();
    _wall_tex = std::make_shared<texture>("wall.png");
    _sky_tex = std::make_shared<texture>("ceiling.jpg");
    _road_tex = std::make_shared<texture>("grass.jpg");
    _map = std::make_shared<Map>("map01.txt");
    _player = std::make_shared<Player>();
    _player->spawn(_map);
}

void Window::main_loop() {
    bool want_quit = false;
    std::thread update_thread {
            [&]() {
                using clk = std::chrono::high_resolution_clock;
                auto delay = std::chrono::microseconds(1'000'000 / 60);
                auto next_time = clk::now() + delay;
                while (not want_quit) {
                    std::this_thread::sleep_until(next_time);
                    next_time += delay;
                    update();
                }
            }
    };

    SDL_Event e;
    for(;;) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                want_quit = true;
                update_thread.join();
                return;
            }
            event(e);
        }
        render();
        SDL_GL_SwapWindow(_window.get());
    }
}

void Window::set_ortho_mode() const {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, _width, _height, 0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
}

void Window::draw_minimap() {
    set_ortho_mode();
    glBegin(GL_QUADS);
    for (int y = 0; y < _map->height(); ++y)
        for (int x = 0; x < _map->width(); ++x) {
            GLdouble rect[8] {
                    GLdouble(MAP_OFFSET+(x+0)*CELL_SIZE),
                    GLdouble(MAP_OFFSET+(y+0)*CELL_SIZE),
                    GLdouble(MAP_OFFSET + (x+1) * CELL_SIZE),
                    GLdouble(MAP_OFFSET+(y+0)*CELL_SIZE),
                    GLdouble(MAP_OFFSET + (x+1) * CELL_SIZE),
                    GLdouble(MAP_OFFSET+(y+1)*CELL_SIZE),
                    GLdouble(MAP_OFFSET + (x+0) * CELL_SIZE),
                    GLdouble(MAP_OFFSET+(y+1)*CELL_SIZE)
            };
            if (_map->is_wall(x, y))
                glColor3d(1.0, 1.0, 1.0);
            else
                glColor3d(0.0, 0.0, 0.0);
            glVertex2dv(&rect[0]);
            glVertex2dv(&rect[2]);
            glVertex2dv(&rect[4]);
            glVertex2dv(&rect[6]);
        }
    glEnd();

    glPointSize(DOT_SIZE);
    int x1, y1, x2, y2;
    x1 = int(_player->x() * CELL_SIZE) + MAP_OFFSET;
    y1 = int(_player->y() * CELL_SIZE) + MAP_OFFSET;
    x2 = x1 + int(CELL_SIZE / 2.0 * cos(_player->dir()));
    y2 = y1 + int(CELL_SIZE / 2.0 * sin(_player->dir()));
    GLint vertices[4] {
            x1, y1,
            x2, y2
    };
    glBegin(GL_POINTS);
    glColor3d(0.25, 1.0, 0.25);
    glVertex2iv(&vertices[0]);
    glEnd();

    glBegin(GL_LINES);
    glColor3d(1.0, 1.0, 0.25);
    glVertex2iv(&vertices[0]);
    glVertex2iv(&vertices[2]);
    glEnd();
}

void Window::draw_hall(int my, int mx) {

    glEnable(GL_TEXTURE_2D);

    GLdouble vertices[24] {
            my + 0.0, mx + 0.0, 0.0,
            my + 1.0, mx + 0.0, 0.0,
            my + 1.0, mx + 1.0, 0.0,
            my + 0.0, mx + 1.0, 0.0,
            my + 0.0, mx + 0.0, WALL_HEIGHT,
            my + 1.0, mx + 0.0, WALL_HEIGHT,
            my + 1.0, mx + 1.0, WALL_HEIGHT,
            my + 0.0, mx + 1.0, WALL_HEIGHT };

    _road_tex->bind();
    glBegin(GL_QUADS);
    glColor3d(1.0, 1.0, 1.0);
    glTexCoord2d(0.0, 0.0); glVertex3dv(&vertices[0]);
    glTexCoord2d(0.0, 1.0); glVertex3dv(&vertices[3]);
    glTexCoord2d(1.0, 1.0); glVertex3dv(&vertices[6]);
    glTexCoord2d(1.0, 0.0); glVertex3dv(&vertices[9]);
    glEnd();

    _sky_tex->bind();
    glBegin(GL_QUADS);
    glColor3d(1.0, 1.0, 1.0);
    glTexCoord2d(0.0, 0.0); glVertex3dv(&vertices[12]);
    glTexCoord2d(0.0, 1.0); glVertex3dv(&vertices[15]);
    glTexCoord2d(1.0, 1.0); glVertex3dv(&vertices[18]);
    glTexCoord2d(1.0, 0.0); glVertex3dv(&vertices[21]);
    glEnd();
    glDisable(GL_TEXTURE_2D);

}

void Window::draw_wall(int my, int mx) {
    glEnable(GL_TEXTURE_2D);
    _wall_tex->bind();

    GLdouble vertices[24] {
            my + 0.0, mx + 0.0, 0.0,
            my + 1.0, mx + 0.0, 0.0,
            my + 1.0, mx + 1.0, 0.0,
            my + 0.0, mx + 1.0, 0.0,
            my + 0.0, mx + 0.0, WALL_HEIGHT,
            my + 1.0, mx + 0.0, WALL_HEIGHT,
            my + 1.0, mx + 1.0, WALL_HEIGHT,
            my + 0.0, mx + 1.0, WALL_HEIGHT };

    glBegin(GL_QUADS);
    glColor3d(1.0, 1.0, 1.0);
    glTexCoord2d(0.0, 1.0); glVertex3dv(&vertices[0*3]);
    glTexCoord2d(1.0, 1.0); glVertex3dv(&vertices[1*3]);
    glTexCoord2d(1.0, 0.0); glVertex3dv(&vertices[5*3]);
    glTexCoord2d(0.0, 0.0); glVertex3dv(&vertices[4*3]);
    glTexCoord2d(0.0, 1.0); glVertex3dv(&vertices[2*3]);
    glTexCoord2d(1.0, 1.0); glVertex3dv(&vertices[3*3]);
    glTexCoord2d(1.0, 0.0); glVertex3dv(&vertices[7*3]);
    glTexCoord2d(0.0, 0.0); glVertex3dv(&vertices[6*3]);
    glTexCoord2d(0.0, 1.0); glVertex3dv(&vertices[1*3]);
    glTexCoord2d(1.0, 1.0); glVertex3dv(&vertices[2*3]);
    glTexCoord2d(1.0, 0.0); glVertex3dv(&vertices[6*3]);
    glTexCoord2d(0.0, 0.0); glVertex3dv(&vertices[5*3]);
    glTexCoord2d(0.0, 1.0); glVertex3dv(&vertices[0*3]);
    glTexCoord2d(1.0, 1.0); glVertex3dv(&vertices[3*3]);
    glTexCoord2d(1.0, 0.0); glVertex3dv(&vertices[7*3]);
    glTexCoord2d(0.0, 0.0); glVertex3dv(&vertices[4*3]);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void Window::draw_view() {
    set_perspective_mode();
    glLoadIdentity();

    double px = _player->x();
    double py = _player->y();
    double pz = WALL_HEIGHT / 2.0;
    double cx = px + cos(_player->dir());
    double cy = py + sin(_player->dir());
    double cz= pz;

    gluLookAt(py, px, pz,
              cy, cx, cz,
              0.0, 0.0, 1.0);
    static double a = 0.0;
    a += 1.0;
    for (int mx = 0; mx < _map->width(); ++mx)
        for (int my = 0; my < _map->height(); ++my) {
            if (_map->is_wall(mx,my)) {
                draw_wall(my, mx);
            } else {
                draw_hall(my, mx);
            }
        }
}

void Window::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw_view();
    draw_minimap();
}

void Window::event(const SDL_Event &e) {
    switch (e.type) {
        case SDL_KEYDOWN:
            switch (e.key.keysym.scancode) {
                case SDL_SCANCODE_W: _controls.walk_fwd = true; break;
                case SDL_SCANCODE_S: _controls.walk_back = true; break;
                case SDL_SCANCODE_Q: _controls.turn_left = true; break;
                case SDL_SCANCODE_E: _controls.turn_right = true; break;
                case SDL_SCANCODE_A: _controls.shift_left = true; break;
                case SDL_SCANCODE_D: _controls.shift_right = true; break;
                default:
                    ;
            }
            break;
        case SDL_KEYUP:
            switch (e.key.keysym.scancode) {
                case SDL_SCANCODE_W: _controls.walk_fwd = false; break;
                case SDL_SCANCODE_S: _controls.walk_back = false; break;
                case SDL_SCANCODE_Q: _controls.turn_left = false; break;
                case SDL_SCANCODE_E: _controls.turn_right = false; break;
                case SDL_SCANCODE_A: _controls.shift_left = false; break;
                case SDL_SCANCODE_D: _controls.shift_right = false; break;
                default:
                    ;
            }
            break;
    }
}


void Window::update() {
    if (_controls.walk_fwd) _player->walk_fwd(WALK_DIST);
    if (_controls.walk_back) _player->walk_back(WALK_DIST);
    if (_controls.shift_left) _player->shift_left(WALK_DIST);
    if (_controls.shift_right) _player->shift_right(WALK_DIST);
    if (_controls.turn_left) _player->turn_left(TURN_ANGLE);
    if (_controls.turn_right) _player->turn_right(TURN_ANGLE);
}
