# Running
```
mkdir build
cd build
cmake ..
make
```
For me
- on Windows it just works, provided msvc is installed, you should find the executable in the build dir
- on OSX (Intel and ARM) it seems to work straight away, at least having xcode etc. installed. Running `make` as above creates `build/dual-contouring.app/Contents/MacOS/dual-contouring`
- On my Ubuntu when I tried years ago, I installed `freeglut3 freeglut3-dev xorg-dev libglu1-mesa-dev` and it seemed to find the required OpenGL headers and work.

Uses structured bindings so you need C++17.

# Background
Working on comprehending and possibly in the future extending various (dual) contouring methods.

It now actually works (on my machine)! 

`cuda-noise` branch has the cuda implementation, but it's slower than the CPU one for now. 
Since most of the time is spent sending data to the GPU, in the branch `precompute-zcrossing` there is 
work towards sending just the zerocrossings per point per direction (=3 * 4 floats) to the GPU instead of n (n=8 here)
points between two points of the octree. Or it might be worth it to do "everything" (octree stuff etc) in the GPU end and just send some stuff back.

It seems like seams (that are a problem with many DC implementations) can easily be handled just by re-running DC for each neighbours' border points'. IIRC. Check the code.

![debug](https://cldup.com/SXcjAPRqY4.gif)
![cube](https://cldup.com/LABApkDK26.gif)
![sphere](https://cldup.com/yEe4nxdPHS.gif)


## Licensing
Everything written by me is (now) MIT licensed

There are some LGPL functions (they specifically start with a comment noting the license) copied from here https://github.com/nickgildea/DualContouringSample, that are originally from the DC paper's example implementation.

The svd and qef (.cpp) implementations are also from nickgildeas repo, where it says "The QEF implementation was written by /u/unzret (on reddit.com)" and the license is the "unlicense", see the beginning of those files for details.

## Dependencies

 * [GLFW](https://github.com/glfw/glfw)
 * [GLEW](http://github.com/nigels-com/glew.git)
 * [GLM](https://github.com/g-truc/glm)
 * [CMake](http://www.cmake.org/)
 * [libnoise](http://libnoise.sourceforge.net/)
