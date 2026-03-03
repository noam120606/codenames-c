PORT=4242
SERVER_IP="172.18.41.75"

make clean
make
./build/client -s $SERVER_IP -p $PORT