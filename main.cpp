#include "CelestialBody.h"
#include "System.h"

int main()
{
  System SolarSystem;
  
  // Masses in units of Solar masses 

  double M_Ea = 3e-6;
  double M_Me = 0.0055 * M_Ea;
  double M_Ve = 0.815  * M_Ea;
  double M_Ma = 0.107  * M_Ea;
  double M_Ju = 317.8  * M_Ea;
  double M_Sa = 95.152 * M_Ea;
  double M_Ur = 14.536 * M_Ea;
  double M_Ne = 17.147 * M_Ea;
  double M_Pl = 0.00218 * M_Ea;
  double M_Ha = 3.6837e-11 * M_Ea;

  // Initial positions and velocities in km and km/s ( convert to our prefered units later)
 
  double vxMe = 2.525822913860079E+01,  vyMe =  3.627931518613809E+01, vzMe = 6.469242537532837E-01;
  double vxVe = -4.838066659924075E+00, vyVe = -3.486533967772683E+01, vzVe = -1.986418837879511E-01;
  double vxEa = -4.406220350009290E+00, vyEa =  2.941898165409254E+01, vzEa = -1.333034560677749E-03;
  double vxMa = 2.355273541766074E+01,  vyMa = 1.072888188164977E+01, vzMa = -3.532953957527511E-01;
  double vxJu = -1.057863541952222E+01, vyJu = -7.282677815690089E+00, vzJu = 2.669653552828916E-01;
  double vxSa = 7.305583007781340E+00,  vySa = -5.678067175058013E+00, vzSa = -1.921159722403775E-01;
  double vxUr = -1.730861423436294E+00, vyUr = 6.275826934901269E+00, vzUr = 4.597420467011978E-02;
  double vxNe = 2.152788365068528E+00,  vyNe = 4.996563864711828E+00, vzNe = -1.527584614750138E-01;
  double vxPl = 5.389758769831626E+00,  vyPl = 5.017824639329278E-02, vzPl = -1.578243168379842E+00;
  double vxHa = -1.236183883191173E-01, vyHa =  1.621369229744805E+00, vzHa = -3.015245553639735E-01;

  double xMe = 4.325741590616594E+07, yMe = -4.447598205227128E+07, zMe = -7.602793726150981E+06; 
  double xVe = -1.065184862973610E+08, yVe = 1.426409267960166E+07, zVe = 6.342590226882618E+06;
  double xEa = 1.484997959420039E+08, yEa = 1.966516392815287E+07, zEa = -1.414904880365577E+03;
  double xMa = 7.561157938277872E+07, yMa = -1.979342346107423E+08, zMa = -6.003106128253528E+06; 
  double xJu = -4.781071082144656E+08, yJu =  6.306239474601380E+08, zJu = 8.079395665674826E+06;
  double xSa = -8.681948970465746E+08, ySa = -1.205449819868287E+09, zSa = 5.552046389307594E+07; 
  double xUr = 2.902003974098086E+09, yUr =  7.358295383909013E+08, zUr = -3.484989767043513E+07;
  double xNe = 4.101038351114639E+09, yNe = -1.811465478741618E+09, zNe =  -5.719412717146279E+07;
  double xPl = 1.063767728198170E+09, yPl = -4.775732770106762E+09, zPl = 2.032610886473450E+08;
  double xHa = -3.061126454498667E+09, yHa = 3.760634554053051E+09, zHa = -1.461631754870368E+09;

  // Keeping the sum of momentum zero  

  double vxSun = -(M_Me*vxMe + M_Ve*vxVe + M_Ea*vxEa + M_Ma*vxMa + M_Ju*vxJu + M_Sa*vxSa + M_Ne*vxNe + M_Ur*vxUr + M_Pl*vxPl + vxHa*M_Ha);
  double vySun = -(M_Me*vyMe + M_Ve*vyVe + M_Ea*vyEa + M_Ma*vyMa + M_Ju*vyJu + M_Sa*vySa + M_Ne*vyNe + M_Ur*vyUr + M_Pl*vyPl + M_Ha*vyHa);
  double vzSun = -(M_Me*vzMe + M_Ve*vzVe + M_Ea*vzEa + M_Ma*vzMa + M_Ju*vzJu + M_Sa*vzSa + M_Ne*vzNe + M_Ur*vzUr + M_Pl*vzPl + M_Ha*vzHa);

  SolarSystem.AddBody(0,0,0,vxSun,vySun,vzSun,1, "Sun");
  //SolarSystem.AddBody(0,0,0,0,0,0,1, "Sun");
  SolarSystem.AddBody( xMe, yMe, zMe, vxMe, vyMe, vzMe, M_Me, "Mercury");
  SolarSystem.AddBody( xVe, yVe, zVe, vxVe, vyVe, vzVe, M_Ve, "Venus");
  SolarSystem.AddBody( xEa, yEa, zEa, vxEa, vyEa, vzEa, M_Ea, "Earth");
  SolarSystem.AddBody( xMa, yMa, zMa, vxMa, vyMa, vzMa, M_Ma, "Mars");
  SolarSystem.AddBody( xJu, yJu, zJu, vxJu, vyJu, vzJu, M_Ju, "Jupiter");
  SolarSystem.AddBody( xSa, ySa, zSa, vxSa, vySa, vzSa, M_Sa, "Saturn");
  SolarSystem.AddBody( xUr, yUr, zUr, vxUr, vyUr, vzUr, M_Ur, "Uranus");
  SolarSystem.AddBody( xNe, yNe, zNe, vxNe, vyNe, vzNe, M_Ne, "Neptune");  
  SolarSystem.AddBody( xPl, yPl, zPl, vxPl, vyPl, vzPl, M_Pl, "Pluto");
  SolarSystem.AddBody( xHa, yHa, zHa, vxHa, vyHa, vzHa, M_Ha, "Halley's Comet");

  SolarSystem.SolverSetup();
  
  SolarSystem.RK4Evolve(248);
}
