/* Siconos-Kernel, Copyright INRIA 2005-2012.
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
 * Contact: Vincent ACARY, siconos-team@lists.gforge.inria.fr
*/

/*! \file Topology.hpp

*/
#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include "SiconosConst.hpp"
#include "SimulationTypeDef.hpp"

class NonSmoothDynamicalSystem;
class Interaction;
class DynamicalSystem;
class SiconosMatrix;
class Interaction;


/**  This class describes the topology of the non-smooth dynamical
 *  system. It holds all the "potential" Interactions".
 *
 *  \author SICONOS Development Team - copyright INRIA
 *  \version 3.0.0.
 *  \date (Creation) July 20, 2005
 *
 *  Topology is built in NSDS constructors but initialized in
 *  Simulation->initialize(), ie when all Interactions have been
 *  clearly defined.
 *
 * Note that indexSet0 holds all the possible relations (declared by
 * user) not only those which are "actives".
 *
 * Construction consists in:
 *    - link with the NSDS that owns the topology.
 *
 * Initialization consists in:
 *    - scan of all the interactions of the NSDS
 *    - initialization of each interaction
 *    - insertion of the relations of all the Interaction into indexSet0
 *
 * Insertion of an Interaction into the set indexSet0:
 * addInteractionInIndexSet(SP::Interaction inter) for each relation
 * of the interaction, it creates a new Interaction and inserts it
 * into indexSet0 It also counts the total number of "constraints" in
 * the system.
 *
 */
class Topology : public std11::enable_shared_from_this<Topology>
{

private:
  /** serialization hooks
  */
  ACCEPT_SERIALIZATION(Topology);


  // --- MEMBERS ---
  /** dynamical systems graphs */
  std::vector<SP::DynamicalSystemsGraph> _DSG;

  /** Interaction graphs (_IG[0]=L[DSG[0]], L is the line graph
      transformation) */
  std::vector<SP::InteractionsGraph> _IG;

  /** check if topology has been updated since nsds modifications
      occur */
  bool _isTopologyUpToDate;

  /** check if topology is static or  not */
  bool _hasChanged;

  /** Total number of (scalar) constraints in the problem, ie sum of
      all nslaw sizes of Interactions of IndexSet0.*/
  unsigned int _numberOfConstraints;

  /** symmetry in the blocks computation */
  bool _symmetric;

  /** initializations ( time invariance) from non
      smooth laws kind */
  struct SetupFromNslaw;
  friend struct Topology::SetupFromNslaw;

  // === PRIVATE FUNCTIONS ===

  /** schedules the relations of Interaction inter into IndexSet0 (ie
  * creates the corresponding Interactions and add them into _DSG
  * and _IG)
  \param  inter the Interaction to add
  \param SP::DynamicalSystem first dynamical system linked to the interaction
  \param SP::DynamicalSystem second dynamical system linked to the interaction (default = None)
  \return a vertex descriptor of the new vertex in IndexSet0
  */
  std::pair<DynamicalSystemsGraph::EDescriptor, InteractionsGraph::VDescriptor> 
  addInteractionInIndexSet(SP::Interaction inter, SP::DynamicalSystem, SP::DynamicalSystem = SP::DynamicalSystem());

  /** remove the Interactions of the interactions from _IG and
   * _DSG
   * \param inter the Interaction to remove
   */
  void removeInteractionFromIndexSet(SP::Interaction inter);

public:

  // --- CONSTRUCTORS/DESTRUCTOR ---

  /** default constructor
  */
  Topology();

  /** destructor */
  ~Topology();


  // === GETTERS/SETTERS ===

  /** check if Interaction inter is in the set
   *  \param inter an Interaction
   *  \return a bool
   */
  bool hasInteraction(SP::Interaction inter) const;

  /** remove an Interaction from the topology. The interaction is
   *  removed from Dynamical Systems graph and Interactions Graph.
   *  The interaction is not removed from actives subgraphs : see updateIndexSet
   *  \param inter the interaction to remove
   */
  void removeInteraction(SP::Interaction inter);

  /** add a dynamical system
   * \param ds the dynamical system to add
   */
  void insertDynamicalSystem(SP::DynamicalSystem ds);

