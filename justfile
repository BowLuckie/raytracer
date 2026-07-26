default: 
    mkdir -p ./build
    g++ -o build/main src/main.cpp
    ./build/main
    display ./out.ppm
