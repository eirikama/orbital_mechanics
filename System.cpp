 #include "System.h"

System::System()
{
  dt = 0.001;
  Bodycount = 0;
  G = 4*M_PI*M_PI;
}

void System::AddBody( double x, double y, double z, double vx, double vy, double vz, double mass_, string name_)
{
  CelestialBody P = CelestialBody(x, y, z, vx, vy, vz, mass_, name_);
  ListofBodies[Bodycount] = P;
  this->Bodycount += 1;
}

void System::SolverSetup()
{
 this->N = this->Bodycount;
 this->State = zeros(6*N);

int c = 6;
for (int i = 0; i < N; i++) 
{
  this->State(i*c + 0) = this->ListofBodies[i].Position(0);
  this->State(i*c + 1) = this->ListofBodies[i].Position(1);
  this->State(i*c + 2) = this->ListofBodies[i].Position(2);
  this->State(i*c + 3) = this->ListofBodies[i].Velocity(0);
  this->State(i*c + 4) = this->ListofBodies[i].Velocity(1);
  this->State(i*c + 5) = this->ListofBodies[i].Velocity(2);
}
 cout << " " <<endl;
cout << "Initial State of our System" << endl;

for (int i = 0; i<N; i++)
   {
cout << " " <<endl;
cout << ListofBodies[i].name << " (" <<ListofBodies[i].mass << " Ms)" <<  endl;
cout << "(x,y): (" << ListofBodies[i].Position(0) << ", " << ListofBodies[i].Position(1) << ", " << ListofBodies[i].Position(2) << ") " << endl;
 cout << "(vx,vy): (" << ListofBodies[i].Velocity(0) << ", " << ListofBodies[i].Velocity(1) <<  ", " << ListofBodies[i].Velocity(2) << ") " <<  endl;
   }
 cout << " " <<endl;
};




vec System::NForce(vec B)
{

 vec Force = zeros(3 * N);
 vec ki = zeros(6 * N);

for (int j = 0; j < N; j++) {
  for (int k = j+1; k < N; k++) {
    double x = B[6*j] - B[6*k];
    double y = B[6*j+1] - B[6*k+1];
    double z = B[6*j+2] - B[6*k+2];
    double r = sqrt(x*x + y*y + z*z);

    double f = -(G*ListofBodies[j].mass*ListofBodies[k].mass)/(r*r*r);
    x = f*x;
    y = f*y;
    z = f*z;
    Force[3*j+0] += x;
    Force[3*j+1] += y;
    Force[3*j+2] += z;                             
    Force[3*k+0] -= x; 
    Force[3*k+1] -= y;
    Force[3*k+2] -= z;
  }

}
for (int j = 0; j < N; j++)
 {
  double m = ListofBodies[j].mass;
  ki[6*j+3] = Force[3*j+0] / m;
  ki[6*j+4] = Force[3*j+1] / m;
  ki[6*j+5] = Force[3*j+2] / m;
  ki[6*j+0] = B[6*j+3]; 
  ki[6*j+1] = B[6*j+4];
  ki[6*j+2] = B[6*j+5];
 }

return ki;

};

void System::RK4()
{
vec k1(6*N), k2(6*N), k3(6*N), k4(6*N);
k1 = NForce(State) * dt;
k2 = NForce(State + 0.5 * k1) * dt;
k3 = NForce(State + 0.5 * k2) * dt;
k4 = NForce(State + k3) * dt;
State += (1.0/6) * (k1 + 2 * (k2 + k3) + k4);


}


void System::RK4Evolve(double numberOfYears) 
{ 
fstream outFile;
outFile.open("RK4Data.dat", ios::out);
cout << "[To be evolved in time-steps of " <<this->dt<< " for " << numberOfYears << " years by 4. order Runge Kutta.]" << endl;

double t = 0; 

while (t < numberOfYears) {
  RK4();

  t+=dt;
  for (int i=0;i<N; i++) 
  { 
 
outFile << this->State(6*i+0) << " " << this->State(6*i+1) << " "<< this->State(6*i+3) << " " << this->State(6*i+4) << " ";

  }
outFile << endl;
}
outFile.close();
};



void System::VerletEvolve(double numberOfYears) 
{ 
  cout << "[To be evolved in time-steps of " <<this->dt<< " for " << numberOfYears << " years by Verlet.]" << endl;

  prevState = State;
  RK4();

  fstream outFile;
  outFile.open("VerletData.dat", ios::out);
  double t = 0; 
  while(t<numberOfYears)
    {
      
      vec acc = zeros(6*N);
      vec force = NForce(State);

    for (int j = 0; j < N; j++)
    {
      acc[6*j+0] = force[6*j+3]; 
      acc[6*j+1] = force[6*j+4];
      acc[6*j+2] = force[6*j+5];
    }
   
    nextState = 2*State - prevState + acc*dt*dt;

    for (int j = 0; j < N; j++)
    {
      State[6*j+3] = (nextState[6*j+0] - prevState[6*j+0])/(2*dt); 
      State[6*j+4] = (nextState[6*j+1] - prevState[6*j+1])/(2*dt);
      State[6*j+5] = (nextState[6*j+2] - prevState[6*j+2])/(2*dt);
    }
  
      prevState = State;
      t += dt;

      for(int i=0; i<N;i++)
	{
        outFile << State(6*i+0) << " " << State(6*i+1) << " " << State(6*i+3) << " " << State(6*i+4) << " ";
        }
        outFile << endl;

     State = nextState;
     }

outFile.close();
};   

vec System::RForce(vec B)
{

 vec Force = zeros(6 * N);
 vec ki = zeros(6 * N);

for (int j = 0; j < N; j++) {
  for (int k = j+1; k < N; k++) {
    double x = B[6*j] - B[6*k];
    double y = B[6*j+1] - B[6*k+1];
    double z = B[6*j+2] - B[6*k+2];
    double vx = B[6*j+3] - B[6*k+3];
    double vy = B[6*j+4] - B[6*k+4];
    double vz = B[6*j+5] - B[6*k+5];
    double v = sqrt(vx*vx + vy*vy +vz*vz);
    double r = sqrt(x*x + y*y +z*z);
    double c = 173.1*365;

    double l = r*v*sin(atan(y/x));

    double f = -((G*ListofBodies[j].mass*ListofBodies[k].mass)/(r*r*r))*(1 + ( (3*l*l)/(r*r*c*c) ));
    x = f*x;
    y = f*y;
    z = f*z;
    Force[3*j+0] += x;
    Force[3*j+1] += y; 
    Force[3*j+2] += z;                                                           
    Force[3*k+0] -= x; 
    Force[3*k+1] -= y; 
    Force[3*k+2] -= z;
}
}
for (int j = 0; j < N; j++)
 {
  double m = ListofBodies[j].mass;
  ki[6*j+3] = Force[3*j+0] / m;
  ki[6*j+4] = Force[3*j+1] / m;
  ki[6*j+5] = Force[3*j+2] / m;
  ki[6*j+0] = B[6*j+3]; 
  ki[6*j+1] = B[6*j+4];
  ki[6*j+2] = B[6*j+5];
 }

return ki;

};  


