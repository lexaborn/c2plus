//
// Created by me on 99.99.2099.
//

#ifndef VERY_AMAZING_TEXTURE_H
#define VERY_AMAZING_TEXTURE_H

#include <GL/gl.h>
#include <GL/glu.h>

class texture final{
private:
    GLuint _tex {};
public:
    texture(const char * filename);
    ~texture();

    void bind() {glBindTexture(GL_TEXTURE_2D, _tex);}

};

#endif
