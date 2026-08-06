#ifndef SYSTEM_H
#define SYSTEM_H

#include "CelestialBody.h"
#include <armadillo>
#include <cmath>
#include <fstream>

using namespace std;

class System {
public:
  double G;
  double dt;
  vec State;
  vec prevState, nextState;

  int Bodycount;
  int N;
  CelestialBody ListofBodies[50];

  System();

  void AddBody(double, double, double, double, double, double, double, string);
  vec NewtonianForce(const vec &);
  vec RelativisticForce(const vec &);
  void SolverSetup();
  void RK4();
  void RK4Evolve(double);
  void VerletEvolve(double);
  void Energy(vec);
  vec CalculateInterpolatedAcc(int targetIdx, vec pos, double alpha,
                               const vec &prevState, const vec &nextState);

  // Moves a specific body forward by small_dt using RK4 with interpolation
  void RefineBodyRK4(int index, double small_dt, double current_alpha,
                     double step_fraction, const vec &prevState,
                     const vec &nextState);
};

#endif
