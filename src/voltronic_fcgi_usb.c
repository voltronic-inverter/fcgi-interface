#include <errno.h>
#include "utils.h"
#include "fcgi_adapter.h"
#include "voltronic_fcgi.h"
#include "voltronic_dev_usb.h"

voltronic_dev_t new_voltronic_dev(void) {
  const char* serial_number = fcgi_getenv("USB_SERIAL_NUMBER");

  reset_last_error();
  const voltronic_dev_t dev = voltronic_usb_create(
    0x0665,
    0x5161,
    serial_number);

  if (dev != 0) {
    return dev;
  } else {
    const char* serial_prefix = "";
    const char* serial_number_str = "";
    if (serial_number != 0) {
      serial_prefix = ", serial_number=";
      serial_number_str = serial_number;
    }

    errno_t errnum;
    const char* errno_prefix = "";
    const char* errno_str = "";
    if (get_last_error(&errnum)) {
      errno_prefix = "; ";
      errno_str = get_error_string(&errno);
    }

    fcgi_printf(
      "Status: 500 Internal Server Error\r\n"
      "\r\n"
      "Could not connect to any USB device with vendor_id=0x0665, "
      "product_id=0x5161%s%s%s%s",
      serial_prefix, serial_number_str, errno_prefix, errno_str);

    return 0;
  }
}
