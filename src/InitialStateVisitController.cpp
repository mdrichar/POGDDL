#include "InitialStateVisitController.h"
#include "processor.h"
#include <sstream>
#include <cassert>
using std::ostringstream;
using std::ostringstream;

namespace VAL {

InitialStateVisitController::InitialStateVisitController(Processor* p_) : proc(p_), ws(proc->worldState) {}; 
void InitialStateVisitController::visit_simple_effect(simple_effect *e)
{
  int propNumber = proc->encodeGroundedFact(e->getProp());
  //initState.push_back(propNumber);
  ws.setTruthValue(propNumber,KNOWN_TRUE);
  
}

void InitialStateVisitController::visit_effect_lists(effect_lists* e)
{
  pc_list<simple_effect*>& addEffects = e->add_effects;
  for (pc_list<simple_effect*>::iterator itr = addEffects.begin(); itr != addEffects.end(); ++itr) {
    (*itr)->visit(this);
  }
  pc_list<assignment*>& assignEffects = e->assign_effects;
  for (pc_list<assignment*>::iterator itr = assignEffects.begin(); itr != assignEffects.end(); ++itr) {
    (*itr)->visit(this);
  }
}

void InitialStateVisitController::visit_assignment(assignment *a)
{
    const parameter_symbol_list& psl = *a->getFTerm()->getArgs();
    //const string& fname = a->getFTerm()->getFunction()->getName();
    VecInt intArgs(psl.size()+1);
    intArgs[0] = proc->funcHeadTbl[a->getFTerm()->getFunction()->getName()];
    //cout << "INIT:GettingPredHeadId: " << intArgs[0] << "\n";
    // Process the l-value
    int i = 1;
    for (parameter_symbol_list::const_iterator itr = psl.begin(); itr != psl.end(); ++itr, ++i) {
      if ((*itr)->isConstant()) {
	intArgs[i] =  proc->objectTbl[(*itr)->getName()];
	assert(intArgs[i] >= 0);
      } else {
  	assert(false);
      }
    }
    // process the rvalue
    a->getExpr()->visit(this);
    ws.setValue(FunctionKey(intArgs),this->rValue);
}

void InitialStateVisitController::visit_float_expression(float_expression *e) 
{
  assert (e);
  this->rValue = e->floating_value();
}

void InitialStateVisitController::visit_int_expression(int_expression *e) 
{
  assert (e);
  this->rValue = e->floating_value(); // Casts the integer to a floating_point value
}
};
