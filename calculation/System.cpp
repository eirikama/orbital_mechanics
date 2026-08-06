#include "System.h"

System::System() {
  dt = 0.00001;
  Bodycount = 0;
  G = 4 * M_PI * M_PI;
}

void System::AddBody(double x, double y, double z, double vx, double vy,
                     double vz, double mass_, string name_) {
  CelestialBody P = CelestialBody(x, y, z, vx, vy, vz, mass_, name_);
  ListofBodies[Bodycount] = P;
  Bodycount += 1;
}

void System::SolverSetup() {
  N = Bodycount;
  State.resize(6 * N);

  int c = 6;
  for (int i = 0; i < N; i++) {
    State(i * c + 0) = ListofBodies[i].Position(0);
    State(i * c + 1) = ListofBodies[i].Position(1);
    State(i * c + 2) = ListofBodies[i].Position(2);
    State(i * c + 3) = ListofBodies[i].Velocity(0);
    State(i * c + 4) = ListofBodies[i].Velocity(1);
    State(i * c + 5) = ListofBodies[i].Velocity(2);
  }
  cout << " " << endl;
  cout << "Initial State of our System" << endl;

  for (int i = 0; i < N; i++) {
    cout << " " << endl;
    cout << ListofBodies[i].name << " (" << ListofBodies[i].mass << " Ms)"
         << endl;
    cout << "(x,y,z): (" << ListofBodies[i].Position(0) << ", "
         << ListofBodies[i].Position(1) << ", " << ListofBodies[i].Position(2)
         << ") " << endl;
    cout << "(vx,vy,vz): (" << ListofBodies[i].Velocity(0) << ", "
         << ListofBodies[i].Velocity(1) << ", " << ListofBodies[i].Velocity(2)
         << ") " << endl;
  }
  cout << " " << endl;
};

vec System::NewtonianForce(const vec &B) {

  vec Force = zeros(3 * N);
  vec ki = zeros(6 * N);

  for (int j = 0; j < N; j++) {
    for (int k = j + 1; k < N; k++) {
      double x = B[6 * j] - B[6 * k];
      double y = B[6 * j + 1] - B[6 * k + 1];
      double z = B[6 * j + 2] - B[6 * k + 2];
      double r = sqrt(x * x + y * y + z * z);

      double f =
          -(G * ListofBodies[j].mass * ListofBodies[k].mass) / (r * r * r);
      x = f * x;
      y = f * y;
      z = f * z;
      Force[3 * j + 0] += x;
      Force[3 * j + 1] += y;
      Force[3 * j + 2] += z;
      Force[3 * k + 0] -= x;
      Force[3 * k + 1] -= y;
      Force[3 * k + 2] -= z;
    }
  }
  for (int j = 0; j < N; j++) {
    double m = ListofBodies[j].mass;
    ki[6 * j + 3] = Force[3 * j + 0] / m;
    ki[6 * j + 4] = Force[3 * j + 1] / m;
    ki[6 * j + 5] = Force[3 * j + 2] / m;
    ki[6 * j + 0] = B[6 * j + 3];
    ki[6 * j + 1] = B[6 * j + 4];
    ki[6 * j + 2] = B[6 * j + 5];
  }

  return ki;
};

void System::RK4() {
  vec k1(6 * N), k2(6 * N), k3(6 * N), k4(6 * N);
  k1 = NewtonianForce(State) * dt;
  k2 = NewtonianForce(State + 0.5 * k1) * dt;
  k3 = NewtonianForce(State + 0.5 * k2) * dt;
  k4 = NewtonianForce(State + k3) * dt;
  State += (1.0 / 6.0) * (k1 + 2.0 * (k2 + k3) + k4);
}

