# Source setenv.sh before running make
#

CFLAGS := -I$(MARVELL_ROOTFS)/usr/include/SDL2
LDFLAGS := -lSDL2 -lSDL2_ttf

testspriteminimal: testspriteminimal.o
	$(CC) -o $@ $< $(LDFLAGS)

clean:
	$(RM) testspriteminimal.o

distclean: clean
	$(RM) testspriteminimal
	$(RM) -r steamlink
