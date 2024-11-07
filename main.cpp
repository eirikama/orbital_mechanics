#include "CelestialBody.h"
#include "System.h"

#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>

int main()
{
  System SolarSystem;
  
  // Masses in units of Solar masses 
  
    // Open and parse the JSON file
  std::ifstream file("initial_conditions.json");
  if (!file.is_open()) {
    std::cerr << "Error opening planets.json file." << std::endl;
    return 1;
  }

  nlohmann::json data;
  file >> data;

  double total_vx = 0.0, total_vy = 0.0, total_vz = 0.0;

for (const auto& body : data["celestial_bodies"]) {
    std::string name = body["name"];

    if (name != "Sun") {
	double mass = static_cast<double>(body["mass"]);
    
    	std::vector<double> position = body["position"].get<std::vector<double>>();
    	std::vector<double> velocity;

        velocity = body["velocity"].get<std::vector<double>>();
        total_vx += mass * velocity[0];
        total_vy += mass * velocity[1];
        total_vz += mass * velocity[2];


    SolarSystem.AddBody(position[0], position[1], position[2], 
                        velocity[0], velocity[1], velocity[2], mass, name);
    }
}

  SolarSystem.AddBody(0, 0, 0, -total_vx, -total_vy, -total_vz, 1, "Sun");


  SolarSystem.SolverSetup();
  SolarSystem.RK4Evolve(248);

}
