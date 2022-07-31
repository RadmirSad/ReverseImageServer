# ReverseImageServer
This repository for the test task for writing an HTTP service that accepts requests to mirror images horizontally

Before you'll start you need to install libraries "libevent" for simple HTTP server and "stb" for working with images
On Ubuntu you can do this with commands written below

"sudo apt-get install libevent-dev"

"sudo apt install libstb-dev"

The service accepts HTTP connections. 
The body of the request expects a JPEG-encoded image.
The response is a mirrored image also in JPEG format.
