# BHV raytracer 

Fork from [raytracer](https://github.com/bramvito/raytracing/) by Bram Vito.

## Build instructions

If on Windows, it uses SDL2 for window management and image rendering which is included in the repository.
```bash 
mingw32-make 
.\rtweekend
```

## Changes

You can see the additions and changes in the following diff url: [Diff](https://github.com/mattie078/INFOMAGR-1-2024/compare/75254b1..cbdf17d)

Notably, the following changes were made:
- Fixed SDL2 window creation and rendering as it was not working on Windows
- Added a 10k triangle model of the Stanford Dragon
- Changed the Camera properties to be in line with kd-tree branch (ex: the up vector is using a different axis for Blender models)
- Fixed a bug in the .obj loader that caused the model to be imported twice

## Debug mode

If you want to run the program in debug mode, you can uncomment rtweekend.h:11 and recompile the program. This will make it slower but it will print out the results of the rendering process at the end.