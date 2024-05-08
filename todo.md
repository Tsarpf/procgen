# Make it work with Godot as library!

# Implement CSGs

# Dear ImGui
- Would be nice to be able to click a button to toggle
    - Wireframe on/off
    - Spawn a new chunk in some direction

# Enlarge bug
- The sizes are doubling and that's not what should be happening? Fix! Probably easy ISH to keep track of size of octree against original size and find the new position inside the new parent octree?

# Mem optimizations:
1) use halfs, ie. 16 bit floats --> immediately halving the memory requirements.

2) investigate if on-grid points could only save booleans whether they're solid or not. along with 3)

3) Only save one point between grid points, the one where the value is closest to zero which is the only point we will use as the zero crossing anyway
It only needs 1 float for distance between points and 3 floats for the gradient.
