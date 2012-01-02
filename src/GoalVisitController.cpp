#include "GoalVisitController.h"
#include "processor.h"
#include <sstream>
#include <cassert>
using std::ostringstream;
using std::ostringstream;

namespace VAL {

int GoalVisitController::encodeFact(const proposition* p)
{
  const parameter_symbol_list& vsl = *p->getArgs();
  VecInt indexes;
  for (parameter_symbol_list::const_iterator itr = vsl.begin(); itr != vsl.end(); itr++) {
    const string& arg = (*itr)->getName();
    ostringstream os;
    StringToInt::const_iterator index = params.find(arg);
    if (index == params.end()) {
      index = objectTbl.find(arg);
      //labels.push_back((*itr)->getName());
    //} else {
      assert (index != objectTbl.end()); // reference to a constant in action def that hasn't been defined
    }
    os << index->second;
    indexes.push_back(index->second);
    labels.push_back(os.str());
  }
  int factNum = getIndex(this->offsets, mults, p->getHead()->getName(), indexes);
  cout << "(" << factNum << ")";
  return factNum;
}

void GoalVisitController::visit_conj_goal(conj_goal* g) 
{
  goal_list& gl = *g->getGoals();
  for (goal_list::iterator itr = gl.begin(); itr != gl.end(); ++itr) {
    (*itr)->visit(this);
  }
}

void GoalVisitController::visit_simple_goal(simple_goal* g)
{
  const pred_symbol* p = g->getProp()->getHead();
  labels.push_back(p->getName());
  int propNumber = encodeFact(g->getProp());
  preconditionNums.push_back(propNumber);
  proc->add(proc->opsPrereqsOf,operatorNumber,propNumber);
  proc->add(proc->factsPrereqsTo,propNumber,operatorNumber);
}

void GoalVisitController::visit_disj_goal(disj_goal* g) 
{
  goal_list& gl = *g->getGoals();
  for (goal_list::iterator itr = gl.begin(); itr != gl.end(); ++itr) {
    (*itr)->visit(this);
  }
}

void GoalVisitController::visit_simple_effect(simple_effect *e)
{
  const pred_symbol* p = e->getProp()->getHead();
  effects.push_back(p->getName());
  int propNumber = proc->encodeFact(e->getProp(),params);
  effectNums.push_back(propNumber);
  proc->add(proc->opsEffectsOf,operatorNumber,propNumber);
  proc->add(proc->factsEffectsTo,propNumber,operatorNumber);
  
}

void GoalVisitController::visit_effect_lists(effect_lists* e)
{
  pc_list<simple_effect*>& addEffects = e->add_effects;
  for (pc_list<simple_effect*>::iterator itr = addEffects.begin(); itr != addEffects.end(); ++itr) {
    (*itr)->visit(this);
  }
  pc_list<simple_effect*>& delEffects = e->del_effects;
  for (pc_list<simple_effect*>::iterator itr = delEffects.begin(); itr != delEffects.end(); ++itr) {
    (*itr)->visit(this);
  }
  pc_list<cond_effect*>& condEffects = e->cond_effects;
  for (pc_list<cond_effect*>::iterator itr = condEffects.begin(); itr != condEffects.end(); ++itr) {
    (*itr)->visit(this);
  }
  pc_list<cond_effect*>& condAssgnEffects = e->cond_assign_effects;
  for (pc_list<cond_effect*>::iterator itr = condAssgnEffects.begin(); itr != condAssgnEffects.end(); ++itr) {
    assert(false); // TODO add support for these when necessary
    (*itr)->visit(this);
  }
  cout << "VISITING EFFECTS LISTS: " << e->observation_effects.size() << "\n";
  pc_list<observation*>& observations = e->observation_effects;
  for (pc_list<observation*>::iterator itr = observations.begin(); itr != observations.end(); ++itr) {
    (*itr)->visit(this);
  }
  assert(e->assign_effects.empty());
  assert(e->timed_effects.empty());
}

int GoalVisitController::getIndex(const StringToBounds& offsets, const PredicateMultTable& multTbl, const string& name, const VecInt& args)
{
  const PredicateMultTable::const_iterator itr = multTbl.find(name);
  assert (itr != multTbl.end());
  const VecInt& mults = itr->second;
  const StringToBounds::const_iterator bitr = offsets.find(name);
  assert (bitr != offsets.end());
  const Bounds& bounds = bitr->second;
  assert (args.size() == mults.size());

  int result = bounds.lower;
  for (unsigned i = 0; i < mults.size(); i++) {
    result += mults[i]*args[i];
  }
  return result;
  
}

void GoalVisitController::visit_observation(observation * r)
{
  assert(r);
  assert(!r->receivers->empty());
  cout << "REVELATION: " <<  "\n";
	for (recipients::const_iterator itr = r->receivers->begin(); itr != r->receivers->end(); ++itr) {
	    (*itr)->visit(this);
        }
}

void GoalVisitController::visit_recipient(recipient* r)
{
  assert(r);
  int argCount = r->prop->getArgs()->size();
  string head = r->prop->getHead()->getName();
}

};
