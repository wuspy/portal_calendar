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
    Utf8Iterator(String str);

    uint16_t next();
    inline unsigned int getCurrentPosition() const { return _pos; };

    static bool isSpaceCodePoint(uint16_t cp);
    static bool isNewlineCodePoint(uint16_t cp);

private:
    unsigned int _pos;
    String _str;

    static uint8_t sequenceLength(uint8_t lead);
};

#endif // PORTALCALENDAR_UTF8ITERATOR_H
