Working on comprehending and possibly in the future extending various (dual) contouring methods.

It now actually works (on my machine)! 

![debug](https://cldup.com/5IHvDNQ6TI.gif)
![cube](https://cldup.com/LABApkDK26.gif)
![sphere](https://cldup.com/yEe4nxdPHS.gif)

To build
```
mkdir build
cd build
cmake ..
```

And enable C++17 features in your compiler. They're only used for structured bindings right now so easy to remove as well.

## Licensing
Assume everything written by me so far is WTFPL or whatever. 

There are some LGPL functions (they specifically start with a comment noting the license) copied from here https://github.com/nickgildea/DualContouringSample, that are originally from the DC paper's example implementation.

The svd and qef (.cpp) implementations are also from nickgildeas repo, where it says "The QEF implementation was written by /u/unzret (on reddit.com)" and the license is the "unlicense", see the beginning of those files for details.

## Dependencies

 * [GLFW](https://github.com/glfw/glfw)
 * [GLEW](http://github.com/nigels-com/glew.git)
 * [GLM](https://github.com/g-truc/glm)
 * [CMake](http://www.cmake.org/)
 * [libnoise](http://libnoise.sourceforge.net/)
