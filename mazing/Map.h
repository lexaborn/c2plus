//
// Created by me on 99.99.2099.
//

#ifndef GAME_MAP_H
#define GAME_MAP_H

#include <vector>
#include <string>

class Map final {
private:
    int _width {}, _height {};
    double _start_x {}, _start_y {}, _start_dir {};
    std::vector<std::string> _data;

public:
    Map(const char * filename);

    int width() const { return _width; }
    int height() const { return _height; }
    double start_x() const { return _start_x; }
    double start_y() const { return _start_y; }
    double start_dir() const { return _start_dir; }

    bool is_wall(int x, int y) const;
    bool is_wall(double x, double y) const;

    void next_line(std::ifstream & file, std::string & line);
};

#endif
