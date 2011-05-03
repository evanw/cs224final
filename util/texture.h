#ifndef TEXTURE_H
#define TEXTURE_H

class Texture
{
private:
    unsigned int id;
    int target;
    int width;
    int height;

public:
    Texture() : id(0), target(0), width(0), height(0) {}
    ~Texture();

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    void init(int target, int width, int height, int format, int internalFormat, int type, int wrap, int filter);
    void bind(int unit);
    void unbind(int unit);

    void startDrawingTo();
    void stopDrawingTo();
};

#endif // TEXTURE_H
