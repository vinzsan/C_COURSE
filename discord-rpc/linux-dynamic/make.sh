gcc main.c -o myrpc -I./lib -L./lib -ldiscord-rpc
LD_LIBRARY_PATH=./lib ./myrpc

