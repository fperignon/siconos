/* Siconos is a program dedicated to modeling, simulation and control
 * of non smooth dynamical systems.
 *
 * Copyright 2016 INRIA.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
/*! \file
  Fricton-Contact Non-Smooth Problem
*/
#ifndef FrictionContact_H
#define FrictionContact_H

#include "LinearOSNS.hpp"

#include <FrictionContactProblem.h>
#include <Friction_cst.h>
/** Pointer to function of the type used for drivers for FrictionContact problems in Numerics */
typedef int (*Driver)(FrictionContactProblem*, double*, double*, SolverOptions*, NumericsOptions*);
TYPEDEF_SPTR(FrictionContactProblem)

/** Formalization and Resolution of a Friction-Contact Problem

   \author SICONOS Development Team - copyright INRIA
   \version 3.0.0.
   \date (Creation) Dec 15, 2005

  This class is devoted to the formalization and the resolution of
  friction contact problems defined by :
  \f{eqnarray}
  velocity =  q + M reaction \\
  \\
  velocity \geq 0, reaction \geq 0,  reaction^{T} velocity =0
  \f}
  and a Coulomb friction law.

  With:
     - \f$velocity \in R^{n} \f$  and \f$reaction \in R^{n} \f$ the unknowns,
     - \f$M \in R^{n \times n } \f$  and \f$q \in R^{n} \f$

  The dimension of the problem (2D or 3D) is given by the variable contactProblemDim and the proper
  Numerics driver will be called according to this value.

  \b Construction: just set Numerics Solver id

  Main functions:

  \b Usage:
  - compute(time) formalize, solve and post-process the problem.

  pre- and post-pro are common to all LinearOSNS and defined in this class.


 */
class FrictionContact : public LinearOSNS
{
protected:
  /** serialization hooks
  */
  ACCEPT_SERIALIZATION(FrictionContact);


  /** Type (dimension) of the contact problem (2D or 3D) */
  int _contactProblemDim;

  /** * friction coefficients */
  SP::MuStorage _mu;

  /** Pointer to the function used to call the Numerics driver to solve the problem */
  Driver _frictionContact_driver;

  FrictionContactProblem _numerics_problem;

public:

  /**
     \param dimPb dimension (2D or 3D) of the FrictionContact problem (default = 3D)
     \param numericsSolverId Numerics solver to use (default = NSGS)
  */
  FrictionContact(int dimPb = 3, int numericsSolverId = SICONOS_FRICTION_3D_NSGS);

  /** destructor
   */
  virtual ~FrictionContact();

  // GETTERS/SETTERS

  /** get the type of FrictionContact problem (2D or 3D)
   *  \return an int (2 or 3)
   */
  inline int getFrictionContactDim() const
  {
    return _contactProblemDim;
  }

  // --- Mu ---
  /** get the vector mu, list of the friction coefficients
   *  \return a vector of double
   */
  inline const MuStorage getMu() const
  {
    return *_mu;
  }

  /** get a pointer to mu, the list of the friction coefficients
   *  \return pointer on a std::vector<double>
   */

  inline SP::MuStorage mu() const
  {
    return _mu;
  }

  /** get the value of the component number i of mu, the vector of the friction coefficients
   *  \param i the component number (starting from 0)
   *  \return double value of mu
   */
  inline double getMu(unsigned int i) const
  {
    return (*_mu)[i];
  }

  /** update mu vector
   */
  void updateMu();

  /** set the driver-function used to solve the problem
      \param newFunction function of prototype Driver
  */
  inline void setNumericsDriver(Driver newFunction)
  {
    _frictionContact_driver = newFunction;
  };

  // --- Others functions ---

  /** initialize the FrictionContact problem(compute topology ...)
      \param simulation the simulation, owner of this OSNSPB
   */
  virtual void initialize(SP::Simulation simulation);

  /**
   * \return the friction contact problem from Numerics
   */
  SP::FrictionContactProblem frictionContactProblem();

  /**
   * \return the friction contact problem from Numerics (raw ptr, do not free)
   */
  FrictionContactProblem *frictionContactProblemPtr();

  /** solve a friction contact problem
   * \param problem the friction contact problem
   * \return info solver information result
   */
  int solve(SP::FrictionContactProblem problem = SP::FrictionContactProblem());

  /** Compute the unknown reaction and velocity and update the Interaction (y and lambda )
   *  \param time the current time
   *  \return int information about the solver convergence (0: ok, >0 problem, see Numerics documentation)
   */
  virtual int compute(double time);

  /** print the data to the screen */
  void display() const;

  /** visitors hook
   */
  ACCEPT_STD_VISITORS();

};

#endif // FrictionContact_H
