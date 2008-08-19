/* Siconos-sample version 3.0.0, Copyright INRIA 2005-2008.
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

#include "SiconosKernel.h"

using namespace std;

int main(int argc, char* argv[])
{
  try
  {


    // ================= Creation of the model =======================

    // -------------------------
    // --- Dynamical systems ---
    // -------------------------

    // Creation of a Dynamical System, First Order and Non Linear, like:
    // M \dot x = f(x,t,z)
    // x the state
    // z a vector of parameters (any)

    unsigned int n = 4; // Size of your system
    SimpleVector x0(4); // Initial conditions
    x0(1) = 2; // ...
    int number = 1; // An id number for the system


    string FPlugin = "myPlugins:myF";
    string jacobianFPlugin = "myPlugins:myJacobianF";

    FirstOrderNonLinearDS* myDS = new FirstOrderNonLinearDS(number, x0, FPlugin, jacobianFPlugin);

    // In this case M is equal to identity
    // If you need M:
    // SimpleMatrix M(size,size);
    // M(0,0) = 3; // and so on
    // myDS->setM(M);



    DynamicalSystemsSet allDS; // the list of all DS
    allDS.insert(myDS);


    // Creation of a non linear relation, like
    // y = h(x,z)
    // R = g(lamba,z)

    // Function to compute h
    string hName = "myPlugins:myH";
    // Function to compute g
    string gName = "myPlugins:myG";
    // function to compute nabla_x h
    string jacobianHName = "myPlugins:myJacobianH";
    // function to compute nabla_lambda g
    string jacobianGName = "myPlugins:myJacobianG";

    FirstOrderType1R * myRelation = new FirstOrderType1R(hName, gName, jacobianHName, jacobianGName);

    // A non-smooth law: complementarity between y and lambda
    int sizeNSL = 1; // size of y
    NonSmoothLaw * nslaw = new ComplementarityConditionNSL(sizeNSL);

    // The interaction which gathered the non-smooth law and the relation
    int sizeInter = sizeNSL; // size of the interaction.
    int interNumber = 1;// Id number for the interaction
    Interaction * inter = new Interaction("any-name", allDS, interNumber, sizeInter, nslaw, myRelation);

    // Set of all interactions
    InteractionsSet allInteractions;
    allInteractions.insert(inter);

    // --------------------------------
    // --- NonSmoothDynamicalSystem ---
    // --------------------------------
    // Building of the NSDS with all ds and all interactions
    NonSmoothDynamicalSystem * nsds = new NonSmoothDynamicalSystem(allDS, allInteractions);

    // -------------
    // --- Model ---
    // -------------

    double t0 = 0;                   // initial computation time
    double T = 10;                  // final computation time
    Model * modelName = new Model(t0, T);
    modelName->setNonSmoothDynamicalSystemPtr(nsds); // set NonSmoothDynamicalSystem of this model


    // From this point the NSDS model is complete.
    // It just needs to be initialized once the simulation parameters will be set.

    // ------------------
    // --- Simulation ---
    // ------------------

    double h = 0.005;                // default time step
    double theta = 0.5;              // theta for Moreau integrator


    // -- Time discretisation --
    TimeDiscretisation * t = new TimeDiscretisation(h, modelName);

    // About time discretisation:
    // h is a default value and can be change at any time during simulation loop.
    // one integration step is from ti to ti+h.
    // If you need to change h, call:
    // t->setCurrentTimeStep(newValue);
    // to get starting time integration:
    // s->getStartingTime();
    // to ending time:
    // s->getNextTime();
    // Warning: these two values are updated and incremented during call to nextStep()

    // The simulation:
    TimeStepping* s = new TimeStepping(t);

    // -- OneStepIntegrators --
    Moreau * OSI = new Moreau(allDS, theta, s);

    // -- OneStepNsProblem: the way the LCP or MLCP is solved

    // Solvers parameters may change according to the chosen solver.
    // Ask us for details
    IntParameters iparam(5);
    iparam[0] = 1000; // Max number of iteration
    DoubleParameters dparam(5);
    dparam[0] = 1e-15; // Tolerance
    string solverName = "Lemke";
    NonSmoothSolver * mySolver = new NonSmoothSolver(solverName, iparam, dparam);
    // -- OneStepNsProblem --
    OneStepNSProblem * osnspb = new LCP(s, mySolver);
    // --- Simulation initialization ---

    cout << "====> Simulation initialisation ..." << endl << endl;
    s->initialize();


    // Simulation loop
    while (s->getNextTime() < T)
    {
      s->computeOneStep();
      s->nextStep();
    }

  }

  catch (SiconosException e)
  {
    cout << e.report() << endl;
  }
  catch (...)
  {
    cout << "Exception caught in BouncingBallTS.cpp" << endl;
  }

}
