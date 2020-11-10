#g++ -std=c++17 -g -ggdb -O0 -fsanitize=address -o bk bk.cpp
g++ -std=c++17 -O3 -o bk bk.cpp
g++ -std=c++17 -O3 -o bk_without_sorting -DWITHOUT_SORTING bk.cpp
