#include "ConditionController.h"
#include "processor.h"
#include <sstream>
#include <cassert>
using std::ostringstream;
using std::ostringstream;

namespace VAL {

void ConditionController::resetFixedPointCheck()
{
    achievedFixedPoint = true;
}

void ConditionController::foundNewTrueFact()
{
    achievedFixedPoint = false;
}

void ConditionController::resetParamList()
{
    argStrings.clear();
    params.clear();
}

bool ConditionController::hasFixedPoint()
{
    return achievedFixedPoint;
}

//void ConditionController::set(int factNum)
//{
//    if (!IsSet(state,factNum)) {
//      Set(state,factNum);
//      achievedFixedPoint = false;
//    }
//}

ConditionController::ConditionController(VAL::Processor* proc_)   : proc(proc_), ws(proc->worldState) { 
	assert(proc);
	resetFixedPointCheck();
	//state = new int[proc->intLength(proc->maxPred)];
	//memcpy(state,proc->stateBits,proc->intLength(proc->maxPred)*sizeof(int));
}

//void ConditionController::copyState(int* s)
//{
//	memcpy(s,state,proc->intLength(proc->maxPred)*sizeof(int));
//}

void ConditionController::visit_conj_goal(conj_goal* g) 
{
  goal_list& gl = *g->getGoals();
  this->truthValue = true; // An empty conjunct is true by default?
  for (goal_list::iterator itr = gl.begin(); itr != gl.end(); ++itr) {
    (*itr)->visit(this);
    if (!this->truthValue) return; // We can stop if we find one false part of conjunction
  }
  // If we make it this far the conjunction is true
}

void ConditionController::visit_neg_goal(neg_goal* g) 
{
  g->getGoal()->visit(this);
  this->truthValue = !this->truthValue;
}

void ConditionController::visit_simple_goal(simple_goal* g)
{
    const parameter_symbol_list& psl = *g->getProp()->getArgs();
    VecInt intArgs(psl.size());
    int i = 0;
    for (parameter_symbol_list::const_iterator paramItr = psl.begin(); paramItr != psl.end(); ++paramItr, ++i) {
      StringToInt::const_iterator argMapper = params.find((*paramItr)->getName());
      if (argMapper == params.end()) cout << "Offending parameter: " << (*paramItr)->getName() << "\n"; //deletable
      assert (argMapper != params.end());
      intArgs[i] = argMapper->second;
    } 
    int factNum = proc->getIndex(g->getProp()->getHead()->getName(),intArgs);
    this->truthValue = (ws.getTruthValue(factNum) == KNOWN_TRUE); //IsSet(state,factNum); 
    //cout << "Look up value for " << g->getProp()->getHead()->getName()
    //    << "(" ;
    //for (unsigned i = 0; i < intArgs.size(); i++) { 
    //  if (i) cout << ",";
    //  cout << intArgs[i];
    //}
    //cout << ") (factNum=" << factNum << "): " << this->truthValue << "\n";
      
}

void ConditionController::visit_disj_goal(disj_goal* g) 
{
  goal_list& gl = *g->getGoals();
  this->truthValue = false; // An empty disjunct is false by default? 
  //cout << "Disjunct count" << gl.size() << "\n";
  for (goal_list::iterator itr = gl.begin(); itr != gl.end(); ++itr) {
    (*itr)->visit(this);
    if (this->truthValue) return; // We can stop if we find one true part in the disjunction
  }
  // if we make it this far, the disjunction is false
}

void ConditionController::visit_qfied_goal(qfied_goal* g)
{
  assert (g->getQuantifier() == E_EXISTS || g->getQuantifier() == E_FORALL);
  const var_symbol_list& vsl = *g->getVars();
  //assert (vst.size() == 1); // Right now, I can only handle one parameter in an exists loop
  string varParam;
  unsigned nPossibilities;
  VecInt maxes(vsl.size(),0);
  VecStr varNames(vsl.size());
  int ptr = 0;
  for (var_symbol_list::const_iterator sitr = vsl.begin(); sitr != vsl.end(); ++sitr) {
	////cout << (*sitr)->type->getName() << " " << (*sitr)->getName() << "\n";
	 nPossibilities = proc->typeTbl[(*sitr)->type->getName()].size();
         varParam = (*sitr)->getName();
         maxes[ptr] = nPossibilities;
         varNames[ptr] = varParam;
	 ++ptr;
	//argTypes.push_back(sitr->second->type->getName());
  }
  VecInt args(maxes.size(),-1);
  ptr = 0;
  while (ptr >= 0) {
    if (ptr >= (int)maxes.size()) {
      g->getGoal()->visit(this);
      if (this->truthValue) return;
      ptr--;
    } else if (args[ptr]+1 >= maxes[ptr]) {
      args[ptr] = -1;
      ptr--;
    } else {
      args[ptr]++;
      params[varNames[ptr]] = args[ptr];
      ++ptr;
    }
  }
  this->truthValue = false;
}
};
