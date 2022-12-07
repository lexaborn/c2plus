//
// Created by me on 99.99.2099.
//

#include <fstream>
#include <stdexcept>
#include <sstream>
#include <cmath>
#include "Map.h"

void Map::next_line(std::ifstream & file, std::string & line) {
    do {
        if (not std::getline(file, line))
            throw std::runtime_error("There are not enough lines in the map description file!");
    } while (line.length() == 0 || line[0] == ';');
}

Map::Map(const char *filename) {
    std::ifstream file { filename };

    std::string line;

    next_line(file, line);
    std::stringstream(line) >> _width >> _height >> _start_x >> _start_y >> _start_dir;

    _data.resize(_height);
    for (auto && l: _data)
        next_line(file, l);

    file.close();
}

bool Map::is_wall(int x, int y) const {
    if (x < 0 or y < 0 or x >= _width or y >= _height)
        return true;
    return _data[y][x] != ' ';
}

bool Map::is_wall(double x, double y) const {
    return is_wall(int(floor(x)), int(floor(y)));
}
