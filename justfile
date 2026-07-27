default: 
    mkdir -p ./build
    g++ -o build/main src/main.cpp
    time ./build/main
    display ./out.ppm || timg out.ppm
