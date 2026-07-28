default: 
    mkdir -p ./build
    g++ -O3 -fopenmp -o build/main src/main.cpp -lraylib -lm -lpthread -ldl -lrt -lX11
    time ./build/main
