#include "LagrangianLinearR.h"
//
#include "LagrangianDS.h"

using namespace std;

// Xml constructor
LagrangianLinearR::LagrangianLinearR(RelationXML* relxml, Interaction* inter):
  Relation(relxml, inter), H(NULL), b(NULL),
  isHAllocatedIn(true), isBAllocatedIn(true)
{
  relationType = LAGRANGIANLINEARRELATION;
  if (relxml != NULL)
  {
    LagrangianLinearRXML* LLRxml = (static_cast<LagrangianLinearRXML*>(relationxml));
    unsigned int row = LLRxml->getH().size(0);
    unsigned int col = LLRxml->getH().size(1);

    if (inter != NULL)
    {
      // get size of vector y
      unsigned int size = interaction->getNInteraction();
      if (row != size)
        RuntimeException::selfThrow("LinearTIR:: xml constructor, inconsistent size with y vector for input vector or matrix");
    }
    H = new SiconosMatrix(row, col);
    *H = LLRxml->getH();
    if (LLRxml->hasB())
    {
      unsigned int rowB = LLRxml->getB().size();
      if (row != rowB)
        RuntimeException::selfThrow("LinearTIR:: xml constructor, inconsistent size between b and H");
      b = new SimpleVector(rowB);
      *b = LLRxml->getB();
    }
  }
  else RuntimeException::selfThrow("LagrangianLinearR::xml constructor xml file=NULL");
}

// Constructor from data: H, b and interaction (optional)
LagrangianLinearR::LagrangianLinearR(const SiconosMatrix& newH, const SimpleVector& newB, Interaction* inter):
  Relation(inter), H(NULL), b(NULL), isHAllocatedIn(true), isBAllocatedIn(true)
{
  relationType = LAGRANGIANLINEARRELATION;
  unsigned int size;
  if (inter != NULL)
  {
    // get size of vector y
    size = interaction->getNInteraction();
    if (newH.size(0) != size || newB.size(0) != size)
      RuntimeException::selfThrow("LinearTIR:: constructor from data, inconsistent size with y vector for input vector or matrix");
  }
  else
    size = newH.size(0);

  H = new SiconosMatrix(size, newH.size(1));
  *H = newH;
  b = new SimpleVector(size);
  *b = newB;
}

// Constructor from data: H and interaction (optional)
LagrangianLinearR::LagrangianLinearR(const SiconosMatrix& newH, Interaction* inter):
  Relation(inter), H(NULL), b(NULL), isHAllocatedIn(true), isBAllocatedIn(true)
{
  relationType = LAGRANGIANLINEARRELATION;
  unsigned int size;
  if (inter != NULL)
  {
    // get size of vector y
    size = interaction->getNInteraction();
    if (newH.size(0) != size)
      RuntimeException::selfThrow("LinearTIR:: constructor from data, inconsistent size with y vector for input vector or matrix");
  }
  else
    size = newH.size(0);

  H = new SiconosMatrix(size, newH.size(1));
  *H = newH;
}

// copy constructor
LagrangianLinearR::LagrangianLinearR(const Relation & newLLR):
  Relation(newLLR), H(NULL), b(NULL), isHAllocatedIn(true), isBAllocatedIn(true)
{
  if (relationType !=  LAGRANGIANLINEARRELATION)
    RuntimeException::selfThrow("LagrangianLinearR:: copy constructor, inconsistent relation types for copy");

  const LagrangianLinearR *  llr = static_cast<const LagrangianLinearR*>(&newLLR);
  H = new SiconosMatrix(llr->getH());
  isHAllocatedIn = true;
  if (llr->getBPtr() != NULL)
  {
    b = new SimpleVector(llr->getB());
    isBAllocatedIn = true;
  }
}

LagrangianLinearR::~LagrangianLinearR()
{
  if (isHAllocatedIn) delete H;
  H = NULL;
  if (isBAllocatedIn) delete b;
  b = NULL;
}

SiconosMatrix LagrangianLinearR::getHRelatingToDS(const int& position)
{
  if (interaction->getDynamicalSystems()[ position ]->getType() != LNLDS
      && interaction->getDynamicalSystems()[ position ]->getType() != LTIDS)
    RuntimeException::selfThrow("LagrangianLinearR::getHRelatingToDS : Error! LagrangianLinear Relation linked to a Dynamical System which is not lagrangian");
  int row, col, gap;
  row = H->size(0);
  col = static_cast<LagrangianDS*>(interaction->getDynamicalSystems()[ position ])->getNdof();

  SiconosMatrix newH(row, col);

  /*
   * the gap is used to select the good part of the H matrix, according to the right DynamicalSystem
   */
  gap = col * position;
  for (int i = 0; i < row; i++)
    for (int j = 0; j < col; j++)
      newH(i, j) = (*H)(i, j + gap);
  return newH;
}

