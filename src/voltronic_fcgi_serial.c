#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "voltronic_fcgi.h"
#include "voltronic_dev_serial.h"

voltronic_dev_t new_voltronic_dev(void) {
  const char* port_name = getenv("SERIAL_PORT_NAME");
  if (port_name != 0) {
    const voltronic_dev_t dev = voltronic_serial_create(
      port_name,
      2400,
      DATA_BITS_EIGHT,
      STOP_BITS_ONE,
      SERIAL_PARITY_NONE);

    if (dev != 0) {
      return dev;
    } else {
      const char* errno_prefix = "";
      const char* errno_str = "";
      if (errno > 0) {
        errno_prefix = "; ";
        errno_str = strerror(errno);
      }

      printf("Status: 500 Internal Server Error\r\n"
        "\r\n"
        "Could not open serial connection to '%s'%s%s",
        port_name, errno_prefix, errno_str);

      return 0;
    }
  } else {
    printf("Status: 500 Internal Server Error\r\n"
      "\r\n"
      "SERIAL_PORT_NAME fastcgi parameter not specified");

    return 0;
  }
}
