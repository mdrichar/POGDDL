#include "NewConditionController.h"
#include "processor.h"
#include <sstream>
#include <cassert>
#include <iterator>
using std::ostringstream;
using std::ostringstream;

const bool VERBOSE = false;
const bool QVERBOSE = true;

namespace VAL {

bool NewConditionController::passesScreeningChecks(operator_* op, int argNum) {
	//return true;
	//cout << "Screening Checks\n";
	if (tracking) {
		//cout << "Clearing\n";
		posScreeners.clear();
		negScreeners.clear();
	}
	if (argNum < 0) {
		for (SetInt::const_iterator it = op->prePos.begin(); it != op->prePos.end(); ++it) {
			if (ws.getTruthValue(*it) == KNOWN_FALSE) {
				if (VERBOSE)
					cout << "INDPOSFAILURE" << "\n";
				return false;
			}
		}
		for (SetInt::const_iterator it = op->preNeg.begin(); it != op->preNeg.end(); ++it) {
			if (ws.getTruthValue(*it) == KNOWN_TRUE) {
				//if (IsSet(state,*it)) {
				if (VERBOSE)
					cout << "INDNEGFAILURE" << "\n";
				return false;
			}
		}
	} else if (argNum < (int) op->prePosList.size()) { // TODO: Fix this kludge; it used to be just 'else'; I changed it when I started calling passesScreeningChecks from AG.oneLegalAction with potentially big argNums
		for (unsigned j = 0; j < op->prePosList[argNum].size(); j++) {
			op->prePosList[argNum][j]->visit(this);
			if (VERBOSE)
				cout << "POSFAILURE" << "\n";
			if (this->truthValue == KNOWN_FALSE)
				return false;
			if (tracking)
				posScreeners.insert(WorldState::lastChecked); // Keep track of the preconditions that must be satisfied
		}
		for (unsigned j = 0; j < op->preNegList[argNum].size(); j++) {
			op->preNegList[argNum][j]->visit(this);
			if (VERBOSE)
				cout << "NEGFAILURE" << "\n";
			if (this->truthValue == KNOWN_TRUE)
				return false;
			if (tracking)
				negScreeners.insert(WorldState::lastChecked); // Keep track of the preconditions that must be satisfied
		}

	}
	return true;
}

bool NewConditionController::passesQFiedScreeningChecks(qfied_goal* g, int argNum) {
	//return true;
	//cout << "Screening Checks\n";
	//if (tracking) {
	//  //cout << "Clearing\n";
	//  posScreeners.clear();
	//  negScreeners.clear();
	//}
	if (argNum < 0) {
		for (SetInt::const_iterator it = g->prePos.begin(); it != g->prePos.end(); ++it) {
			if (ws.getTruthValue(*it) == KNOWN_FALSE) {
				if (QVERBOSE)
					cout << "INDPOSFAILURE " << proc->getFact(WorldState::lastChecked) << "\n";
				return false;
			}
		}
		for (SetInt::const_iterator it = g->preNeg.begin(); it != g->preNeg.end(); ++it) {
			if (ws.getTruthValue(*it) == KNOWN_TRUE) {
				//if (IsSet(state,*it)) {
				if (QVERBOSE)
					cout << "INDNEGFAILURE " << proc->getFact(WorldState::lastChecked) << "\n";
				return false;
			}
		}
	} else {
		for (unsigned j = 0; j < g->prePosList[argNum].size(); j++) {
			g->prePosList[argNum][j]->visit(this);
			if (QVERBOSE)
				cout << "POSFAILURE " << proc->getFact(WorldState::lastChecked) << "\n";
			if (this->truthValue == KNOWN_FALSE)
				return false;
			//if (tracking) posScreeners.insert(WorldState::lastChecked); // Keep track of the preconditions that must be satisfied
		}
		for (unsigned j = 0; j < g->preNegList[argNum].size(); j++) {
			g->preNegList[argNum][j]->visit(this);
			if (QVERBOSE)
				cout << "NEGFAILURE " << proc->getFact(WorldState::lastChecked) << "\n";
			if (this->truthValue == KNOWN_TRUE)
				return false;
			//if (tracking) negScreeners.insert(WorldState::lastChecked); // Keep track of the preconditions that must be satisfied
		}

	}
	return true;
}

void NewConditionController::resetParamList() {
	params.clear();
}

//void NewConditionController::set(int factNum)
//{
//    if (!IsSet(state,factNum)) {
//      Set(state,factNum);
//    }
//}
//
//void NewConditionController::unset(int factNum)
//{
//    if (IsSet(state,factNum)) {
//      Reset(state,factNum);
//    }
//}

//void NewConditionController::clearEffects()
//{
//  addFacts.clear();
//  delFacts.clear();
//}

NewConditionController::NewConditionController(VAL::Processor* proc_, WorldState& worldState) :
		proc(proc_), ws(worldState) {
	assert(proc);
	//state = new int[proc->intLength(proc->maxPred)];
	tracking = false;
//	memcpy(state,proc->stateBits,proc->intLength(proc->maxPred)*sizeof(int));
}

//void NewConditionController::copyState(int* s)
//{
//	memcpy(s,state,proc->intLength(proc->maxPred)*sizeof(int));
//}

void NewConditionController::visit_conj_goal(conj_goal* g) {
	goal_list& gl = *g->getGoals();
	TruthState tmpTruthState = KNOWN_TRUE;
	for (goal_list::iterator itr = gl.begin(); itr != gl.end(); ++itr) {
		(*itr)->visit(this);
		// We can stop if we find one false part of conjunction
		if (this->truthValue == KNOWN_FALSE)
			return;
		if (this->truthValue == UNKNOWN) {
			tmpTruthState = UNKNOWN;
		}
	}
	this->truthValue = tmpTruthState;

}

void NewConditionController::visit_neg_goal(neg_goal* g) {
	g->getGoal()->visit(this);
	if (this->truthValue == KNOWN_TRUE) {
		this->truthValue = KNOWN_FALSE;
	} else if (this->truthValue == KNOWN_FALSE) {
		this->truthValue = KNOWN_TRUE;
	}
	// Otherwise, it's don't know, and the negation of don't know is don't know
}

void NewConditionController::visit_gain(gain_def *g) {
	g->getExp()->visit(this);
	//cout << "SETTING PAYOFF: " << this->valueStack.top() << "\n";
	g->setPayoff(this->valueStack.top());
	this->valueStack.pop();
	g->getCond()->visit(this); // set the truth value appropriately
}

void NewConditionController::visit_simple_goal(simple_goal* g) {
	//int factNum = proc->fastNumLookupByArgIndex(g->getProp()->getHeadId(),g->getProp()->argMap,this->args);
	//this->truthValue = IsSet(state,factNum);
	this->truthValue = ws.getTruthValue(g->getProp()->getHeadId(), g->getProp()->argMap, this->args);
	//cout << "Truth value of " << proc->getFact(WorldState::lastChecked) << " is " << this->truthValue << std::endl;
}

void NewConditionController::visit_func_term(func_term * f) {
	//static const bool verbose = false;
	static VecInt intArgs;
	NumScalar funcVal = ws.fastFuncLookupByArgIndex(intArgs, f->getHeadId(), f->argMap, args);
	//int funcVal = ws.getFuncVal(f->getHeadId()
	this->valueStack.push(funcVal);

}

void NewConditionController::visit_disj_goal(disj_goal* g) {
	goal_list& gl = *g->getGoals();
	TruthState tmpTruthState = KNOWN_FALSE;
	for (goal_list::iterator itr = gl.begin(); itr != gl.end(); ++itr) {
		(*itr)->visit(this);
		// We can stop if we find one false part of conjunction
		if (this->truthValue == KNOWN_TRUE)
			return;
		if (this->truthValue == UNKNOWN) {
			tmpTruthState = UNKNOWN;
		}
	}
	this->truthValue = tmpTruthState;
}

void NewConditionController::visit_qfied_goal(qfied_goal* g) {
	//assert(false);
	// TODO
	// The qfied goal should have maxes that specify the number of each argument
	// that need to be iterated over (g->prop->argMaxes).  The g->prop->argMap
	// should tell which argument in this->args corresponds to each variable
	// being quantified.  We need to iterate over each of those combinations
	// In each case, we need to set the correct value for the correct argument
	// in this->args, and then visit the g->body.  For the predicates in the body,
	// the variables may correspond to variables in the action list or in the
	// quantifier list.  It's the challenge to figure out how to do that.
	// Setting it up correctly should be done in OperatorController::visit_qfied_goal.
	// Note that because argMap may return indices that exceed the number of args
	// in the parameter list of the action.  So assert(opArgMap.size() == this->args.size()),
	// but ! assert(this->args.size() == op->parameters->size()) .

	quantifier qfier = g->getQuantifier();
	assert(qfier == E_EXISTS || qfier == E_FORALL);
	VecInt& maxes = g->argMaxes;
	VecInt& argMap = g->argMap;
	TruthState tmpTruthState = (qfier == E_FORALL) ? KNOWN_TRUE : KNOWN_FALSE;
	// Need to initialize all args that I'll be iterating over to -1
	int ptr = 0;
	for (ptr = 0; ptr < (int) maxes.size(); ++ptr) {
		assert(argMap[ptr] < (int)args.size());
		args[argMap[ptr]] = -1;
	}
	ptr = 0;
	while (ptr >= 0) {
		if (ptr >= (int) maxes.size()) {
			//cout << "args[argMap[0]]: " << args[argMap[0]] << " Testing goal: ";
			//g->getGoal()->write2(cout,0);
			g->getGoal()->visit(this);
			if ((this->truthValue == KNOWN_TRUE && qfier == E_EXISTS) || // Short circuit condition for "there exists" is true
					(this->truthValue == KNOWN_FALSE && qfier == E_FORALL))
				return; // short circuit condition for "forall" is false
			if (this->truthValue == UNKNOWN) {
				tmpTruthState = UNKNOWN;
			}
			ptr--;
		} else if (args[argMap[ptr]] + 1 >= maxes[ptr]) { // TODO: rename maxes to sizes or something,
			// TODO (cont) which is one more than the maximum value the arg can take, since they're numbered 0 to (#-1).
			args[argMap[ptr]] = -1;
			ptr--;
		} else {
			args[argMap[ptr]]++;
			//params[varNames[ptr]] = args[ptr];
			//if (passesQFiedScreeningChecks(g,argMap[ptr])) { //\TODO: Screening checks for quantified vars still need to be implemented -- more important for EXISTS than FORALL
			++ptr;
			//}
		}
	}
	// If we get to this point, that means that we didn't short circuit.  If it's a FORALL expr, that means that none
	// of the subgoals was false, so we want to return true.  If it's a EXISTS expr, that means none of the subgoals
	// was true, so we want to return false;
	this->truthValue = tmpTruthState; //(qfier == E_FORALL);
	cout << "Result for quantified goal: " << this->truthValue << std::endl;
}

// EFFECTS
void NewConditionController::visit_forall_effect(forall_effect *e) {
//very similar to the action in visit_qfied_goal
	VecInt& maxes = e->argMaxes;
	VecInt& argMap = e->argMap;
	// Need to initialize all args that I'll be iterating over to -1
	int ptr = 0;
	for (ptr = 0; ptr < (int) maxes.size(); ++ptr) {
		assert(argMap[ptr] < (int)args.size());
		args[argMap[ptr]] = -1;
	}
	ptr = 0;
	while (ptr >= 0) {
		if (ptr >= (int) maxes.size()) {
			e->getEffects()->visit(this); // <-- this is what is different that visiting a qfied goal
			ptr--;
		} else if (args[argMap[ptr]] + 1 >= maxes[ptr]) { // TODO: rename maxes to sizes or something,
			// TODO (cont) which is one more than the maximum value the arg can take, since they're numbered 0 to (#-1).
			args[argMap[ptr]] = -1;
			ptr--;
		} else {
			args[argMap[ptr]]++;
			//params[varNames[ptr]] = args[ptr];
			++ptr;
		}
	}
}

void NewConditionController::visit_effect_lists(effect_lists* e) {
	//cout << "VISITING EFFECTS LISTS: " << e->observation_effects.size() << "\n";
	pc_list<simple_effect*>& addEffects = e->add_effects;
	for (pc_list<simple_effect*>::iterator itr = addEffects.begin(); itr != addEffects.end(); ++itr) {
		(*itr)->visit(this);
		//addFacts.insert(currentFact);
		ws.add(currentFact);
	}
	pc_list<simple_effect*>& delEffects = e->del_effects;
	for (pc_list<simple_effect*>::iterator itr = delEffects.begin(); itr != delEffects.end(); ++itr) {
		(*itr)->visit(this);
		//delFacts.insert(currentFact);
		ws.del(currentFact);
	}
	pc_list<cond_effect*>& condEffects = e->cond_effects;
	for (pc_list<cond_effect*>::iterator itr = condEffects.begin(); itr != condEffects.end(); ++itr) {
		(*itr)->visit(this);
	}
	pc_list<cond_effect*>& condAssgnEffects = e->cond_assign_effects;
	for (pc_list<cond_effect*>::iterator itr = condAssgnEffects.begin(); itr != condAssgnEffects.end(); ++itr) {
		assert(false);
		// TODO add support for these when necessary
		(*itr)->visit(this);
	}
	pc_list<assignment*>& assignEffects = e->assign_effects;
	for (pc_list<assignment*>::iterator itr = assignEffects.begin(); itr != assignEffects.end(); ++itr) {
		(*itr)->visit(this);
	}
	pc_list<observation*>& observations = e->observation_effects;
	for (pc_list<observation*>::iterator itr = observations.begin(); itr != observations.end(); ++itr) {
		(*itr)->visit(this);
	}

	pc_list<forall_effect*>& forallEffects = e->forall_effects;
	for (pc_list<forall_effect*>::iterator itr = forallEffects.begin(); itr != forallEffects.end(); ++itr) {
		(*itr)->visit(this);
	}
	//assert(e->assign_effects.empty());
	assert(e->timed_effects.empty());
}

void NewConditionController::visit_simple_effect(simple_effect* e) {
	this->currentFact = proc->fastNumLookupByArgIndex(e->getProp()->getHeadId(), e->getProp()->argMap, this->args);
}

void NewConditionController::visit_cond_effect(cond_effect* e) {
	assert(e);
	assert(e->getEffects());
	assert(e->getCondition());
	// Only visit the effects if the condition is satisfied
	this->truthValue = KNOWN_FALSE;
	e->getCondition()->visit(this);
	if (this->truthValue == KNOWN_TRUE) {
		e->getEffects()->visit(this);
	} else {
		// \TODO: Figure out what to do here -- I honestly don't know.  Originally, I commented this out when working on Territory game in which
		// conditional effects for the move action caused the score to increase if something wasn't visited. The observation depends on detectability
		// None of these effects affect the applicability of future actions, so I don't need to execute the updates.  If they did affect future applicability
		// I would probably have an ill-defined game.
		//assert (this->truthValue == KNOWN_FALSE); // We can't be executing actions right now if we don't know the truth value of the condition for sure
	}
}

// fluents, arithmetic expressions
void NewConditionController::visit_comparison(comparison* c) {
	static const bool verbose = VERBOSE;
	c->getLHS()->visit(this);
	assert(!this->valueStack.empty());
	c->getRHS()->visit(this);
	//cout << "Value stack size: " << this->valueStack.size() << "\n";
	assert(this->valueStack.size() > 1);
	NumScalar rhsVal = this->valueStack.top();
	this->valueStack.pop();
	NumScalar lhsVal = this->valueStack.top();
	this->valueStack.pop();
	ostringstream os;
	switch (c->getOp()) {
	case E_GREATER:
		os << ">";
		this->truthValue = (lhsVal > rhsVal) ? KNOWN_TRUE : KNOWN_FALSE;
		break;
	case E_GREATEQ:
		os << ">=";
		this->truthValue = (lhsVal >= rhsVal) ? KNOWN_TRUE : KNOWN_FALSE;
		break;
	case E_LESS:
		os << "<";
		this->truthValue = (lhsVal < rhsVal) ? KNOWN_TRUE : KNOWN_FALSE;
		break;
	case E_LESSEQ:
		os << "<=";
		this->truthValue = (lhsVal <= rhsVal) ? KNOWN_TRUE : KNOWN_FALSE;
		break;
	case E_EQUALS:
		os << "=";
		this->truthValue = (lhsVal == rhsVal) ? KNOWN_TRUE : KNOWN_FALSE;
		break;
	default:
		assert(false);
		break;
	}
	if (verbose) {
		cout << "Look up value for " << os.str() << ": " << this->truthValue << " based on " << lhsVal << " " << rhsVal
				<< "\n";
	}

}

void NewConditionController::visit_div_expression(div_expression *e) {
	e->getLHS()->visit(this);
	e->getRHS()->visit(this);
	NumScalar rhsVal = this->valueStack.top();
	this->valueStack.pop();
	NumScalar lhsVal = this->valueStack.top();
	this->valueStack.pop();
	this->valueStack.push(lhsVal / rhsVal);
}

void NewConditionController::visit_mul_expression(mul_expression *e) {
	e->getLHS()->visit(this);
	e->getRHS()->visit(this);
	NumScalar rhsVal = this->valueStack.top();
	this->valueStack.pop();
	NumScalar lhsVal = this->valueStack.top();
	this->valueStack.pop();
	this->valueStack.push(rhsVal * lhsVal);
}

void NewConditionController::visit_plus_expression(plus_expression *e) {
	e->getLHS()->visit(this);
	e->getRHS()->visit(this);
	NumScalar rhsVal = this->valueStack.top();
	this->valueStack.pop();
	NumScalar lhsVal = this->valueStack.top();
	this->valueStack.pop();
	this->valueStack.push(rhsVal + lhsVal);
}

void NewConditionController::visit_minus_expression(minus_expression *e) {
	e->getLHS()->visit(this);
	e->getRHS()->visit(this);
	NumScalar rhsVal = this->valueStack.top();
	this->valueStack.pop();
	NumScalar lhsVal = this->valueStack.top();
	this->valueStack.pop();
	this->valueStack.push(lhsVal - rhsVal);

}

void NewConditionController::visit_float_expression(float_expression *e) {
	this->valueStack.push(e->floating_value());
}

void NewConditionController::visit_int_expression(int_expression *e) {
	this->valueStack.push(e->floating_value());
}

void NewConditionController::visit_assignment(assignment *a) {
	// compute the rvalue
	a->getExpr()->visit(this);
	//FunctionKey key(intArgs);
	static VecInt intArgs;
	NumScalar currentValue = ws.fastFuncLookupByArgIndex(intArgs, a->getFTerm()->getHeadId(), a->getFTerm()->argMap,
			args);
	NumScalar newValue = 0;
	NumScalar rValue = this->valueStack.top();
	this->valueStack.pop();
	switch (a->getOp()) {
	case E_ASSIGN:
		newValue = rValue;
		break;
	case E_INCREASE:
		newValue = currentValue + rValue;
		break;
	case E_DECREASE:
		newValue = currentValue - rValue;
		break;
	default:
		assert(false);
		break;
	}
	ws.setValue(intArgs, newValue);
	//cout << newValue << "\n";
}

// observation
void NewConditionController::visit_observation(observation *r) {
	static const bool verbose = false;
	if (verbose)
		cout << "REV:" << r->receivers->size() << ": ";
	VecInt hasReceived(proc->nRoles, 0); // flag gets set to true as soon as at least one message is set to this role
	int i = 0;
	for (recipients::const_iterator itr = r->receivers->begin(); itr != r->receivers->end(); ++itr, ++i) {
		//cout << (*itr)->receivers->size() << " ";
		FunctionKey fk = getGroundedProp((*itr)->prop, this->args);
		if (verbose)
			cout << asIntString(fk) << " ";
		for (parameter_symbol_list::const_iterator pitr = (*itr)->receivers->begin(); pitr != (*itr)->receivers->end();
				++pitr) {
			//cout << ":";
			const string& name = (*pitr)->getName();
			if ((*pitr)->isConstant()) {
				if (name == "all") {
					if (verbose)
						cout << "msg " << i << " to all; ";
					hasReceived = VecInt(proc->nRoles, 1);
					for (unsigned k = 0; k < proc->nRoles; k++) {
						ws.obsListByPlyr[k].push_back(fk);
					}
					break;
				} else if (name != "others") { // Message is targeted to a player mentioned as a constant
					// Note that this is independent of args and could be computed in advance
					if (verbose)
						cout << "Msg " << i << " to " << (-1 - r->recipientMapper[name]) << " ("
								<< proc->invObjectTbl["role"][-1 - r->recipientMapper[name]] << ");";
					hasReceived[-1 - r->recipientMapper[name]] = 1;
					ws.obsListByPlyr[-1 - r->recipientMapper[name]].push_back(fk);
				}
			} else { // message is targeted to a player that is bound to a variable
				if (verbose)
					cout << "msg " << i << " to " << args[r->recipientMapper[name]] << " ("
							<< proc->invObjectTbl["role"][args[r->recipientMapper[name]]] << ");";
				hasReceived[args[r->recipientMapper[name]]] = 1;
				ws.obsListByPlyr[args[r->recipientMapper[name]]].push_back(fk);
			}
			//if (name != "all" && name != "others") cout << "Mapping " << name << " to " << r->recipientMapper[name] << "\n";
		}
	}
	i = 0;
	for (recipients::const_iterator itr = r->receivers->begin(); itr != r->receivers->end(); ++itr, ++i) {
		//cout << (*itr)->receivers->size() << " ";
		if ((*itr)->receivers->size() == 1 && (*(*itr)->receivers->begin())->getName() == "others") {
			FunctionKey fk = getGroundedProp((*itr)->prop, this->args);
			if (verbose)
				cout << asIntString(fk) << " ";
			if (verbose)
				cout << "Sending message to OTHERS: ";
			for (unsigned j = 1; j < hasReceived.size(); j++) {
				if (!hasReceived[j]) {
					if (verbose)
						cout << j << " ";
					ws.obsListByPlyr[j].push_back(fk);
				}
			}
			//cout << "\n";
		}
	}
	//cout << "\n";

}

FunctionKey NewConditionController::getGroundedProp(proposition* p, const VecInt& args) {
	//FunctionKey result(p->getHead()->getName(),VecInt(p->getArgs()->size()));
	//const parameter_symbol_list& psl = *p->getArgs();
	const VecInt& argMap = p->argMap;
	unsigned argsize = p->getArgs()->size();
	FunctionKey intArgs(argsize + 1); // = result.args;
	intArgs[0] = p->getHeadId();
	//cout << "GettingPredHeadId: " << intArgs[0] << "\n";
	for (unsigned i = 0; i < argsize; i++) {
		if (argMap[i] >= 0) {
			assert(argMap[i] < (int)args.size());
			intArgs[i + 1] = args[argMap[i]];
		} else if (argMap[i] == UVAL) {
			intArgs[i + 1] = UVAL; // Unknown val
		} else {
			intArgs[i + 1] = -1 - argMap[i];
		}
	}
	return intArgs;
}
}
;