void System::RK4Evolve(double numberOfYears) {
  ofstream outFile("../data/RK4Data.bin", ios::binary);
  int numBodies = N;
  outFile.write(reinterpret_cast<char *>(&numBodies), sizeof(int));

  const int saveInterval = 4;
  const int subSteps =
      10; // 50 mini-steps per global step is plenty for the Moon
  const double dt_sub = dt / subSteps;
  const double step_fraction =
      1.0 / subSteps; // How much alpha increases per mini-step
  const int moonIdx = 4;

  double t = 0;
  int stepCounter = 0;

  while (t < numberOfYears) {
    vec prevState = State; // Save state before anything moves

    // 1. GLOBAL STEP (Moves Earth/Sun to the future)
    RK4();
    vec nextState = State;

    // 2. REWIND THE MOON (Bring it back to the past)
    for (int i = 0; i < 6; i++)
      State[6 * moonIdx + i] = prevState[6 * moonIdx + i];

    // 3. SUB-STEP THE MOON (Smoothly move it to the future)
    for (int s = 0; s < subSteps; s++) {
      double current_alpha = (double)s / subSteps;
      RefineBodyRK4(moonIdx, dt_sub, current_alpha, step_fraction, prevState,
                    nextState);
    }

    // 4. SAVE TO FILE
    if (stepCounter % saveInterval == 0) {
      outFile.write(reinterpret_cast<char *>(State.memptr()),
                    State.n_elem * sizeof(double));
    }

    t += dt;
    stepCounter++;
  }
  outFile.close();
}

vec System::CalculateInterpolatedAcc(int targetIdx, vec pos, double alpha,
                                     const vec &prevState,
                                     const vec &nextState) {
  vec acc = zeros(3);

  for (int i = 0; i < N; i++) {
    if (i == targetIdx)
      continue;

    // Linearly interpolate the background body's position between START and END
    // of global step
    double otherX =
        prevState[6 * i + 0] * (1.0 - alpha) + nextState[6 * i + 0] * alpha;
    double otherY =
        prevState[6 * i + 1] * (1.0 - alpha) + nextState[6 * i + 1] * alpha;
    double otherZ =
        prevState[6 * i + 2] * (1.0 - alpha) + nextState[6 * i + 2] * alpha;

    double dx = otherX - pos[0];
    double dy = otherY - pos[1];
    double dz = otherZ - pos[2];

    double r2 = dx * dx + dy * dy + dz * dz +
                1e-15; // Softening to prevent division by zero
    double r = sqrt(r2);

    // Newtonian Gravity: a = G * M / r^3 * dist_vector
    double magnitude = (G * ListofBodies[i].mass) / (r2 * r);

    acc[0] += magnitude * dx;
    acc[1] += magnitude * dy;
    acc[2] += magnitude * dz;
  }
  return acc;
}

void System::RefineBodyRK4(int index, double small_dt, double alpha,
                           double step_fraction, const vec &prevState,
                           const vec &nextState) {
  vec p0 = {State[6 * index + 0], State[6 * index + 1], State[6 * index + 2]};
  vec v0 = {State[6 * index + 3], State[6 * index + 4], State[6 * index + 5]};

  // Stage 1 (at current alpha)
  vec kv1 = CalculateInterpolatedAcc(index, p0, alpha, prevState, nextState) *
            small_dt;
  vec kp1 = v0 * small_dt;

  // Stage 2 (at half small_dt)
  double alpha_mid = alpha + 0.5 * step_fraction;
  vec kv2 = CalculateInterpolatedAcc(index, p0 + 0.5 * kp1, alpha_mid,
                                     prevState, nextState) *
            small_dt;
  vec kp2 = (v0 + 0.5 * kv1) * small_dt;

  // Stage 3 (at half small_dt)
  vec kv3 = CalculateInterpolatedAcc(index, p0 + 0.5 * kp2, alpha_mid,
                                     prevState, nextState) *
            small_dt;
  vec kp3 = (v0 + 0.5 * kv2) * small_dt;

  // Stage 4 (at full small_dt)
  double alpha_end = alpha + step_fraction;
  vec kv4 = CalculateInterpolatedAcc(index, p0 + kp3, alpha_end, prevState,
                                     nextState) *
            small_dt;
  vec kp4 = (v0 + kv3) * small_dt;

  // Update Moon's state in the global 'State' array
  for (int i = 0; i < 3; i++) {
    State[6 * index + i] +=
        (1.0 / 6.0) * (kp1[i] + 2.0 * kp2[i] + 2.0 * kp3[i] + kp4[i]);
    State[6 * index + i + 3] +=
        (1.0 / 6.0) * (kv1[i] + 2.0 * kv2[i] + 2.0 * kv3[i] + kv4[i]);
  }
}

