rm a.out
g++ main.cpp -std=c++17 -Wall -lpthread -I/usr/lib/x86_64-linux-gnu/
chmod +x a.out
./a.out n=100000 threads=2 file=filenametest.txt
