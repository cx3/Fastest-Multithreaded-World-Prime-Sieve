rm a.out
g++ main.cpp -std=c++17 -Wall -lpthread -I/usr/lib/x86_64-linux-gnu/ -O2
chmod +x a.out
./a.out n=1000000000 threads=4 file=miliard.txt