void LagrangianLinearR::computeOutput(const double& time)
{
  IN("LagrangianLinearR::computeOutput\n");

  vector<DynamicalSystem*> vDS = interaction->getDynamicalSystems();
  vector<LagrangianDS*> vLDS;
  unsigned int size = vDS.size(), i;
  CompositeVector *qTmp = new CompositeVector();
  CompositeVector *velocityTmp = new CompositeVector();
  for (i = 0; i < size; i++)
  {
    vLDS.push_back(static_cast<LagrangianDS*>(vDS[i]));
    // check dynamical system type
    if (vDS[i]->getType() != LTIDS && vDS[i]->getType() != LNLDS)
      RuntimeException::selfThrow("LagrangianLinearR::computeOutput not yet implemented dynamical system of type: " + vDS[i]->getType());
    // Put q and velocity of each DS into a composite
    // Warning: use copy constructors, no link between pointers
    qTmp->add(vLDS[i]->getQ());
    velocityTmp->add(vLDS[i]->getVelocity());
  }

  SimpleVector *y = interaction->getYPtr();
  SimpleVector *yDot = interaction->getYDotPtr();
  // compute y and yDot
  if (b != NULL)
    *y = (*H * *qTmp) + *b;
  else
    *y = (*H * *qTmp) ;

  *yDot = (*H * *velocityTmp);

  // free memory
  delete qTmp;
  delete velocityTmp;
  OUT("LagrangianLinearR::computeOutput\n");
}

void LagrangianLinearR::computeFreeOutput(const double& time)
{
  IN("LagrangianLinearR::computeFreeOutput\n");

  vector<DynamicalSystem*> vDS = interaction->getDynamicalSystems();
  vector<LagrangianDS*> vLDS;
  unsigned int size = vDS.size(), i;
  CompositeVector *qFreeTmp = new CompositeVector();
  CompositeVector *velocityFreeTmp = new CompositeVector();

  for (i = 0; i < size; i++)
  {
    vLDS.push_back(static_cast<LagrangianDS*>(vDS[i]));
    // check dynamical system type
    if (vDS[i]->getType() != LTIDS && vDS[i]->getType() != LNLDS)
      RuntimeException::selfThrow("LagrangianLinearR::computeFreeOutput not yet implemented for this type of dynamical system " + vDS[i]->getType());
    // Put qFree and velocityFree of each DS into a composite
    // Warning: use copy constructors, no link between pointers
    qFreeTmp->add(vLDS[i]->getQFree());
    velocityFreeTmp->add(vLDS[i]->getVelocityFree());
  }

  SimpleVector *y = interaction->getYPtr();
  SimpleVector *yDot = interaction->getYDotPtr();

  // compute y and yDot
  if (b != NULL)
    *y = (*H * *qFreeTmp) + *b;
  else
    *y = (*H * *qFreeTmp);

  *yDot = (*H * *velocityFreeTmp);

  // free memory
  delete qFreeTmp;
  delete velocityFreeTmp;

  OUT("LagrangianLinearR::computeFreeOutput\n");
}

void LagrangianLinearR::computeInput(const double& time)
{
  IN("LagrangianLinearR::computeInput\n");
  vector<DynamicalSystem*> vDS = interaction->getDynamicalSystems();
  vector<LagrangianDS*> vLDS;
  unsigned int size = vDS.size(), i;
  CompositeVector *p = new CompositeVector();

  for (i = 0; i < size; i++)
  {
    vLDS.push_back(static_cast<LagrangianDS*>(vDS[i]));
    // check dynamical system type
    if (vDS[i]->getType() != LTIDS && vDS[i]->getType() != LNLDS)
      RuntimeException::selfThrow("LagrangianLinearR::computeInput not yet implemented for this type of dynamical system " + vDS[i]->getType());
    // Put p each DS into a composite
    // Warning: use addPtr -> link between pointers
    p->addPtr(vLDS[i]->getPPtr());
  }
  SimpleVector *lambda = interaction->getLambdaPtr();
  // if(size==1)
  //  *p = matTransVecMult(*H, *lambda);
  //else
  *p += matTransVecMult(*H, *lambda);
  OUT("LagrangianLinearR::computeInput\n");

}

void LagrangianLinearR::display() const
{
  cout << "---------------------------------------------------" << endl;
  cout << "____ data of the LagrangianLinearR " << endl;
  cout << "| h " << endl;
  if (H != NULL) H->display();
  else cout << "->NULL" << endl;
  cout << "| b " << endl;
  if (b != NULL) b->display();
  else cout << "->NULL" << endl;
  cout << "____________________________" << endl;
  cout << "---------------------------------------------------" << endl;
}

void LagrangianLinearR::saveRelationToXML()
{
  IN("LagrangianLinearR::saveRelationToXML\n");
  if (relationxml != NULL)
  {
    (static_cast<LagrangianLinearRXML*>(relationxml))->setH(*H) ;
    (static_cast<LagrangianLinearRXML*>(relationxml))->setB(*b) ;
  }
  else RuntimeException::selfThrow("LagrangianLinearR::saveRelationToXML - object RelationXML does not exist");
  OUT("LagrangianLinearR::saveRelationToXML\n");
}

LagrangianLinearR* LagrangianLinearR::convert(Relation *r)
{
  cout << "LagrangianLinearR::convert (Relation *r)" << endl;
  LagrangianLinearR* llr = dynamic_cast<LagrangianLinearR*>(r);
  return llr;
}

// Default (private) constructor
LagrangianLinearR::LagrangianLinearR():
  Relation(), H(NULL), b(NULL), isHAllocatedIn(false), isBAllocatedIn(false)
{
  relationType = LAGRANGIANLINEARRELATION;
}
