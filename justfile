default: 
    mkdir -p ./build
    g++ -O3 -fopenmp -o build/main src/main.cpp
    time ./build/main
    display ./out.ppm || timg out.ppm
