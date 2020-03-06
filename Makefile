libalign.so: gedit-plugin-align.c
	gcc -Wall -fPIC -shared -o libalign.so `pkg-config --cflags gedit` gedit-plugin-align.c `pkg-config --libs gedit`
