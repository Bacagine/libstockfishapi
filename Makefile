# Makefile of project stockfish_api
#
# Written by Gustavo Bacagine <gustavo.bacagine@protonmail.com> in Jul 2025
#
#

CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -shared
LDLIBS =
LIBRARY = libstockfishapi.so
SRCDIR = src
INCDIR = include
OBJDIR = obj

LIBDIR = lib

SRCFILES = $(SRCDIR)/stockfish_api.c
OBJFILES = $(SRCFILES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

ifdef LINUX
	CFLAGS += -DLINUX
endif

all: $(OBJDIR) $(LIBDIR) $(LIBRARY)

$(OBJDIR):
	mkdir obj

$(LIBDIR):
	mkdir lib

$(LIBRARY): $(OBJFILES)
	$(CC) $(LDFLAGS) $(LDLIBS) -o $(LIBDIR)/$@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	rm -rf $(OBJDIR)/*.o $(LIBDIR)/$(LIBRARY)

distclean:
	rm -rf $(OBJDIR) $(LIBDIR)
	rm -rf a.out
	rm -rf test.log

test:
	$(CC)  $(CFLAGS) -I$(INCDIR) test/main.c -L lib -lstockfishapi $(LDLIBS)
run:
	@./run

install: $(LIBRARY)
	cp $(LIB_DIR)/$(LIBRARY) /usr/local/lib/
	cp $(INC_DIR)/stockfish_api.h /usr/local/include/

uninstall:
	rm -f /usr/local/lib/$(LIBRARY)
	rm -f /usr/local/include/stockfish_api.h

.PHONY: all clean distclean test run install uninstall
