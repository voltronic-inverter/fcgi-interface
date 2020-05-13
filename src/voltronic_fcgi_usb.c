#include <errno.h>
#include "utils.h"
#include "fcgi_adapter.h"
#include "voltronic_fcgi.h"
#include "voltronic_dev_usb.h"

#define VOLTRONIC_VENDOR_ID  0x0665
#define VOLTRONIC_DEVICE_ID  0x5161

voltronic_dev_t new_voltronic_dev(void) {
  const voltronic_dev_t dev = voltronic_usb_create(
    VOLTRONIC_VENDOR_ID,
    VOLTRONIC_DEVICE_ID);

  if (dev != 0) {
    return dev;
  } else {
    const last_error_t last_error = GET_LAST_ERROR();
    const char* errno_prefix = "";
    const char* errno_str = "";
    if (last_error != 0) {
      errno_prefix = "; ";
      errno_str = GET_ERROR_STRING(last_error);
    }

    fcgi_printf(
      "Status: 500 Internal Server Error\r\n"
      "\r\n"
      "Could not connect to any USB device with vendor_id=0x0665, "
      "product_id=0x5161%s%s",
      errno_prefix, errno_str);

    return 0;
  }
}
