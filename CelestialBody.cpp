#include "CelestialBody.h"

CelestialBody::CelestialBody(){}

CelestialBody::CelestialBody( double x, double y, double z, double vx, double vy, double vz, double mass_, string name_)
{
  Position.set_size(3);
  Velocity.set_size(3);
  Position(0) = x/149597871;
  Position(1) = y/149597871;
  Position(2) = z/149597871; 
  Velocity(0) = (vx/149597871)*3600*24*365.25;
  Velocity(1) = (vy/149597871)*3600*24*365.25;
  Velocity(2) = (vz/149597871)*3600*24*365.25;
  mass = mass_;
  name = name_;
  
}
