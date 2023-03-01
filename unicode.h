#include <Arduino.h>

#ifndef PORTALCALENDAR_UTF8ITERATOR_H
#define PORTALCALENDAR_UTF8ITERATOR_H

/**
 * A very simple UTF8 iterator based on https://github.com/nemtrif/utfcpp
 *
 * Only handles code points in the basic multilingual plane, and does not validate fully against invalid sequences.
 */
class Utf8Iterator
{
public:
    Utf8Iterator(String str) 
    {
        _str = str;
        _pos = 0;
    }

    uint16_t next()
    {
        if (_pos >= _str.length()) {
            return 0;
        }

        uint8_t len = sequenceLength(_str[_pos]);
        uint16_t cp = _str[_pos];
        switch (len) {
            case 1:
                ++_pos;
                return cp;
            case 2:
                cp = ((cp << 6) & 0x7FF) + (_str[++_pos] & 0x3F);
                ++_pos;
                return cp;
            case 3:
                cp = ((cp << 12) & 0xFFFF) + ((_str[++_pos] << 6) & 0xFFF);
                cp += _str[++_pos] & 0x3F;
                ++_pos;
                return cp;
            case 4:
                // Code points over U+FFFF are not supported
                _pos += 4;
                return 0xFFFD;
            default:
                return 0;
        }
    }

private:
    size_t _pos;
    String _str;

    static uint8_t sequenceLength(uint8_t lead)
    {
        if (lead < 0x80) {
            return 1;
        } else if ((lead >> 5) == 0x6) {
            return 2;
        } else if ((lead >> 4) == 0xE) {
            return 3;
        } else if ((lead >> 3) == 0x1E) {
            return 4;
        } else {
            return 0;
        }
    }
};

bool isSpaceCodePoint(uint16_t cp) {
    return cp == 0x9 || cp == 0x20 || cp == 0xA0;
};

#endif // PORTALCALENDAR_UTF8ITERATOR_H
