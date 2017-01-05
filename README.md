## wwwconf - fast engine for tree-structured forums (boards).
Exported from https://code.google.com/archive/p/wwwconf/ for the further development.

License: GNU GPL v3.

Main goals:
* Fast rendering of all pages.
* Minimal list of dependencies and low footprint (resource usage).
* Compatibility with "special" clients - it must work even without JS and it shouldn't behave badly under such conditions.
* Unicode characters shouldn't be able to break things and should be supported as much as possible.

Disclaimer: this is a legacy software written in C with random bits in C++.
Most of the development was focused on premature optimizations.
Code quality was (and still is) quite low. You've been warned.

### Building from source.
Due to bad usage of C types currently only 32-bit little-endian platforms are supported.

Other requirements: Make, C++ compiler (both GCC and Clang are tested) and UNIX-like environment.
Native Win32 support didn't worth the efforts to support it and was dropped long ago,
use Cygwin or similar libraries if you really need it.

For 32-bit Debian (and its derivatives) you should install `libc6-dev` and `g++`,
`clang` is also a good choice. For 64-bit Debian you'd need `libc6-dev-i386`
(or `libc6-dev:i386` if you've added i386 architecture) and `g++-multilib` or `clang`.

I hope you've got the idea in case you are using one of the other OSes.
Now you can just run `make` and get two binaries:
* `dbtool` - CLI tool for operations with the custom DB.
* `index.cgi` - forum engine itself, it requires CGI-compliant HTTP server.

### Testing and running.
As it was already mentioned, CGI-compliant HTTP server is required.
You can choose Nginx + uwsgi (or fcgi-wrap), Lighttpd, Boa, etc...

Here's a simple way to get started (don't forget to install uwsgi-core):
```
# create initial database
./dbtool -n
# (optional step) create user 'Admin' with password 'AdminPass':
./dbtool -aa Admin AdminPass
# run single uwsgi worker (no master process) in CGI mode, bind it to port 7542.
uwsgi-core --plugins http,cgi --http-socket :7542 --cgi . --cgi-allowed-ext .cgi --http-socket-modifier1 9 --check-static static
```
Then you can go to http://127.0.0.1:7542/index.cgi to see a working board.
