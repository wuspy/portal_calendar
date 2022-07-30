#ifndef IMAGE_H
#define IMAGE_H

typedef struct Image
{
    const uint16_t width;
    const uint16_t height;
    const uint8_t *data;
};

#endif // IMAGE_H
