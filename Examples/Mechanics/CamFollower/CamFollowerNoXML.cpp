/* Siconos-Examples version 3.0.0, Copyright INRIA 2005-2008.
 * Siconos is a program dedicated to modeling, simulation and control
 * of non smooth dynamical systems.
 * Siconos is a free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * Siconos is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Siconos; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Contact: Vincent ACARY vincent.acary@inrialpes.fr
*/

/*!\file CamFollowerNoXML.cpp
\brief \ref EMCamFollower - C++ input file version - M. di Bernardo, G. Osorio, S. Santini.
*/

#include "SiconosKernel.hpp"
#include "CamState.h"

using namespace std;

int main(int argc, char* argv[])
{
  double rpm = 358;
  try
  {

    // ================= Creation of the model =======================

    // User-defined main parameters
    unsigned int dsNumber = 1;       // the Follower and the ground
    unsigned int nDof = 1;           // degrees of freedom for the ball
    double t0 = 0;                   // initial computation time
    double T = 1;                   // final computation time
    double h = 0.0001;                // time step
    double position_init = 0.;//;40;      // initial position for lowest bead.
    double velocity_init = 0.;//4;      // initial velocity for lowest bead.
    double theta = 0.5;              // theta for Moreau integrator
    // -------------------------
    // --- Dynamical systems ---
    // -------------------------

    SP::SimpleMatrix Mass(new SimpleMatrix(nDof, nDof));
    SP::SimpleMatrix K(new SimpleMatrix(nDof, nDof));
    SP::SimpleMatrix C(new SimpleMatrix(nDof, nDof));       // mass/rigidity/viscosity
    (*Mass)(0, 0) = 1.221;
    (*K)(0, 0) = 1430.8;

    // -- Initial positions and velocities --
    vector<SP::SimpleVector> q0;
    vector<SP::SimpleVector> velocity0;
    q0.resize(dsNumber);
    velocity0.resize(dsNumber);
    q0[0].reset(new SimpleVector(nDof));
    velocity0[0].reset(new SimpleVector(nDof));
    (*(q0[0]))(0) = position_init;
    (*(velocity0[0]))(0) = velocity_init;
    SP::LagrangianLinearTIDS lds(new LagrangianLinearTIDS(q0[0], velocity0[0], Mass, K, C));
    lds->setComputeFExtFunction("FollowerPlugin.so", "FollowerFExtR");

    // Example to set a list of parameters in FExt function.
    // 1 - Create a simple vector that contains the required parameters.
    SP::SiconosVector param(new SimpleVector(1)); // Here we only set one parameter, the DS number.
    //    (*param)(0) = vectorDS[0]->getNumber();
    (*param)(0) = rpm;
    // 2 - Assign this param to the function FExt
    lds->setZPtr(param);
    // 2 corresponds to the position of FExt in the stl vector of possible parameters. 0 is mass, 1 FInt and so on.
    // Now the DS number will be available in FExt plugin.

    DynamicalSystemsSet allDS;
    allDS.insert(lds);

    // --------------------
    // --- Interactions ---
    // --------------------

    // -- nslaw --
    double e = 0.8;

    // Interaction Follower-floor
    //
    SimpleMatrix H(1, nDof);
    H(0, 0) = 1.0;
    SP::NonSmoothLaw nslaw0(new NewtonImpactNSL(e));
    SP::Relation relation0(new LagrangianLinearTIR(H));
    DynamicalSystemsSet dsConcerned;
    dsConcerned.insert(lds);

    SP::Interaction inter(new Interaction("Follower-Ground", dsConcerned, 0, 1, nslaw0, relation0));
    InteractionsSet allInteractions;
    allInteractions.insert(inter);

    // -------------
    // --- Model ---
    // -------------

    SP::Model Follower(new Model(t0, T, allDS, allInteractions));

    // ----------------
    // --- Simulation ---
    // ----------------

    // -- Time discretisation --
    SP::TimeDiscretisation t(new TimeDiscretisation(t0, h));

    SP::TimeStepping S(new TimeStepping(t));


    // -- OneStepIntegrator --
    SP::OneStepIntegrator OSI(new Moreau(lds, theta));
    S->insertIntegrator(OSI);

    // -- OneStepNsProblem --
    IntParameters iparam(5);
    iparam[0] = 101; // Max number of iteration
    DoubleParameters dparam(5);
    dparam[0] = 1e-5; // Tolerance
    string solverName = "QP" ;
    SP::NonSmoothSolver mySolver(new NonSmoothSolver(solverName, iparam, dparam));
    SP::OneStepNSProblem osnspb(new LCP(mySolver));
    S->insertNonSmoothProblem(osnspb);

    cout << "=== End of model loading === " << endl;
    // =========================== End of model definition ===========================

    // ================================= Computation =================================

    // --- Simulation initialization ---
    Follower->initialize(S);
    cout << "End of model initialisation" << endl;


    int k = 0;
    int N = (int)((T - t0) / h); // Number of time steps


    // --- Get the values to be plotted ---
    // -> saved in a matrix dataPlot
    unsigned int outputSize = 8;
    SimpleMatrix DataPlot(N + 1, outputSize);
    // For the initial time step:
    // time
    DataPlot(k, 0) = t0;
    DataPlot(k, 1) = (*lds->q())(0);
    DataPlot(k, 2) = (*lds->velocity())(0);
    DataPlot(k, 3) = (*Follower->nonSmoothDynamicalSystem()->topology()->interactions()->getPtr(0)->lambda(1))(0);
    DataPlot(k, 4) = (*lds->fExt())(0);

    // State of the Cam
    //    double rpm=358;
    double CamEqForce, CamPosition, CamVelocity, CamAcceleration;

    CamEqForce = CamState(t0, rpm, CamPosition, CamVelocity, CamAcceleration);
    // Position of the Cam
    DataPlot(k, 5) = CamPosition;
    // Velocity of the Cam
    DataPlot(k, 6) = CamVelocity;
    // Acceleration of the Cam
    DataPlot(k, 7) = CamPosition + (*lds->q())(0);
    boost::timer tt;
    tt.restart();
    // --- Time loop ---
    cout << "Start computation ... " << endl;
    while (k < N)
    {
      // get current time step
      k++;
      // solve ...
      S->computeOneStep();

      // --- Get values to be plotted ---

      DataPlot(k, 0) = S->nextTime();
      DataPlot(k, 1) = (*lds->q())(0);
      DataPlot(k, 2) = (*lds->velocity())(0);
      DataPlot(k, 3) = (*Follower->nonSmoothDynamicalSystem()->topology()->interactions()->getPtr(0)->lambda(1))(0);
      DataPlot(k, 4) = (*lds->fExt())(0);

      CamEqForce = CamState(S->nextTime(), rpm, CamPosition, CamVelocity, CamAcceleration);
      DataPlot(k, 5) = CamPosition;
      DataPlot(k, 6) = CamVelocity;
      DataPlot(k, 7) = CamPosition + (*lds->q())(0);
      // transfer of state i+1 into state i and time incrementation
      S->nextStep();
    }
    // --- Output files ---
    ioMatrix io("result.dat", "ascii");
    io.write(DataPlot, "noDim");
    cout << "time = " << tt.elapsed() << endl;
    cout << "End of computation - Number of iterations done: " << k << endl;
  }

  catch (SiconosException e)
  {
    cout << e.report() << endl;
  }
  catch (...)
  {
    cout << "Exception caught in \'sample/CamFollower\'" << endl;
  }
}
