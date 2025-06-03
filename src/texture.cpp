#include <iostream>
#include <cstdio>
#include "Angel.h"   
#include "texture.h"

GLuint loadPPMTexture(const char* filename, int& width, int& height) {
    FILE* fp;
    char buf[100];
    int max_val;

    fp = fopen(filename, "r");
    if (!fp) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        exit(EXIT_FAILURE);
    }

    fgets(buf, 100, fp);
    if (buf[0] != 'P' || buf[1] != '3') {
        std::cerr << filename << " is not a valid P3 PPM file!" << std::endl;
        exit(EXIT_FAILURE);
    }

    do {
        fgets(buf, 100, fp);
    } while (buf[0] == '#');

    sscanf(buf, "%d %d", &width, &height);
    fscanf(fp, "%d", &max_val);

    int size = width * height;
    GLubyte* image = new GLubyte[3 * size];
    for (int i = 0; i < size; ++i) {
        int r, g, b;
        fscanf(fp, "%d %d %d", &r, &g, &b);
        image[3 * i] = r;
        image[3 * i + 1] = g;
        image[3 * i + 2] = b;
    }
    fclose(fp);

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    delete[] image;
    return textureID;
}
