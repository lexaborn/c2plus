//
// Created by me on 99.99.2099.
//

#include <stdexcept>
#include <string>
#include <chrono>
#include <thread>
#include <SDL2/SDL_image.h>
#include "Window.h"

static constexpr double Pi = acos(-1.0);

Window::Window(int w, int h): _width(w), _height(h) {

    create_window();
    create_renderer();

    auto [t, tw, th] = load_texture("wall-e.png");
    _wall_tex = t;
    _wall_tex_width = tw;
    _wall_tex_height = th;

    _map = std::make_shared<Map>("map01.txt");
    _player = std::make_shared<Player>();
    _player -> spawn(_map);
}

std::tuple<std::shared_ptr<SDL_Texture>, int, int>
Window::load_texture(const char *filename) {

    std::shared_ptr<SDL_Texture> result;
    int w, h;
    result = std::shared_ptr<SDL_Texture>(
            IMG_LoadTexture(_renderer.get(), filename),
            SDL_DestroyTexture);
    if (result == nullptr)
        throw std::runtime_error(
                std::string("Не могу загрузить текстуру: ") +
                std::string(SDL_GetError()));
    SDL_QueryTexture(result.get(), nullptr, nullptr, &w, &h);
    return std::make_tuple(result, w, h);
}

void Window::create_renderer() {
    _renderer = std::shared_ptr<SDL_Renderer>(
            SDL_CreateRenderer(_window.get(), -1,
                               SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
            SDL_DestroyRenderer);
    if (_renderer == nullptr)
        throw std::runtime_error(
                std::string("Не могу создать рендер: ") +
                std::string(SDL_GetError()));
}

void Window::create_window() {
    _window = std::shared_ptr<SDL_Window>(
            SDL_CreateWindow(
                    "AMAZING AMAZING",
                    SDL_WINDOWPOS_CENTERED,
                    SDL_WINDOWPOS_CENTERED,
                    _width, _height, 0),
            SDL_DestroyWindow);
    if (_window == nullptr)
        throw std::runtime_error(
                std::string("Не могу создать окно: ") +
                std::string(SDL_GetError()));
}

void Window::main_loop() {

    bool want_quit = false;

    std::thread update_thread {
        [&]() {
            using clk = std::chrono::high_resolution_clock;
            auto delay = std::chrono::microseconds(1'000'000 / 60);
            auto next_time = clk::now() + delay;

            while (not want_quit){
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

        render(_renderer.get());
        SDL_RenderPresent(_renderer.get());
    }

}

void Window::render(SDL_Renderer *r) {

    SDL_Rect cr {0, 0, _width, _height/2};
    SDL_Rect fr {0, _height/2, _width, _height/2};
    SDL_SetRenderDrawColor(r, 127, 127, 127, 255);
    SDL_RenderFillRect(r, &cr);
    SDL_SetRenderDrawColor(r, 7, 75, 22, 255);
    SDL_RenderFillRect(r, &fr);
    SDL_SetRenderDrawColor(r, 50, 54, 150, 255);

    double fov = FOV * Pi / 180.0;
    double ds = _width / 2.0 / tan(fov/2);
    double px = _player -> x();
    double py = _player -> y();
    double alpha = _player -> dir();
    double eps = 0.00001;
    for (int col = 0; col < _width; ++col) {
        double gamma = atan2(col - _width/2, ds);
        double beta = alpha + gamma;
        double rx, ry, dx, dy;
        int h;
        double d, dv, dh;
        double txv, txh, tx;

//        bool w;

        if (sin(beta) > eps) {
            dy = 1.0;
            dx = 1.0 / tan(beta);
            ry = floor(py) + eps;
            rx = px - (py - ry) * dx;
            cast_ray(rx, ry, dx, dy);
            dh = hypot(rx-px, ry-py);
            txh = 1.0 - (rx - floor(rx));
        } else if (sin(beta) < -eps) {
            dy = -1.0;
            dx = 1.0 / tan(-beta);
            ry = ceil(py) - eps;
            rx = px - (ry - py) * dx;
            cast_ray(rx, ry, dx, dy);
            dh = hypot(rx-px, ry-py);
            txh = rx - floor(rx);
        } else {
            dh = INFINITY;
        }

        if (cos(beta) > eps) {
            dx = 1.0;
            dy = tan(beta);
            rx = floor(px) + eps;
            ry = py - (px - rx) * dy;
            cast_ray(rx, ry, dx, dy);
            dv = hypot(rx-px, ry-py);
            txv = ry - floor(ry);
        } else if (cos(beta) < -eps) {
            dx = -1.0;
            dy = tan(-beta);
            rx = ceil(px) - eps;
            ry = py - (rx - px) * dy;
            cast_ray(rx, ry, dx, dy);
            dv = hypot(rx-px, ry-py);
            txv = 1.0 - (ry - floor(ry));
        } else {
            dv = INFINITY;
        }

        if (dv < dh) {
            d = dv;
            tx = txv;
        } else {
            d = dh;
            tx = txh;
        }

        h = WALL_HEIGHT * ds / d / cos(gamma);

        SDL_Rect src {
            int(floor(_wall_tex_width * tx)), 0,
            1, _wall_tex_height };
        SDL_Rect dst {
            col, _height/2 - h/2, 1, h };
        SDL_RenderCopy(r, _wall_tex.get(), &src, &dst);
    }

    draw_minimap(r);
}

void Window::draw_minimap(SDL_Renderer *r) const {
    for (int y = 0; y < _map-> height(); ++y)
        for (int x = 0; x < _map-> width(); ++x) {
            SDL_Rect cr {
                MAP_OFFSET + x * CELL_SIZE,
                MAP_OFFSET + y * CELL_SIZE,
                CELL_SIZE, CELL_SIZE };
            if (_map-> is_wall(x, y))
                SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
            else
                SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
            SDL_RenderFillRect(r, &cr);
        }
    int x1, y1, x2, y2;
    x1 = int(_player-> x() * CELL_SIZE) + MAP_OFFSET;
    y1 = int(_player-> y() * CELL_SIZE) + MAP_OFFSET;
    x2 = x1 + int(CELL_SIZE/2.0 * cos(_player-> dir()));
    y2 = y1 + int(CELL_SIZE/2.0 * sin(_player-> dir()));
    SDL_Rect pr {
        x1 - DOT_SIZE/2,
        y1 - DOT_SIZE/2,
        DOT_SIZE, DOT_SIZE };
    SDL_SetRenderDrawColor(r, 64, 255, 64, 255);
    SDL_RenderFillRect(r, &pr);
    SDL_SetRenderDrawColor(r, 255, 255, 64, 255);
    SDL_RenderDrawLine(r, x1, y1, x2, y2);
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
        default:
            ;
    }

}


void Window::cast_ray(double &rx, double &ry, double dx, double dy) {
    do {
        rx += dx;
        ry += dy;
    } while (not _map -> is_wall(rx, ry));
}

void Window::update() {

    if (_controls.walk_fwd) _player -> walk_fwd(WALK_DIST);
    if (_controls.walk_back) _player -> walk_back(WALK_DIST);
    if (_controls.shift_left) _player -> shift_left(WALK_DIST);
    if (_controls.shift_right) _player -> shift_right(WALK_DIST);
    if (_controls.turn_left) _player -> turn_left(TURN_ANGLE);
    if (_controls.turn_right) _player -> turn_right(TURN_ANGLE);

}
