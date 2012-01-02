#include "OperatorController.h"
#include "processor.h"
#include <sstream>
#include <cassert>
#include <iterator>

using std::ostringstream;
using std::ostringstream;

const bool VERBOSE = false;

namespace VAL {

void OperatorController::considerOptimizations(simple_goal* g)
{
  if (!preconditionMode) return; // Don't process the conditions for conditional effects
  if (goalLevel <= 1) {
    int maxArg = g->getProp()->lastParamUsed();
    if (maxArg < 0) { // all constants
      int factNum = proc->factNumLookupByArgIndex(g->getProp()->getHead()->getName(),g->getProp()->argMap, VecInt());
      if (negationLevel % 2 == 0) {
        this->currentOp->prePos.insert(factNum);
	if (VERBOSE) cout << "ADDING+\n";
      } else {
        this->currentOp->preNeg.insert(factNum);
	if (VERBOSE) cout << "ADDING-\n";
      }
    } else {
      if (negationLevel % 2 == 0) {
	assert ((int)this->currentOp->prePosList.size() > maxArg);
	this->currentOp->prePosList[maxArg].push_back(g);
	if (VERBOSE) cout << "ADDING POS\n";
      } else {
	assert ((int)this->currentOp->preNegList.size() > maxArg);
	this->currentOp->preNegList[maxArg].push_back(g);
	if (VERBOSE) cout << "ADDING NEG\n";
      }
    }
  }
  if (VERBOSE) g->write2(cout,0); 
}

void OperatorController::considerQFiedGoalOptimizations(simple_goal* g)
{
  //return;
  if (!qfiedGoalMode) return; // Don't process the conditions for conditional effects
  if (qgoalLevel <= 1) {
    int maxArg = g->getProp()->lastParamUsed();
    if (maxArg < 0) { // all constants
      int factNum = proc->factNumLookupByArgIndex(g->getProp()->getHead()->getName(),g->getProp()->argMap, VecInt());
      if (qnegationLevel % 2 == 0) {
        this->currentQFiedGoal->prePos.insert(factNum);
	if (VERBOSE) cout << "ADDING+\n";
      } else {
        this->currentQFiedGoal->preNeg.insert(factNum);
	if (VERBOSE) cout << "ADDING-\n";
      }
    } else {
      if (qnegationLevel % 2 == 0) {
	assert ((int)this->currentQFiedGoal->prePosList.size() > maxArg);
	this->currentQFiedGoal->prePosList[maxArg].push_back(g);
	if (VERBOSE) cout << "ADDING QFIED POS\n";
      } else {
	assert ((int)this->currentQFiedGoal->preNegList.size() > maxArg);
	this->currentQFiedGoal->preNegList[maxArg].push_back(g);
	if (VERBOSE) cout << "ADDING QFIED NEG\n";
      }
    }
  }
  if (VERBOSE) g->write2(cout,0); 
}

OperatorController::OperatorController(VAL::Processor* proc_)   : proc(proc_) { 
	assert(proc);
	goalLevel = 0;
	negationLevel = 0;
	preconditionMode = true;
	qfiedGoalMode = false;
	this->inLValue = false;
}

void OperatorController::visit_conj_goal(conj_goal* g) 
{
  goal_list& gl = *g->getGoals();
  for (goal_list::iterator itr = gl.begin(); itr != gl.end(); ++itr) {
    ++goalLevel;
    ++qgoalLevel;
    (*itr)->visit(this);
    --qgoalLevel;
    --goalLevel;
  }
}

void OperatorController::assignParameterIndex(const string& name)
{
  unsigned nextIndex = opArgsMap.size();
  if (!(opArgsMap.find(name) == opArgsMap.end())) {
    if (VERBOSE) cout << "Repeat arg: " << name << " " << opArgsMap[name] << "\n";
  }
  assert (opArgsMap.find(name) == opArgsMap.end());
  opArgsMap[name] = nextIndex;
}

void OperatorController::visit_operator_(operator_* op) 
{
  assert(op);
  op->argMaxes.resize(op->parameters->size());
  //cout << "SIZE: " << op->argMaxes.size() << " " << op->name->getName() << "\n";
  op->argTypes.resize(op->parameters->size());
  int i = 0;
  for (var_symbol_list::const_iterator sitr = op->parameters->begin(); sitr != op->parameters->end(); ++i, ++sitr) {
      assignParameterIndex((*sitr)->getName());
      op->argMaxes[i] = proc->typeTbl[(*sitr)->type->getName()].size();
      op->argTypes[i] = (*sitr)->type->getName();
      op->argMapper[(*sitr)->getName()] = i;
  }
  op->prePosList.resize(op->argMaxes.size());
  op->preNegList.resize(op->argMaxes.size());
  //copy(op->argMaxes.begin(),op->argMaxes.end(),std::ostream_iterator<int>(cout," ")); cout << "\n";
}

void OperatorController::visit_gain(gain_def *g) 
{
  assert(g);
  // Set up the mapping for all the arguments so that propositions in the gain goal condition will know
  // that, e.g., ?p refers to arg[0];
  for (var_symbol_list::const_iterator sitr = g->getArgs()->begin(); sitr != g->getArgs()->end(); ++sitr) {
      assignParameterIndex((*sitr)->getName());
      //op->argMaxes[i] = proc->typeTbl[(*sitr)->type->getName()].size();
  }
  preconditionMode=false;
  g->getCond()->visit(this);
  g->getExp()->visit(this);
  g->setArgSizeNeeded(opArgsMap.size());
  preconditionMode=true;
}

void OperatorController::visit_action(action * op) 
{
  assert(op);
  this->visit_operator_(op);
  this->currentOp = op;
  preconditionMode = true;
  op->precondition->visit(this);
  preconditionMode = false;
  op->effects->visit(this);
  op->setArgSizeNeeded(this->opArgsMap.size());
  
}

void OperatorController::visit_neg_goal(neg_goal* g) 
{
  negationLevel++;
  qnegationLevel++;
  g->getGoal()->visit(this);
  qnegationLevel--;
  negationLevel--;
  
  //g->getGoal()->setPolarity(false);
}

void OperatorController::visit_simple_goal(simple_goal* g)
{
    const parameter_symbol_list& psl = *g->getProp()->getArgs();
    VecInt argMap(psl.size());
    int i = 0;
    if (VERBOSE) cout << "Prop: " << g->getProp()->getHead()->getName() << " ";
    for (parameter_symbol_list::const_iterator paramItr = psl.begin(); paramItr != psl.end(); ++paramItr, ++i) {
      if ((*paramItr)->isConstant()) {
	argMap[i] = -1 - proc->objectTbl[(*paramItr)->getName()];
        if (VERBOSE) cout << argMap[i];
      } else {
	argMap[i] = opArgsMap[(*paramItr)->getName()];
        if (VERBOSE) cout << argMap[i];
        if (VERBOSE) cout << "?";
      }
      if (VERBOSE) cout << (*paramItr)->getName() << " "; 
    } 
    g->prop->argMap = argMap;
    g->getProp()->setHeadId(proc->predHeadTbl[g->getProp()->getHead()->getName()]);
    considerOptimizations(g);
    considerQFiedGoalOptimizations(g);
    if (VERBOSE) cout << "  Setting up argMap for " << g->getProp()->getHead()->getName()
	<< "(" ;
    if (VERBOSE) for (unsigned i = 0; i < argMap.size(); i++) { 
      if (i) cout << ",";
      cout << argMap[i];
    }
    if (VERBOSE) cout << ")\n";
}

void OperatorController::visit_disj_goal(disj_goal* g) 
{
  goal_list& gl = *g->getGoals();
  for (goal_list::iterator itr = gl.begin(); itr != gl.end(); ++itr) {
    ++goalLevel;
    (*itr)->visit(this);
    --goalLevel;
  }
}

void OperatorController::visit_qfied_goal(qfied_goal* g)
{
  assert (g->getQuantifier() == E_EXISTS || g->getQuantifier() == E_FORALL);
  const var_symbol_list& vsl = *g->getVars();
  string varParam;
  unsigned nPossibilities;
  VecInt maxes(vsl.size(),0);
  VecInt argMap(vsl.size(),-1);
  int ptr = 0;
  for (var_symbol_list::const_iterator sitr = vsl.begin(); sitr != vsl.end(); ++sitr) {
	if (VERBOSE) cout << (*sitr)->type->getName() << " "
	 << (*sitr)->getName() << "\n";
	 nPossibilities = proc->typeTbl[(*sitr)->type->getName()].size();
         varParam = (*sitr)->getName();
	 StringToInt::const_iterator argEntry = opArgsMap.find(varParam);
         if (argEntry == opArgsMap.end()) {
  	   unsigned nextIndex = opArgsMap.size();
	   opArgsMap[varParam] = nextIndex;
	   argMap[ptr] = nextIndex;
         } else {
	   assert (false); // This means that one of the variables being quantified over
	   // is in fact already bound to some value in a higher level of nesting in the
	   // expression, I believe.  This kind of variable cannot be allowed.  There's no
	   // reason (other than convenience) that a user can't just rename the variable so that
 	   // the names don't donflict.
	   argMap[ptr] = argEntry->second;
	 }
         maxes[ptr] = nPossibilities;
	 if (VERBOSE) cout << varParam << ":" << argMap[ptr] << " ";
	 ++ptr;
	//argTypes.push_back(sitr->second->type->getName());
  }
  // No reason why these can't be used straight up instead of copying them over at the end like this
  g->argMaxes = maxes;
  g->argMap = argMap; 
  qgoalLevel = 0;
  qnegationLevel = 0;
  //cout << "SIZE: " << g->argMaxes.size() << " " << opArgsMap.size() << " " <<  "\n";
  g->prePosList.resize(opArgsMap.size()); // The slots in the lists corresponding to arguments for the op (rather than the quantified formula) may not be used; they're just placeholders
  g->preNegList.resize(opArgsMap.size());
  //cout << "SIZE: " << g->prePosList.size() << " " << g->preNegList.size() << "\n";
  this->currentQFiedGoal = g;
  this->qfiedGoalMode = true;
  g->getGoal()->visit(this); // now recurse to visit the body of the goal
  this->qfiedGoalMode = false;
  if (VERBOSE) cout << "\n";
}

// Effects
void OperatorController::visit_forall_effect(forall_effect *e) 
{
  // Mostly copied from OperatorController
  const var_symbol_list& vsl = *e->getVarsList();
  string varParam;
  unsigned nPossibilities;
  VecInt maxes(vsl.size(),0);
  VecInt argMap(vsl.size(),-1);
  int ptr = 0;
  for (var_symbol_list::const_iterator sitr = vsl.begin(); sitr != vsl.end(); ++sitr) {
	//cout << (*sitr)->type->getName() << " "
	// << (*sitr)->getName() << "\n";
	 nPossibilities = proc->typeTbl[(*sitr)->type->getName()].size();
         varParam = (*sitr)->getName();
	 StringToInt::const_iterator argEntry = opArgsMap.find(varParam);
         if (argEntry == opArgsMap.end()) {
  	   unsigned nextIndex = opArgsMap.size();
	   opArgsMap[varParam] = nextIndex;
	   argMap[ptr] = nextIndex;
         } else {
	   assert (false); // This means that one of the variables being quantified over
	   // is in fact already bound to some value in a higher level of nesting in the
	   // expression, I believe.  This kind of variable cannot be allowed.  There's no
	   // reason (other than convenience) that a user can't just rename the variable so that
 	   // the names don't donflict.
	   argMap[ptr] = argEntry->second;
	 }
         maxes[ptr] = nPossibilities;
	 //cout << varParam << ":" << argMap[ptr] << " ";
	 ++ptr;
	//argTypes.push_back(sitr->second->type->getName());
  }
  // No reason why these can't be used straight up instead of copying them over at the end like this
  e->argMaxes = maxes;
  e->argMap = argMap; 
  e->getEffects()->visit(this); // now recurse to visit the body of the goal
  //cout << "\n";

}

void OperatorController::visit_effect_lists(effect_lists* e) 
{
  if (VERBOSE) cout << "VISITING EFFECTS LISTS: " << e->observation_effects.size() << "\n";
  pc_list<simple_effect*>& addEffects = e->add_effects;
  for (pc_list<simple_effect*>::iterator itr = addEffects.begin(); itr != addEffects.end(); ++itr) {
    (*itr)->visit(this);
  }
  pc_list<simple_effect*>& delEffects = e->del_effects;
  for (pc_list<simple_effect*>::iterator itr = delEffects.begin(); itr != delEffects.end(); ++itr) {
    (*itr)->visit(this);
  }
  pc_list<cond_effect*>& condEffects = e->cond_effects;
  int i = 0;
  for (pc_list<cond_effect*>::iterator itr = condEffects.begin(); itr != condEffects.end(); ++itr, ++i) {
    (*itr)->visit(this);
  }
  pc_list<assignment*>& assignEffects = e->assign_effects;
  for (pc_list<assignment*>::iterator itr = assignEffects.begin(); itr != assignEffects.end(); ++itr) {
    (*itr)->visit(this);
  }
  pc_list<cond_effect*>& condAssgnEffects = e->cond_assign_effects;
  for (pc_list<cond_effect*>::iterator itr = condAssgnEffects.begin(); itr != condAssgnEffects.end(); ++itr) {
    assert(false); // TODO add support for these when necessary
    (*itr)->visit(this);
  }
  // \TODO add support for observations
  pc_list<observation*>& observations = e->observation_effects;
  i = 0;
  for (pc_list<observation*>::iterator itr = observations.begin(); itr != observations.end(); ++itr, ++i) {
    (*itr)->visit(this);
  }

  pc_list<forall_effect*>& forallEffects= e->forall_effects;
  for (pc_list<forall_effect*>::iterator itr = forallEffects.begin(); itr != forallEffects.end(); ++itr) {
    (*itr)->visit(this);
  }
  //assert(e->assign_effects.empty());
  assert(e->timed_effects.empty());
}

void OperatorController::visit_simple_effect(simple_effect* e)
{
    const parameter_symbol_list& psl = *e->getProp()->getArgs();
    VecInt argMap(psl.size());
    int i = 0;
    if (VERBOSE) cout << "EProp: " << e->getProp()->getHead()->getName() << " ";
    for (parameter_symbol_list::const_iterator paramItr = psl.begin(); paramItr != psl.end(); ++paramItr, ++i) {
      if ((*paramItr)->isConstant()) {
	argMap[i] = -1 - proc->objectTbl[(*paramItr)->getName()];
        if (VERBOSE) cout << argMap[i];
      } else {
	argMap[i] = opArgsMap[(*paramItr)->getName()];
        if (VERBOSE) cout << argMap[i];
        if (VERBOSE) cout << "?";
      }
      if (VERBOSE) cout << (*paramItr)->getName() << " "; 
    } 
    e->prop->argMap = argMap;
    e->prop->setHeadId(proc->predHeadTbl[e->getProp()->getHead()->getName()]);
    proc->mutablePreds[e->prop->getHeadId()] = 1; // These kinds of facts can change over time
    if (VERBOSE) cout << "MUTABLE: " << e->prop->getHead()->getName() << "\n";
    if (VERBOSE) cout << "  Setting up argMap for " << e->getProp()->getHead()->getName()
	<< "(" ;
    if (VERBOSE) for (unsigned i = 0; i < argMap.size(); i++) { 
      if (i) cout << ",";
      cout << argMap[i];
    }
    if (VERBOSE) cout << ")\n";
}

void OperatorController::visit_cond_effect(cond_effect* e)
{
  assert(e);
  assert(e->getEffects());
  assert(e->getCondition());
  // Since this is just a preprocessing step to build up the argMap infrastructure that the system will use
  // We need to "visit" both the condition and the effects
  e->getCondition()->visit(this);
  e->getEffects()->visit(this);
}

void OperatorController::visit_mul_expression(mul_expression *m) 
{
  m->getLHS()->visit(this);
  m->getRHS()->visit(this);
}

void OperatorController::visit_div_expression(div_expression *d) 
{
  d->getLHS()->visit(this);
  d->getRHS()->visit(this);
}

void OperatorController::visit_plus_expression(plus_expression *e)
{
  e->getLHS()->visit(this);
  e->getRHS()->visit(this);
}

void OperatorController::visit_int_expression(int_expression *)
{
// Don't need to do anything here
}

void OperatorController::visit_func_term(func_term * e)
{

// Most of this was pasted from visit_simple_effect -- there could/should be some consolidation / method extraction
    const parameter_symbol_list& psl = *e->getArgs();
    VecInt argMap(psl.size());
    unsigned funcHeadId = 0;
    const string& name = e->getFunction()->getName();
    StringToInt::const_iterator headFinder = proc->funcHeadTbl.find(name);
    if (headFinder != proc->funcHeadTbl.end()) {
	funcHeadId = headFinder->second;
    } else {
	funcHeadId = proc->funcHeadTbl.size();
	proc->inverseFuncs.push_back(name);
        proc->funcHeadTbl[name] = funcHeadId;
        //proc->inverseFuncs.push_back(name);
    }
    //cout << "Function head: " << name << " " << funcHeadId << "\n";
    e->setHeadId(funcHeadId);
    if (this->inLValue) {
	proc->mutableFuncs[e->getHeadId()] = 1; // This function value is mutable
    }
    int i = 0;
    //cout << "Function head: " << e->getFunction()->getName() << " ";
    for (parameter_symbol_list::const_iterator paramItr = psl.begin(); paramItr != psl.end(); ++paramItr, ++i) {
      if ((*paramItr)->isConstant()) {
	argMap[i] = -1 - proc->objectTbl[(*paramItr)->getName()];
        //cout << argMap[i];
      } else {
	argMap[i] = opArgsMap[(*paramItr)->getName()];
        //cout << argMap[i];
        //cout << "?";
      }
      //cout << (*paramItr)->getName() << " "; 
    } 
    e->setArgs(argMap);
    //cout << "  Setting up argMap for " << e->getProp()->getHead()->getName()
    //    << "(" ;
    //for (unsigned i = 0; i < argMap.size(); i++) { 
    //  if (i) cout << ",";
    //  cout << argMap[i];
    //}
    //cout << ")\n";

}

void OperatorController::visit_comparison(comparison *e) 
{
  e->getLHS()->visit(this);
  e->getRHS()->visit(this);
}

void OperatorController::visit_assignment(assignment *e) 
{
  this->inLValue = true;
  e->getFTerm()->visit(this);
  this->inLValue = false;
  e->getExpr()->visit(this);
}

void OperatorController::visit_minus_expression(minus_expression *e) 
{
  e->getLHS()->visit(this);
  e->getRHS()->visit(this);
}

// observations

void OperatorController::visit_observation(observation *r)
{
  //cout << "observation Visitation " << r->receivers->size() << ": ";
  for (recipients::const_iterator itr = r->receivers->begin(); itr != r->receivers->end(); ++itr) {
	//cout << (*itr)->receivers->size() << " ";
    for (parameter_symbol_list::const_iterator pitr = (*itr)->receivers->begin(); pitr != (*itr)->receivers->end(); ++pitr) {
	//cout << ":";
        const string& name = (*pitr)->getName();
	if ((*pitr)->isConstant()) {
          if (name != "all" && name != "others") {
	    r->recipientMapper[name] = -1 - proc->objectTbl[name]; 
          }
        } else {
            r->recipientMapper[name] = this->opArgsMap[name];
	}
	//if (name != "all" && name != "others") cout << "Mapping " << name << " to " << r->recipientMapper[name] << "\n";
    }
    this->processRevMsg((*itr)->prop);
  }
  //cout << "\n";
 
}

void OperatorController::processRevMsg(proposition* p) 
{
    const parameter_symbol_list& psl = *p->getArgs();
    const string& name = p->getHead()->getName();
    unsigned revHeadId = 0;
    StringToInt::const_iterator headFinder = proc->revHeadTbl.find(name);
    if (headFinder != proc->revHeadTbl.end()) {
	revHeadId = headFinder->second;
    } else {
	revHeadId = proc->revHeadTbl.size();
        proc->inverseRevs.push_back(name);
        proc->opsByRev.push_back(SetVecInt());
        proc->revHeadTbl[name] = revHeadId;
        proc->revParamTypeTbl.push_back(VecInt(psl.size(),-1));
        proc->revParamTypeCardTbl.push_back(VecInt(psl.size(),-1));
    }
    VecInt& revParamTypeIds = proc->revParamTypeTbl[revHeadId];
    VecInt& revParamTypeCards = proc->revParamTypeCardTbl[revHeadId];
    VecInt argMap(psl.size());
    //if (!proc->isRevAlreadyPresent(name)) {
    //    proc->newRevelationHeadId(name,psl.size());
    //}
    p->setHeadId(proc->revHeadTbl[name]);
    VecInt revMapper(psl.size()+1,0);
    revMapper[psl.size()] = this->currentOp->getHeadId();
    int i = 0;
    for (parameter_symbol_list::const_iterator paramItr = psl.begin(); paramItr != psl.end(); ++paramItr, ++i) {
      if ((*paramItr)->isConstant()) {
	if ((*paramItr)->getName() == "u") {
	  argMap[i] = UVAL;
	  revMapper[i] = UVAL;
        } else {
	  argMap[i] = -1 - proc->objectTbl[(*paramItr)->getName()];
	  revMapper[i] = argMap[i];
	  int typeId = proc->typeNameIds[proc->objectTypes[(*paramItr)->getName()]];
	  assert (revParamTypeIds[i] == -1 || revParamTypeIds[i] == typeId);
          revParamTypeIds[i] = typeId;
 	  revParamTypeCards[i] = 	proc->typeCardinalities[typeId];
	}
      } else {
	revMapper[i] = this->currentOp->getVarIndex((*paramItr)->getName());
	//cout << "Setting arg " << (*paramItr)->getName() << " to " << revMapper[i];
	argMap[i] = opArgsMap[(*paramItr)->getName()];
	int typeId = proc->typeNameIds[currentOp->argTypes[argMap[i]]];
	assert (revParamTypeIds[i] == -1 || revParamTypeIds[i] == typeId);
        revParamTypeIds[i] = typeId;
 	revParamTypeCards[i] = 	proc->typeCardinalities[typeId];
      }
    } 
    proc->opsByRev[p->getHeadId()].insert(revMapper);
    p->argMap = argMap;
}

};
