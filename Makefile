# define the C compiler to use
CC = gcc
CP = cp -f

#directories
IDIR = include
LDIR = lib
SDIR = src
ODIR = obj

# define any compile-time flags
CFLAGS = -std=c99 -Werror -Wall -Wextra -Wpedantic -Wmissing-prototypes -Wshadow -O3 -flto -fomit-frame-pointer

# add includes
CFLAGS += -I$(IDIR) -I$(LDIR)/fcgi2/include -I$(LDIR)/libserialport -I$(LDIR)/hidapi/hidapi

# shared libraries
SHARED_LIBS = -lfcgi

# define the C source files
SRCS = $(wildcard $(SDIR)/*.c)

# Object files shared by all directives
SHARED_OBJS = $(ODIR)/main.o $(ODIR)/time_util.o $(ODIR)/voltronic_crc.o $(ODIR)/voltronic_dev.o  $(ODIR)/voltronic_fcgi.o

# Directives
default:
	@echo "Different compile options exist; ie. make libserialport; make hidapi; etc."
	@echo "  libserialport - Serial port using libserialport"
	@echo "  hidapi - USB support using HIDApi in Mac, Windows, FreeBSD"
	@echo "  hidapi-hidraw - USB support in Linux using HIDApi utilizing HIDRaw"
	@echo "  hidapi-libusb - USB support using HIDApi utilizing LibUSB"

libserialport: $(SHARED_OBJS) $(ODIR)/voltronic_dev_serial_libserialport.o $(ODIR)/voltronic_fcgi_libserialport.o
	$(CC) -o $@ $^ $(CFLAGS) $(SHARED_LIBS) -lserialport
	$(CP) $@ voltronic_fcgi_libserialport
	$(RM) $@

hidapi: $(SHARED_OBJS) $(ODIR)/voltronic_dev_usb_hidapi.o $(ODIR)/voltronic_fcgi_hidapi.o
	$(CC) -o $@ $^ $(CFLAGS) $(SHARED_LIBS) -lhidapi
	$(CP) $@ voltronic_fcgi_hidapi
	$(RM) $@

hidapi-hidraw: $(SHARED_OBJS) $(ODIR)/voltronic_dev_usb_hidapi.o $(ODIR)/voltronic_fcgi_hidapi_hidraw.o
	$(CC) -o $@ $^ $(CFLAGS) $(SHARED_LIBS) -lhidapi-hidraw
	$(CP) $@ voltronic_fcgi_hidapi_hidraw
	$(RM) $@

hidapi-libusb: $(SHARED_OBJS) $(ODIR)/voltronic_dev_usb_hidapi.o $(ODIR)/voltronic_fcgi_hidapi_libusb.o
	$(CC) -o $@ $^ $(CFLAGS) $(SHARED_LIBS) -lhidapi-libusb
	$(CP) $@ voltronic_fcgi_hidapi_libusb
	$(RM) $@

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	$(RM) $(ODIR)/*.o *~ core voltronic_fcgi_libserialport voltronic_fcgi_hidapi voltronic_fcgi_hidapi_hidraw voltronic_fcgi_hidapi_libusb $(INCDIR)/*~ 
