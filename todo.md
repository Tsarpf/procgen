# If chunks are added by position, they can be generated way outside the octree - the octree should be grown/enlarged multiple times in a row in this case
# Fix the last piece of seams between two seams... 
    - It's this one by nick gildea https://blogger.googleusercontent.com/img/b/R29vZ2xl/AVvXsEjS5IvTGbndbV0XX0nonX-40FclWFjFiJLJU5LMnpUFEgBAEE642uz-TN3kXeqtsXqKUNnNp1S7N-vNNNABssT-FVt8nenFfWbuthD2KPs7d69PS6VVwdVZyOqwNIbPjunc80GQQA7S9jI/s1600/select_seams_4.png
# Make it work with Godot as library!

# Implement CSGs

# Dear ImGui
- Would be nice to be able to click a button to toggle
    - Wireframe on/off
    - Spawn a new chunk in a specific direction



# Mem optimizations:
0) Octree node class is pretty bulky, and even all non-leaf nodes get assigned stuff like Qef data currently.

1) use halfs, ie. 16 bit floats --> immediately halving the memory requirements.

2) investigate if on-grid points could only save booleans whether they're solid or not. along with 3)

3) Only save one point between grid points, the one where the value is closest to zero which is the only point we will use as the zero crossing anyway
It only needs 1 float for distance between points and 3 floats for the gradient.

# Other
- Rename "OctreeMesh" to RootOctree or something, it seems to just be managing octree related miscellaneous data
    - Is it handling stuff both about the full scene and individual meshes/octree nodes? weird stuff, needs a mega refactoring
- Rename Octree to OctreeNode or just Node or something
    - Maybe leaf nodes should be a different class or something? Or maybe inherit from general nodes, add the Qef stuff etc leaf-related data on top.