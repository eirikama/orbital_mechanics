import numpy as np
import imageio
from vispy import scene, app

CelestialBodies = 11
N = np.linspace(0, 4 * (CelestialBodies - 1), CelestialBodies)
names = [
    "Mercury",
    "Venus",
    "Earth",
    "Mars",
    "Jupiter",
    "Saturn",
    "Uranus",
    "Neptune",
    "Pluto",
    "Halley",
    "Sun",
]

orbit_dict = {}
for j, name in zip(N, names):
    j = int(j)
    x, y, z = [], [], []

    with open("RK4Data.dat", "r") as infile:
        for line in infile:
            a = line.split()
            x.append(float(a[j]))
            y.append(float(a[1 + j]))
            z.append(float(a[2 + j]))

    orbit_dict[name] = {
        "x": x[::50],
        "y": y[::50],
        "z": z[::50],
    }

# Frame count
N = len(orbit_dict["Sun"]["x"])

proportion_dict = {
    "Sun": 1,
    "Mercury": 0.0035,
    "Venus": 0.0087,
    "Earth": 0.0092,
    "Mars": 0.0049,
    "Jupiter": 0.1004,
    "Saturn": 0.0836,
    "Uranus": 0.0364,
    "Neptune": 0.0354,
    "Pluto": 0.002,
    "Halley": 0.0018,
}

color_dict = {
    "Sun": (1, 1, 0),
    "Mercury": (0.5, 0.5, 0.5),
    "Venus": (1, 0.5, 0),
    "Earth": (0, 0, 1),
    "Mars": (1, 0, 0),
    "Jupiter": (0.5, 0.25, 0),
    "Saturn": (1, 0.84, 0),
    "Uranus": (0.678, 0.847, 0.902),
    "Neptune": (0.5, 0.0, 0.5),
    "Pluto": (1, 1, 1),
    "Halley": (1, 1, 1),
}

# Create VisPy canvas for 3D rendering
canvas = scene.SceneCanvas(keys="interactive", show=True, size=(656, 608))
view = canvas.central_widget.add_view(camera="turntable")
view.camera = scene.cameras.TurntableCamera(elevation=90, azimuth=0)
view.camera.fov = 5
view.camera.set_range(x=(-34, 7), y=(14.5, 15), z=(-0.0, 0.0))
# canvas.bgcolor = (1.0, 1.0, 1.0, 1.0)  # RGBA, with alpha=1 for opaque

# Initialize markers and trajectories
planet_markers = {}
trajectory_lines = {}

for planet, data in orbit_dict.items():
    # Planet markers
    scatter = scene.Markers()
    scatter.set_data(
        np.array([[data["x"][0], data["y"][0], data["z"][0]]]),
        face_color=color_dict[planet],
        size=np.log(proportion_dict[planet] ),
    )
    view.add(scatter)
    planet_markers[planet] = scatter

    # Initialize trajectory line with the first position of the planet
    line = scene.Line(
        pos=np.array(
            [[data["x"][0], data["y"][0], data["z"][0]]]
        ),  # Initialize with the first position
        color=color_dict[planet] + (0.3,),
        width=1.5,
        parent=view.scene,
    )
    trajectory_lines[planet] = line

# Animation update function
def update_graph(frame):
    for planet, scatter in planet_markers.items():
        # Update marker position
        new_position = np.array(
            [
                [
                    orbit_dict[planet]["x"][frame],
                    orbit_dict[planet]["y"][frame],
                    orbit_dict[planet]["z"][frame],
                ]
            ]
        )
        scatter.set_data(new_position, face_color=color_dict[planet])

        # Update trajectory line by appending new position
        current_positions = trajectory_lines[planet].pos
        new_positions = np.vstack([current_positions, new_position])
        trajectory_lines[planet].set_data(new_positions)


# Set up video writer
writer = imageio.get_writer("orbit_animation_with_trajectories.mp4", fps=30)

# Frame capture function
def capture_frame(event):
    frame = canvas.render()
    writer.append_data(frame)


# Timer for animation
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
        writer.close()
        timer.stop()
        app.quit()


legend_y_pos = 23  # Start position for the legend items
legend_x_pos = -36  # Start position for the legend items
for i, (planet, color) in enumerate(color_dict.items()):
    # Add a small colored marker
    marker = scene.Markers(parent=view.scene)
    marker.set_data(
        np.array([[legend_x_pos, legend_y_pos - i * 2, 0]]),
        face_color=color,
        size=np.log(proportion_dict[planet] * 10e5),
    )

    # Add text label for the planet
    text = scene.Text(
        planet,
        color="white",
        font_size=4500,
        pos=(legend_x_pos + 3.0, legend_y_pos - i * 2),
        parent=view.scene,
    )


app.run()
