import pygame
from pygame.locals import *
from OpenGL.GL import *
from OpenGL.GLU import *

# Define chunk size and starting position
chunk_size = 1.0
chunks = { (0,0,0) }  # Using a set to track chunk positions

def main():
    pygame.init()
    display = (800, 600)
    pygame.display.set_mode(display, DOUBLEBUF|OPENGL)
    gluPerspective(45, (display[0]/display[1]), 0.1, 50.0)
    glTranslatef(0.0,0.0, -10)

    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                quit()
            if event.type == pygame.KEYDOWN:
                handle_key_event(event.key)

        #glRotatef(1, 3, 1, 1)
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT)
        draw_world()
        pygame.display.flip()
        pygame.time.wait(10)

def handle_key_event(key):
    directions = {
        pygame.K_1: (1, 0, 0),   # Right
        pygame.K_2: (-1, 0, 0),  # Left
        pygame.K_3: (0, 1, 0),   # Up
        pygame.K_4: (0, -1, 0),  # Down
        pygame.K_5: (0, 0, 1),   # Forward
        pygame.K_6: (0, 0, -1)   # Backward
    }
    if key in directions:
        add_chunk(directions[key])

def add_chunk(direction):
    # Find the last added chunk's position
    last_pos = next(iter(chunks))
    new_pos = tuple(map(lambda x, y: x + y * chunk_size, last_pos, direction))
    chunks.add(new_pos)

def draw_world():
    for pos in chunks:
        draw_cube(pos)

def draw_cube(position):
    x, y, z = position
    glPushMatrix()
    glTranslate(x, y, z)
    glBegin(GL_QUADS)
    for surface in surfaces:
        for vertex in surface:
            glVertex3fv(vertices[vertex])
    glEnd()
    glPopMatrix()

vertices = (
    (1, -1, -1),
    (1, 1, -1),
    (-1, 1, -1),
    (-1, -1, -1),
    (1, -1, 1),
    (1, 1, 1),
    (-1, -1, 1),
    (-1, 1, 1)
)

surfaces = (
    (0,1,2,3),
    (3,2,7,6),
    (6,7,5,4),
    (4,5,1,0),
    (1,5,7,2),
    (4,0,3,6)
)

if __name__ == "__main__":
    main()
