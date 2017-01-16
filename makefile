PACKAGE_NAME        := libMPEGTS
FILE                := $(CURDIR)/libMPEGTS/include/libMPEGTS.h
VERSION             := $(shell cat ${FILE} | grep -e "@version")
CC                  := cc
DESTINATION         := /usr/local/Packages/$(PACKAGE_NAME)
BUILD_DIR           := $(CURDIR)/BUILD
CFLAGS              := -std=c11 -march=native
LDFLAGS             := -flto=thin
DEB_ERROR_OPTIONS   := -Wno-unused-parameter -Wno-unused-variable -Wno-int-conversion
REL_ERROR_OPTIONS   := -Weverything -Wunreachable-code -Wno-conversion
DEB_FLAGS           := $(CFLAGS) -g -o0 $(DEB_ERROR_OPTIONS) $(LDFLAGS)
SANITIZER           := -fsanitize=undefined -fsanitize=address
REL_FLAGS           := $(CFLAGS) -ofast $(REL_ERROR_OPTIONS) $(LDFLAGS)
BUILD_LIB           := $(BUILD_DIR)/libMPEGTS

.PHONY: all check distclean CheckVer release debug test install clean uninstall

all: release
	$(release)
check: test
	$(test)
distclean: clean
	$(clean)
CheckVer:
	$(shell echo ${VERSION})
release: $(CURDIR)/libMPEGTS/src/DemuxMPEGTS.c
	mkdir -p   $(BUILD_DIR)
	mkdir -p   $(BUILD_LIB)
	$(CC)      $(REL_FLAGS) -c $(CURDIR)/libMPEGTS/src/DemuxMPEGTS.c -o $(BUILD_LIB)/DemuxMPEGTS.o
	ar -crsu   $(BUILD_LIB)/libMPEGTS.a $(BUILD_LIB)/*.o
	ranlib -sf $(BUILD_LIB)/libMPEGTS.a
	strip	   $(BUILD_LIB)/libMPEGTS.a
debug: $(CURDIR)/libMPEGTS/src/DemuxMPEGTS.c
	mkdir -p   $(BUILD_DIR)
	mkdir -p   $(BUILD_LIB)
	$(CC)      $(DEB_FLAGS) -c $(CURDIR)/libMPEGTS/src/DemuxMPEGTS.c -o $(BUILD_LIB)/DemuxMPEGTS.o
	ar -crsu   $(BUILD_LIB)/libMPEGTS.a $(BUILD_LIB)/*.o
	ranlib -sf $(BUILD_LIB)/libMPEGTS.a
install:
	install -d -m 777 $(DESTINATION)/lib
	install -d -m 777 $(DESTINATION)/include
	install -C -v -m 444 $(BUILD_LIB)/libMPEGTS.a $(DESTINATION)/lib/libMPEGTS.a
	install -C -v -m 444 $(CURDIR)/libMPEGTS/include/libMPEGTS.h $(DESTINATION)/include/libMPEGTS.h
	install -C -v -m 444 (CURDIR)/libMPEGTS.pc /usr/share/pkgconfig/libMPEGTS.pc
clean:
	cd $(BUILD_LIB)/
	rm -f -v -r DemuxMPEGTS.o
	rm -f -v -r libMPEGTS.a
	rm -f -v -r .DS_Store
	rm -f -v -r Thumbs.db
	rm -f -v -r desktop.ini
	rmdir $(BUILD_DIR)
uninstall:
	rm -d -i $(DESTINATION)
