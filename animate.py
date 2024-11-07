import numpy as np
import pandas as pd
import imageio
from vispy import scene, app

CelestialBodies = 11
N = np.linspace(0, 4*(CelestialBodies - 1), CelestialBodies)
names = ['Sun', 'Mercury', 'Venus', 'Earth', 'Mars',
         'Jupiter', 'Saturn', 'Uranus', 'Neptune', 'Pluto', 'Halley']

orbit_dict ={}
for j, name in zip(N, names):
    j = int(j)
    x, y, z = [], [], []
   
    infile = open("RK4Data.dat", 'r')
    for line in infile:          
        a = line.split()
        x.append(float(a[j]))
        y.append(float(a[1+j]))
        z.append(float(a[2+j]))
    infile.close()
    orbit_dict[name] = {}
    orbit_dict[name]["x"] = x[::20]
    orbit_dict[name]["y"] = y[::20]
    orbit_dict[name]["z"] = z[::20]

# Sample data setup (replace this with your actual orbit data)
N = len(orbit_dict['Sun']['x'])  # Number of frames
planets = ['Sun', 'Mercury', 'Venus', 'Earth', 'Mars', 'Jupiter', 'Saturn',
           'Uranus', 'Neptune', 'Pluto', 'Halley']

# Create DataFrames for each planet
df_dict = {
    planet: pd.DataFrame({
        "time": np.arange(N),
        "x": orbit_dict[planet]["x"],
        "y": orbit_dict[planet]["y"],
        "z": orbit_dict[planet]["z"]
    })
    for planet in planets
}

# Define colors and sizes for each planet
size_dict = {'Sun': 3, 'Mercury': 3, 'Venus': 4, 'Earth': 5, 'Mars': 4,
             'Jupiter': 15, 'Saturn': 13, 'Uranus': 12, 'Neptune': 12, 'Pluto': 3, 'Halley': 2}


proportion_dict = {
'Sun': 1,
'Mercury': 0.0035,
'Venus': 0.0087,
'Earth': 0.0092,
'Mars': 0.0049,
'Jupiter': 0.1004,
'Saturn': 0.0836,
'Uranus': 0.0364,
'Neptune': 0.0354,
'Pluto': 0.002,
'Halley': 0.0018,
}

color_dict = {'Sun': (1, 1, 0), 'Mercury': (0.5, 0.5, 0.5), 'Venus': (1, 0.5, 0),
              'Earth': (0, 0, 1), 'Mars': (1, 0, 0), 'Jupiter': (0.5, 0.25, 0),
              'Saturn': (1, 0.84, 0), 'Uranus': (0.678, 0.847, 0.902),
              'Neptune': (0.5, 0.0, 0.5), 'Pluto': (1, 1, 1), 'Halley': (1, 1, 1)}

# Create a VisPy canvas for 3D rendering

canvas = scene.SceneCanvas(keys='interactive', show=True, size=(800, 600))

view = canvas.central_widget.add_view(camera='turntable')

# view = canvas.central_widget.add_view()

view.camera = scene.cameras.TurntableCamera(elevation=90, azimuth=90)
view.camera.fov = 45  # Field of view

view.camera.set_range(x=(-35, 35), y=(-35, 35), z=(-15, 15))

# view.bgcolor = (1.0, 1.0, 1.0, 1.0)  # White


# Create a list to hold planet visual objects
planet_visuals = {}
# Create planet markers
graph_dict = {}
for planet in df_dict.keys():
    scatter = scene.Markers()
    scatter.set_data(np.array([[orbit_dict[planet]["x"][0], orbit_dict[planet]["y"][0], orbit_dict[planet]["z"][0]]]),
                     face_color=color_dict[planet], size=np.log(proportion_dict[planet] * 100000))
    view.add(scatter)
    graph_dict[planet] = scatter

# Update function to animate planet positions
def update_graph(frame):
    for planet, scatter in graph_dict.items():
        data = np.array([[orbit_dict[planet]["x"][frame], orbit_dict[planet]["y"][frame], orbit_dict[planet]["z"][frame]]])
        scatter.set_data(data, face_color=color_dict[planet], size=np.log(proportion_dict[planet] * 100000))

# Set up video writer
writer = imageio.get_writer('orbit_animation.mp4', fps=40)

# Frame capture function
def capture_frame(event):
    frame = canvas.render()
    writer.append_data(frame)  # Append frame to video

# Timer to animate and capture frames
timer = app.Timer(interval=0.001, start=True)
frame_counter = 0

@timer.connect
def on_timer(event):
    global frame_counter
    if frame_counter < N:
        update_graph(frame_counter)
        capture_frame(event)
        frame_counter += 1
    else:
        writer.close()  # Close video file
        timer.stop()  # Stop timer
        app.quit()  # Quit Vispy app



legend_y_pos = 38  # Start position for the legend items
legend_x_pos = 38  # Start position for the legend items
for i, (planet, color) in enumerate(color_dict.items()):
    # Add a small colored marker
    marker = scene.Markers(parent=view.scene)
    marker.set_data(np.array([[legend_x_pos, legend_y_pos - i * 8, 0]]), face_color=color, size=np.log(proportion_dict[planet] * 100000))

    # Add text label for the planet
    text = scene.Text(planet, color='white', font_size=900, pos=(legend_x_pos + 2.5, legend_y_pos - i * 8 ),
parent=view.scene)

app.run()

