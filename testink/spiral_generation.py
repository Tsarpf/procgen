#%%
import numpy as np
import matplotlib.pyplot as plt

def generate_spiral(N):
    # Initialize the grid
    grid_size = 2 * N + 1
    grid = np.zeros((grid_size, grid_size), dtype=int)
    directions = [(0, 1), (1, 0), (0, -1), (-1, 0)]  # right, down, left, up
    x, y = N, N  # Start from the center of the grid
    dx, dy = directions[0]  # Start with moving right
    step, change = 1, 0  # Step size and step change index
    count = 0  # Counter for steps in current direction

    for i in range(1, grid_size**2 + 1):
        grid[x, y] = i  # Place a block
        if count == step:  # Time to change direction
            dx, dy = directions[(change + 1) % 4]  # Change direction
            change = (change + 1) % 4
            if change % 2 == 0:
                step += 1  # Increase step size every full loop (2 direction changes)
            count = 0  # Reset counter

        x += dx
        y += dy
        count += 1

    return grid

N = 5
spiral = generate_spiral(N)

# Plotting with labels on each block
plt.figure(figsize=(8, 8))
cmap = plt.cm.viridis
norm = plt.Normalize(vmin=spiral.min(), vmax=spiral.max())
rgba = cmap(norm(spiral))

# Create a color grid object with text annotations
for (i, j), val in np.ndenumerate(spiral):
    plt.text(j, i, int(val), color='black' if rgba[i, j, 0] > 0.5 else 'white', ha='center', va='center', fontsize=8)

plt.imshow(rgba, aspect='equal')
plt.title('2D Spiral of Blocks (N=5) with Labels')
plt.grid(which='both', color='white', linestyle='-', linewidth=2)
plt.xticks(ticks=np.arange(-0.5, N*2+1, 1), labels=[])
plt.yticks(ticks=np.arange(-0.5, N*2+1, 1), labels=[])
plt.show()

#%%
def plot_3d_spiral_corrected_with_indices(N, height):
    spiral = generate_spiral(N)
    fig = plt.figure(figsize=(10, 10))
    ax = fig.add_subplot(111, projection='3d')
    
    # Create color map based on the top view values
    norm = plt.Normalize(vmin=spiral.min(), vmax=spiral.max())
    cmap = plt.cm.viridis

    block_size = 0.5  # Smaller than 1 to create gaps
    offset = (1 - block_size) / 2

    # First plot all the blocks
    for (x, z), value in np.ndenumerate(spiral):
        colors = cmap(norm(value))
        for y in range(height):  # Create vertical columns
            ax.bar3d(x + offset, z + offset, y, block_size, block_size, block_size, color=colors, alpha=0.8, shade=True)

    fig.canvas.draw()  # Force the canvas to redraw after adding text
    # Set plot appearance
    ax.view_init(elev=40, azim=120)  # Adjusted viewing angle
    ax.set_title('3D Spiral Blocks Corrected with Indices')
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.set_zlabel('Z')
    ax.set_xlim([0, 2*N])
    ax.set_ylim([0, 2*N])
    ax.set_zlim([0, height + 1])  # Make sure zlim is high enough to show text

    plt.show()

plot_3d_spiral_corrected_with_indices(N=5, height=10)

# %%
