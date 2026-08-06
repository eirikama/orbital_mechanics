import json
import requests
import urllib3

urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)

# Added '1P' for Halley's Comet
bodies = [
    {"name": "Sun", "id": "10", "mass": 1.0},
    {"name": "Mercury", "id": "199", "mass": 1.6601e-7},
    {"name": "Venus", "id": "299", "mass": 2.4478e-6},
    {"name": "Earth", "id": "399", "mass": 3.0034e-6},
    {"name": "Moon", "id": "301", "mass": 3.6942e-8},
    {"name": "Mars", "id": "499", "mass": 3.2271e-7},
    {"name": "Jupiter", "id": "599", "mass": 9.5479e-4},
    {"name": "Saturn", "id": "699", "mass": 2.8588e-4},
    {"name": "Uranus", "id": "799", "mass": 4.3662e-5},
    {"name": "Neptune", "id": "899", "mass": 5.1513e-5},
    {"name": "Pluto", "id": "999", "mass": 6.5821e-9},
    # {"name": "Io", "id": "501", "mass": 4.217e-8},
    # {"name": "Europa", "id": "502", "mass": 2.535e-8},
    # {"name": "Ganymede", "id": "503", "mass": 7.804e-8},
    # {"name": "Callisto", "id": "504", "mass": 5.668e-8},
    # {"name": "Titan", "id": "606", "mass": 6.460e-8},
    {
        "name": "Halley",
        "id": "90000030",
        "mass": 1.1051e-11,
    },
    # {"name": "Ceres", "id": "1", "mass": 4.719e-10},
]

EPOCH = "2026-02-25"


def get_horizons_data(obj_id, epoch):
    url = "https://ssd.jpl.nasa.gov/api/horizons.api"
    params = {
        "format": "json",
        "COMMAND": f"'{obj_id}'",  # Added quotes to handle alphanumeric IDs like 1P
        "OBJ_DATA": "NO",
        "MAKE_EPHEM": "YES",
        "EPHEM_TYPE": "VECTORS",
        "CENTER": "500@0",
        "START_TIME": epoch,
        "STOP_TIME": "2026-02-26",
        "STEP_SIZE": "1d",
        "REF_PLANE": "ECLIPTIC",
        "CSV_FORMAT": "YES",
        "VEC_TABLE": "3",
        "OUT_UNITS": "KM-S",
    }

    response = requests.get(url, params=params, verify=False)
    data = response.json()
    result_text = data.get("result", "")

    if "$$SOE" not in result_text:
        return None

    lines = result_text.split("\n")
    for i, line in enumerate(lines):
        if "$$SOE" in line:
            vals = [v.strip() for v in lines[i + 1].split(",")]
            return [
                float(vals[2]),
                float(vals[3]),
                float(vals[4]),
                float(vals[5]),
                float(vals[6]),
                float(vals[7]),
            ]
    return None


def main():
    final_output = {
        "epoch": f"{EPOCH}T00:00:00Z",
        "units": {"distance": "km", "velocity": "km/s", "mass": "M_sun"},
        "celestial_bodies": [],
    }

    for body in bodies:
        vectors = get_horizons_data(body["id"], EPOCH)
        if vectors:
            final_output["celestial_bodies"].append(
                {
                    "name": body["name"],
                    "mass": body["mass"],
                    "position": vectors[:3],
                    "velocity": vectors[3:],
                }
            )
            print(f"  [OK]  {body['name']}")

    with open("initial_conditions.json", "w") as f:
        json.dump(final_output, f, indent=4)


if __name__ == "__main__":
    main()
