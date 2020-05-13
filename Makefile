# define the C compiler to use
CC = gcc
CP = cp -f

#directories
LDIR = lib
SDIR = src
ODIR = obj
VDIR = $(LDIR)/libvoltronic
VLDIR = $(LDIR)/libvoltronic/$(LDIR)
VSDIR = $(LDIR)/libvoltronic/$(SDIR)
VODIR = $(VDIR)/$(ODIR)

# define any compile-time flags
CFLAGS = -std=c99 -Werror -Wall -Wextra -Wpedantic -Wmissing-prototypes -Wshadow -O3 -flto -fomit-frame-pointer

# add includes
CFLAGS += -Iinclude -I$(VDIR)/include -I$(LDIR)/fcgi2/include -I$(VLDIR)/libserialport -I$(VLDIR)/hidapi/hidapi -I$(VLDIR)/libusb/libusb

# shared libraries
SHARED_LIBS = -lfcgi

# Object files shared by all directives
SHARED_OBJS = $(VODIR)/voltronic_crc.o $(VODIR)/voltronic_dev.o $(ODIR)/utils.o $(ODIR)/main.o $(ODIR)/fcgi_adapter.o $(ODIR)/voltronic_fcgi.o

# Directives
default:
	@echo "Different compile options exist using different underlying hardware and libraries to communicate with the hardware"
	@echo ""
	@echo "  libserialport - Serial port using libserialport"
	@echo "  hidapi - USB support using HIDApi in Mac, Windows, FreeBSD"
	@echo "  hidapi-hidraw - USB support in Linux using HIDApi utilizing HIDRaw"
	@echo "  hidapi-libusb - USB support using HIDApi utilizing LibUSB"
	@echo ""
	@echo "Usage: make libserialport; make hidapi; etc."

libserialport: $(SHARED_OBJS) $(ODIR)/voltronic_fcgi_serial.o $(VODIR)/voltronic_dev_serial_libserialport.o
	$(CC) -o $@ $^ $(CFLAGS) $(SHARED_LIBS) -lserialport
	$(CP) $@ voltronic_fcgi_libserialport
	$(RM) $@

hidapi: $(SHARED_OBJS) $(ODIR)/voltronic_fcgi_usb.o $(VODIR)/voltronic_dev_usb_hidapi.o
	$(CC) -o $@ $^ $(CFLAGS) $(SHARED_LIBS) -lhidapi
	$(CP) $@ voltronic_fcgi_hidapi
	$(RM) $@

hidapi-hidraw: $(SHARED_OBJS) $(ODIR)/voltronic_fcgi_usb.o $(VODIR)/voltronic_dev_usb_hidapi.o
	$(CC) -o $@ $^ $(CFLAGS) $(SHARED_LIBS) -lhidapi-hidraw
	$(CP) $@ voltronic_fcgi_hidapi_hidraw
	$(RM) $@

hidapi-libusb: $(SHARED_OBJS) $(ODIR)/voltronic_fcgi_usb.o $(VODIR)/voltronic_dev_usb_hidapi.o
	$(CC) -o $@ $^ $(CFLAGS) $(SHARED_LIBS) -lhidapi-libusb
	$(CP) $@ voltronic_fcgi_hidapi_libusb
	$(RM) $@

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(VODIR)/%.o: $(VSDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	$(RM) $(ODIR)/*.o $(VODIR)/*.o *~ voltronic_fcgi_libserialport voltronic_fcgi_hidapi voltronic_fcgi_hidapi_hidraw voltronic_fcgi_hidapi_libusb $(INCDIR)/*~ 
