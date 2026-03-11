PORT=4242
SERVER_IP="127.0.0.1"

make clean
make
./build/client -s $SERVER_IP -p $PORT