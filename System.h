#ifndef SYSTEM_H
#define SYSTEM_H

#include "CelestialBody.h"
#include<armadillo>
#include<cmath>
#include<fstream>

using namespace std;

class System
{
 public:
  double G;
  double dt;
  vec State;
  vec prevState, nextState;

  int Bodycount;
  int N;
  CelestialBody ListofBodies[50];
  
  System();

  void AddBody( double, double, double, double, double, double, double, string);
  vec NForce(vec);
  vec RForce(vec);
  void SolverSetup();
  void RK4();
  void RK4Evolve(double);
  void VerletEvolve(double);
  void Energy(vec);
  
  
};

#endif