  /** remove a dynamical system
   * \param ds the dynamical system to remove
   */
  void removeDynamicalSystem(SP::DynamicalSystem ds);

  /** link two dynamical systems to a relation
   * \param inter a SP::Interaction
   * \param ds a SP::DynamicalSystem
   * \param ds2 a SP::DynamicalSystem (optional)
   \return a vertex descriptor of the new vertex in IndexSet0
   */
  std::pair<DynamicalSystemsGraph::EDescriptor, InteractionsGraph::VDescriptor>
  link(SP::Interaction inter, SP::DynamicalSystem ds, SP::DynamicalSystem ds2 = SP::DynamicalSystem());

  /** specify if the given Interaction is for controlling the DS
   * \param inter Interaction
   * \param isControlInteraction true if the Interaction is used for
   * control purposes
   **/
  void setControlProperty(SP::Interaction inter,
                          const bool isControlInteraction);

  /** get a pointer to the graph of all Interactions.
   *  \return a SP::InteractionsGraph
   */
  inline SP::InteractionsGraph indexSet0() const
  {
    return _IG[0];
  }

  /** get a pointer to the graph at level num of Interactions
   *  \return a SP::InteractionsGraph
   */
  inline SP::InteractionsGraph indexSet(unsigned int num) const
  {
    assert(num < _IG.size()) ;
    return _IG[num];
  };

  /** get a pointer to the graph at level num of Interactions
   *  \return a SP::InteractionsGraph
   */
  inline unsigned int numberOfIndexSet() const
  {
    return _IG.size();
  };

  /** reset graph at level num of Interactions
   *  \return a SP::InteractionsGraph
   */
  inline void resetIndexSetPtr(unsigned int num)
  {
    assert(num < _IG.size()) ;

    // .. global properties may be defined here with
    // InteractionsSubGraphProperties(), see SiconosProperties.hpp
    // VertexSubProperties or EdgeSubProperties and the macros
    // INSTALL_GRAPH_PROPERTIES

    _IG[num].reset(new InteractionsGraph());

    _IG[num]->properties().symmetric = _symmetric;
    _IG[num]->update_vertices_indices();
    _IG[num]->update_edges_indices();

  };

  /** get a pointer to the graph at level num of Dynamical System
   * \param num the level
   *\return a SP::DynamicalSystemsGraph
   */
  inline SP::DynamicalSystemsGraph dSG(unsigned int num) const
  {
    assert(num < _DSG.size()) ;
    return _DSG[num];
  };

  /** get the number of Interactions Graphs */
  inline unsigned int indexSetsSize() const
  {
    return _IG.size();
  };

  /** resize Interactions Graphs
   * \param newSize the new size
   */
  inline void indexSetsResize(unsigned int newSize)
  {
    _IG.resize(newSize);
  };

  // --- isTopologyUpToDate ---

  /** check if topology has been updated since modifications occurs on nsds
  *  \return a bool
  */
  inline bool isUpToDate() const
  {
    return _isTopologyUpToDate;
  }

  // --- _hasChanged ---

  /** set _hasChanged to val
  *  \param val a bool
  */
  inline void setHasChanged(const bool val)
  {
    _hasChanged = val;
  }

  /** check
  *  \return a bool
  */
  inline bool hasChanged() const
  {
    return _hasChanged;
  }

  /** get the total number of scalar constraints
  *  \return an unsigned int
  */
  inline unsigned int numberOfConstraints() const
  {
    return _numberOfConstraints;
  };

  /** initializes the topology (called in Simulation->initialize)
  */
  void initialize();

  void clear();

  /** set symmetry in the blocks computation
   * \param val a bool
   */

  void setSymmetric(bool val)
  {
    _symmetric = val;
  }

  /** initialize graphs properties */
  void setProperties();

  /** Get a dynamical system using its number 
   \param requiredNumber the required number
  */
  SP::DynamicalSystem getDynamicalSystem(unsigned int requiredNumber);

  /** Helper to get the descriptor in DSG0 from a DynamicalSystem */
  DynamicalSystemsGraph::VDescriptor getDSG0Descriptor(SP::DynamicalSystem ds)
  {
    return _DSG[0]->descriptor(ds);
  }


};

DEFINE_SPTR(Topology)

#endif // TOPOLOGY_H
