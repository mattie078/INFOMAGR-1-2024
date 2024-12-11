# kd-tree raytracer 

Fork from [ray-tracing-kdtree](https://github.com/sio13/ray-tracing-kdtree) by Martin Gaňo.

## Build instructions

I like to use MinGW32 Makefiles. For a quick setup, you can use the following commands:
```bash 
cmake -G "MinGW Makefiles" -B build
cd build
mingw32-make
.\raytracer.exe
```

## Changes

You can see the additions and changes in the following diff url: [Diff](https://github.com/mattie078/INFOMAGR-1-2024/compare/d5e4c1c..d47a110)

Notably, the following changes were made:
- Made the scene the same as the bvh branch
- Implemented worker job multithreading for rendering the image
- Implemented samples per pixel to be in line with the bvh branch
- Added a 10k triangle model of the Stanford Dragon
- Added vof for the Camera class
- Added scaling for .obj models and fixed related segfaults
- Added instrumentation for the rendering process

## Debug mode

If you want to run the program in debug (verbose) mode, you can set defaults.h:9 and recompile the program. This will make it slower but it will print out the results of the rendering process at the end.