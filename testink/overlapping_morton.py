#%%
def morton_encode(x, y):
    def part1by1(n):
        n &= 0x0000ffff
        n = (n ^ (n << 8)) & 0x00ff00ff
        n = (n ^ (n << 4)) & 0x0f0f0f0f
        n = (n ^ (n << 2)) & 0x33333333
        n = (n ^ (n << 1)) & 0x55555555
        return n

    return (part1by1(y) << 1) + part1by1(x)


def morton_decode(morton):
    def compact1by1(x):
        x &= 0x55555555
        x = (x ^ (x >> 1)) & 0x33333333
        x = (x ^ (x >> 2)) & 0x0f0f0f0f
        x = (x ^ (x >> 4)) & 0x00ff00ff
        x = (x ^ (x >> 8)) & 0x0000ffff
        return x

    return compact1by1(morton), compact1by1(morton >> 1)


def generate_overlapping_grid(num_squares_x, num_squares_y, window_size=2):
    morton_grid = [[morton_encode(x, y) for x in range(num_squares_x)] for y in range(num_squares_y)]
    overlapping_grid = []

    for i in range(num_squares_y - window_size + 1):
        for j in range(num_squares_x - window_size + 1):
            window = [morton_decode(morton_grid[y][x]) for y in range(i, i + window_size) for x in range(j, j + window_size)]
            overlapping_grid.append(window)

    return overlapping_grid

num_squares_x = 2
num_squares_y = 2
overlapping_grid = generate_overlapping_grid(num_squares_x, num_squares_y)

for square in overlapping_grid:
    print(square)

# %%
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import random

def generate_color():
    return (random.uniform(0, 1), random.uniform(0, 1), random.uniform(0, 1))

def draw_overlapping_grid(overlapping_grid):
    fig, ax = plt.subplots()
    ax.set_xlim(0, num_squares_x)
    ax.set_ylim(0, num_squares_y)
    ax.set_aspect('equal')

    for square in overlapping_grid:
        (x1, y1), (x2, y2), (x3, y3), (x4, y4) = square
        rect = patches.Rectangle((x1, y1), x2 - x1, y3 - y1, linewidth=1, edgecolor='black', facecolor=generate_color(), alpha=0.5)
        ax.add_patch(rect)

    plt.show()

draw_overlapping_grid(overlapping_grid)


# %%
