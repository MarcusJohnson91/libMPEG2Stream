PACKAGE_NAME        := libMPEG2Stream
FILE                := $(CURDIR)/libMPEG2Stream/include/libMPEG2Stream.h
VERSION             := $(shell cat ${FILE} | grep -e "@version")
CC                  := cc
DESTINATION         := /usr/local/Packages/$(PACKAGE_NAME)
BUILD_DIR           := $(CURDIR)/BUILD
CFLAGS              := -std=c11 -march=native -funroll-loops -lmath `pkg-config --libs libBitIO`
LDFLAGS             := -flto=thin
DEB_ERROR_OPTIONS   := -Wno-unused-parameter -Wno-unused-variable -Wno-int-conversion
REL_ERROR_OPTIONS   := -Weverything -Wunreachable-code -Wno-conversion
DEB_FLAGS           := $(CFLAGS) -g -o0 $(DEB_ERROR_OPTIONS) $(LDFLAGS)
SANITIZER           := -fsanitize=undefined -fsanitize=address
REL_FLAGS           := $(CFLAGS) -ofast $(REL_ERROR_OPTIONS) $(LDFLAGS)
BUILD_LIB           := $(BUILD_DIR)/libMPEG2Stream

.PHONY: all check distclean CheckVer release debug test install clean uninstall

all: release
	$(release)
check: test
	$(test)
distclean: clean
	$(clean)
CheckVer:
	$(shell echo ${VERSION})
release: $(CURDIR)/libMPEG2Stream/src/DemuxMPEG2Stream.c
	mkdir -p   $(BUILD_DIR)
	mkdir -p   $(BUILD_LIB)
	$(CC)      $(REL_FLAGS) -c $(CURDIR)/libMPEG2Stream/src/DemuxMPEG2Stream.c -o $(BUILD_LIB)/DemuxMPEG2Stream.o
	$(CC)      $(REL_FLAGS) -c $(CURDIR)/libMPEG2Stream/src/MuxMPEG2Stream.c -o $(BUILD_LIB)/MuxMPEG2Stream.o
	ar -crsu   $(BUILD_LIB)/libMPEG2Stream.a $(BUILD_LIB)/*.o
	ranlib -sf $(BUILD_LIB)/libMPEG2Stream.a
	strip	   $(BUILD_LIB)/libMPEG2Stream.a
debug: $(CURDIR)/libMPEG2Stream/src/DemuxMPEG2Stream.c
	mkdir -p   $(BUILD_DIR)
	mkdir -p   $(BUILD_LIB)
	$(CC)      $(DEB_FLAGS) -c $(CURDIR)/libMPEG2Stream/src/DemuxMPEG2Stream.c -o $(BUILD_LIB)/DemuxMPEG2Stream.o
	$(CC)      $(DEB_FLAGS) -c $(CURDIR)/libMPEG2Stream/src/MuxMPEG2Stream.c -o $(BUILD_LIB)/MuxMPEG2Stream.o
	ar -crsu   $(BUILD_LIB)/libMPEG2Stream.a $(BUILD_LIB)/*.o
	ranlib -sf $(BUILD_LIB)/libMPEG2Stream.a
install:
	install -d -m 777 $(DESTINATION)/lib
	install -d -m 777 $(DESTINATION)/include
	install -C -v -m 444 $(BUILD_LIB)/libMPEG2Stream.a $(DESTINATION)/lib/libMPEG2Stream.a
	install -C -v -m 444 $(CURDIR)/libMPEG2Stream/include/libMPEG2Stream.h $(DESTINATION)/include/libMPEG2Stream.h
	install -C -v -m 444 (CURDIR)/libMPEG2Stream.pc /usr/share/pkgconfig/libMPEG2Stream.pc
clean:
	cd $(BUILD_LIB)/
	rm -f -v -r *.o
	rm -f -v -r libMPEG2Stream.a
	rm -f -v -r .DS_Store
	rm -f -v -r Thumbs.db
	rm -f -v -r desktop.ini
	rmdir $(BUILD_DIR)
uninstall:
	rm -d -i $(DESTINATION)
