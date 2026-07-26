# raytracer

## building

to build and generate an image just run the just script

```bash
just
```

this runs this:

```bash
mkdir -p ./build
g++ -o build/main src/main.cpp
./build/main
display ./out.ppm || timg out.ppm
```

imagemagick or timg are used by this script but you can use any image veiwer,
the output is a ppm file.
