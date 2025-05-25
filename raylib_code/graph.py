import matplotlib.pyplot as plt
import numpy as np
from mpl_toolkits.mplot3d import Axes3D

# Data for each biome
biomes = {
    "Forêt tempérée": {"temp": (5, 25), "humidity": (60, 80), "rainfall": (500, 1500)},
    "Forêt boréale": {"temp": (-10, 10), "humidity": (50, 70), "rainfall": (200, 600)},
    "Forêt tropicale humide": {"temp": (20, 35), "humidity": (75, 95), "rainfall": (2000, 5000)},
    "Forêt tropicale sèche": {"temp": (25, 35), "humidity": (40, 70), "rainfall": (1000, 2000)},
    "Forêt méditerranéenne": {"temp": (20, 40), "humidity": (60, 80), "rainfall": (300, 800)},
}

# Create a 3D plot with filled areas
fig = plt.figure(figsize=(12, 10))
ax = fig.add_subplot(111, projection='3d')

# Plot each biome as a semi-transparent plane
colors = ['r', 'g', 'b', 'c', 'm']
for idx, (biome, data) in enumerate(biomes.items()):
    temp = data["temp"]
    humidity = data["humidity"]
    rainfall = data["rainfall"]
    
    # Create a grid for the plane
    x = np.linspace(temp[0], temp[1], 10)
    y = np.linspace(humidity[0], humidity[1], 10)
    X, Y = np.meshgrid(x, y)
    Z = np.ones_like(X) * rainfall[0]
    
    # Plot the bottom face
    ax.plot_surface(X, Y, Z, color=colors[idx], alpha=0.3, label=biome)
    
    # Plot the top face
    Z = np.ones_like(X) * rainfall[1]
    ax.plot_surface(X, Y, Z, color=colors[idx], alpha=0.3)
    
    # Plot the vertical edges
    for x_val in [temp[0], temp[1]]:
        for y_val in [humidity[0], humidity[1]]:
            ax.plot([x_val, x_val], [y_val, y_val], [rainfall[0], rainfall[1]], color=colors[idx], alpha=0.5)

# Set labels and title
ax.set_xlabel('Température moyenne (°C)')
ax.set_ylabel('Humidité (%)')
ax.set_zlabel('Précipitations annuelles (mm)')
ax.set_title('Visualisation 3D des Biomes avec Zones Remplies')

# Show legend
ax.legend()

# Show plot
plt.show()
