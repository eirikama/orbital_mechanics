#ifndef CELESTIALBODY_H
#define CELESTIALBODY_H

#include<iostream>
#include<armadillo>
#include<fstream>

using namespace std;
using namespace arma;

class CelestialBody
{
 public:

  CelestialBody();

  vec Position;
  vec Velocity;
  double mass;
  string name;

  CelestialBody( double x, double y, double z, double vx, double vy, double vz, double mass_, string name_);


};

#endif
