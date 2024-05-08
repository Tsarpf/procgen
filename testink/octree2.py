from panda3d.core import Vec3, AmbientLight, DirectionalLight, Point3
from direct.showbase.ShowBase import ShowBase
import random

class OctreeNode:
    def __init__(self, center, size, depth=0):
        self.center = center
        self.size = size
        self.depth = depth
        self.children = None
        self.cubes = []

    def is_leaf(self):
        return self.children is None

    def insert(self, cube, pos):
        if self.is_leaf():
            self.cubes.append((cube, pos))
            return True
        else:
            # Determine the correct child to insert into
            index = 0
            if pos.x >= self.center.x:
                index |= 1
            if pos.y >= self.center.y:
                index |= 2
            if pos.z >= self.center.z:
                index |= 4
            return self.children[index].insert(cube, pos)

    def subdivide(self):
        if not self.is_leaf():
            return  # already subdivided
        half = self.size / 2
        quarter = self.size / 4
        self.children = [None] * 8
        for i in range(8):
            offset = Vec3(quarter * (1 if i & 1 else -1),
                          quarter * (1 if i & 2 else -1),
                          quarter * (1 if i & 4 else -1))
            child_center = self.center + offset
            self.children[i] = OctreeNode(child_center, half, self.depth + 1)

class OctreeDemo(ShowBase):
    def __init__(self):
        super().__init__()
        self.disableMouse()
        self.camera.setPos(0, -50, 0)
        self.camera.lookAt(0, 0, 0)
        self.setup_scene()
        self.root = OctreeNode(Vec3(0, 0, 0), 50)
        self.setup_controls()

    def setup_scene(self):
        # Lighting
        alight = AmbientLight('ambient')
        alight.setColor((0.5, 0.5, 0.5, 1))
        alnp = self.render.attachNewNode(alight)
        self.render.setLight(alnp)

        dlight = DirectionalLight('directional')
        dlight.setColor((0.8, 0.8, 0.8, 1))
        dlnp = self.render.attachNewNode(dlight)
        dlnp.setHpr(0, -60, 0)
        self.render.setLight(dlnp)

    def setup_controls(self):
        self.accept('1', self.add_cube, [Vec3(2, 0, 0)])
        self.accept('2', self.add_cube, [Vec3(-2, 0, 0)])
        self.accept('3', self.add_cube, [Vec3(0, 2, 0)])
        self.accept('4', self.add_cube, [Vec3(0, -2, 0)])
        self.accept('5', self.add_cube, [Vec3(0, 0, 2)])
        self.accept('6', self.add_cube, [Vec3(0, 0, -2)])
        self.accept('escape', self.close_app)

    def add_cube(self, direction):
        new_pos = Vec3(0, 0, 0) if not self.root.cubes else self.root.cubes[-1][1] + direction
        new_cube = self.loader.loadModel("models/box")
        new_cube.setScale(1)
        new_cube.setPos(new_pos)
        new_cube.reparentTo(self.render)
        if not self.root.insert(new_cube, new_pos):
            self.root.subdivide()
            assert self.root.insert(new_cube, new_pos), "Failed to insert after subdivision"

    def close_app(self):
        self.userExit()

app = OctreeDemo()
app.run()
