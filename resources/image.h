#ifndef PORTALCALENDAR_IMAGE_H
#define PORTALCALENDAR_IMAGE_H

struct Image {
    Image(uint16_t width, uint16_t height, uint8_t rleBits, const uint8_t *data):
        width(width),
        height(height),
        rleBits(rleBits),
        data(data)
    { }

    const uint16_t width;
    const uint16_t height;
    const uint8_t rleBits;
    const uint8_t *data;
};

class ImageReader
{
private:
    size_t byte;
    uint8_t run, color, crumb;
    const Image &image;

    uint8_t read()
    {
        uint8_t x = (image.data[byte] >> ((3 - crumb) * 2)) & 0b11;
        if (crumb == 3) {
            crumb = 0;
            ++byte;
        } else {
            ++crumb;
        }
        return x;
    }

public:
    ImageReader(const Image &image): image(image), byte(0), crumb(0), run(0)
    { }

    uint8_t next()
    {
        if (run == 0) {
            color = read();
            for (uint8_t i = image.rleBits; i >= 2; i -= 2) {
                run |= read() << (i - 2);
            }
        } else {
            --run;
        }
        return color;
    }
};

#endif // PORTALCALENDAR_IMAGE_H
