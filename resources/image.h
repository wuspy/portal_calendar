#ifndef IMAGE_H
#define IMAGE_H

typedef struct {
    const uint16_t width;
    const uint16_t height;
    const uint8_t *data;
} Image;

#endif // IMAGE_H
