This is a complete mp3 player example for the xtcw toolkit.
To start this example you need to have

* taglib (https://taglib.github.io/)

* pkgconfig

* mpeg123

* socat

installed.

than you need to create a database with mp3 songs:

* create-database /path/music/all-mp3

then you can start mpeg123 with remote interface enabled and
stdin/stdout/stderr redirected to a network socket.

* start-mpg123.sh

To finaly start the GUI use:

* ./ex11 -geometry 400x600

READY.