void System::VerletEvolve(double numberOfYears) {
  cout << "[To be evolved in time-steps of " << dt << " for " << numberOfYears
       << " years by Verlet.]" << endl;

  prevState = State;
  RK4();

  fstream outFile;
  outFile.open("VerletData.dat", ios::out);
  double t = 0;
  while (t < numberOfYears) {

    vec acc = zeros(6 * N);
    vec force = NewtonianForce(State);

    for (int j = 0; j < N; j++) {
      acc[6 * j + 0] = force[6 * j + 3];
      acc[6 * j + 1] = force[6 * j + 4];
      acc[6 * j + 2] = force[6 * j + 5];
    }

    nextState = 2 * State - prevState + acc * dt * dt;

    for (int j = 0; j < N; j++) {
      State[6 * j + 3] =
          (nextState[6 * j + 0] - prevState[6 * j + 0]) / (2 * dt);
      State[6 * j + 4] =
          (nextState[6 * j + 1] - prevState[6 * j + 1]) / (2 * dt);
      State[6 * j + 5] =
          (nextState[6 * j + 2] - prevState[6 * j + 2]) / (2 * dt);
    }

    prevState = State;
    t += dt;

    for (int i = 0; i < N; i++) {
      outFile << State(6 * i + 0) << " " << State(6 * i + 1) << " "
              << State(6 * i + 3) << " " << State(6 * i + 4) << " ";
    }
    outFile << endl;

    State = nextState;
  }

  outFile.close();
};

vec System::RelativisticForce(const vec &B) {

  vec Force = zeros(6 * N);
  vec ki = zeros(6 * N);

  for (int j = 0; j < N; j++) {
    for (int k = j + 1; k < N; k++) {
      double x = B[6 * j] - B[6 * k];
      double y = B[6 * j + 1] - B[6 * k + 1];
      double z = B[6 * j + 2] - B[6 * k + 2];
      double vx = B[6 * j + 3] - B[6 * k + 3];
      double vy = B[6 * j + 4] - B[6 * k + 4];
      double vz = B[6 * j + 5] - B[6 * k + 5];
      double v = sqrt(vx * vx + vy * vy + vz * vz);
      double r = sqrt(x * x + y * y + z * z);
      double c = 173.1 * 365;

      double l = r * v * sin(atan(y / x));

      double f =
          -((G * ListofBodies[j].mass * ListofBodies[k].mass) / (r * r * r)) *
          (1 + ((3 * l * l) / (r * r * c * c)));
      x = f * x;
      y = f * y;
      z = f * z;
      Force[3 * j + 0] += x;
      Force[3 * j + 1] += y;
      Force[3 * j + 2] += z;
      Force[3 * k + 0] -= x;
      Force[3 * k + 1] -= y;
      Force[3 * k + 2] -= z;
    }
  }
  for (int j = 0; j < N; j++) {
    double m = ListofBodies[j].mass;
    ki[6 * j + 3] = Force[3 * j + 0] / m;
    ki[6 * j + 4] = Force[3 * j + 1] / m;
    ki[6 * j + 5] = Force[3 * j + 2] / m;
    ki[6 * j + 0] = B[6 * j + 3];
    ki[6 * j + 1] = B[6 * j + 4];
    ki[6 * j + 2] = B[6 * j + 5];
  }

  return ki;
};
