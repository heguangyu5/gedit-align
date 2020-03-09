SOURCES = gedit-plugin-align.c \
          gedit-plugin-align-configurable.c

libalign.so: $(SOURCES)
	gcc -Wall -fPIC -shared -o libalign.so `pkg-config --cflags gedit gtksourceview-3.0` $(SOURCES) `pkg-config --libs gedit gtksourceview-3.0`
