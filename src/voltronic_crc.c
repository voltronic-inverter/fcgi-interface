#include "voltronic_crc.h"

#define IS_SET(_ch_, _bits_) \
  (((_ch_) & (_bits_)) == (_bits_))

#define IS_RESERVED_CHARACTER(_ch_) \
  (IS_SET(_ch_, 0x28) || IS_SET(_ch_, 0x0d) || IS_SET(_ch_, 0x0a))

static const voltronic_crc_t xmodem_crc_table[16] = {
    0x0000, 0x1021, 0x2042, 0x3063,
    0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b,
    0xc18c, 0xd1ad, 0xe1ce, 0xf1ef
};

static inline int _write_voltronic_crc(
    const voltronic_crc_t crc,
    unsigned char* buffer,
    const size_t buffer_length) {

  if (buffer_length >= sizeof(voltronic_crc_t)) {
    buffer[0] = (unsigned char) crc;
    buffer[1] = (unsigned char) (crc >> 8);
    return 1;
  } else {
    return 0;
  }
}

static inline voltronic_crc_t _read_voltronic_crc(
    const unsigned char* buffer,
    const size_t buffer_length) {

  if (buffer_length >= sizeof(voltronic_crc_t)) {
    voltronic_crc_t crc = 0;

    crc |= (voltronic_crc_t) buffer[0];
    crc = crc << 8;
    crc |= (voltronic_crc_t) buffer[1];

    return crc;
  } else {
    return 0;
  }
}

static inline voltronic_crc_t _calculate_voltronic_crc(
    const unsigned char* buffer,
    size_t buffer_length) {

  voltronic_crc_t crc = 0;

  if (buffer_length > 0) {
    do {
      const unsigned char b = *buffer;

      crc = xmodem_crc_table[(crc >> 12) ^ (b >> 4)] ^ (crc << 4);
      crc = xmodem_crc_table[(crc >> 12) ^ (b & 0x0f)] ^ (crc << 4);

      buffer += sizeof(unsigned char);
    } while(--buffer_length);

    if (IS_RESERVED_CHARACTER(crc)) {
      crc |= 1;
    }

    if (IS_RESERVED_CHARACTER(crc >> 8)) {
      crc |= 1 << 8;
    }
  }

  return crc;
}

int write_voltronic_crc(
    const voltronic_crc_t crc,
    char* buffer,
    const size_t buffer_length) {

  return _write_voltronic_crc(
    crc, (unsigned char*) buffer, buffer_length);
}

voltronic_crc_t read_voltronic_crc(
    const char* buffer,
    const size_t buffer_length) {

  return _read_voltronic_crc(
    (const unsigned char*) buffer, buffer_length);
}

voltronic_crc_t calculate_voltronic_crc(
    const char* buffer,
    size_t buffer_length) {

  return _calculate_voltronic_crc(
    (const unsigned char*) buffer, buffer_length);
}
