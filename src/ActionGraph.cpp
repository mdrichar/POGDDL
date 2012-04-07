#include "ActionGraph.h"
#include "NewConditionController.h"
#include "InfoSetGenerator.h"
#include "PerformanceCounters.h"
#include <sstream>
#include <iterator>
#include <climits>
using std::ostringstream;
using std::ostream_iterator;

const bool agverbose = true;
bool ActionGraph::externallySetSuperVerbose = false;

// Potentially woefully inefficient
bool isSubSetOf(SetInt& subset, VecInt& superset)
{
  for (SetInt::const_iterator itr = subset.begin(); itr != subset.end(); ++itr) {
    VecInt::iterator loc = find(superset.begin(),superset.end(),*itr);
    if (loc == superset.end()) {
      return false;
    }    
  }
  return true;
}

namespace VAL {

Processor* ActionGraph::gproc;
VecFluents ActionGraph::fluentHistory;
string Satisfiers::asString() {
  ostringstream os;
  if (!doable) os << "X";
  os << "  t: " << t << " -- ";
  //copy(candidates.begin(),candidates.end(),ostream_iterator<int>(os," "));
  for (unsigned j = 0; j < candidates.size(); j++) {
    os << ActionGraph::gproc->operatorIndexToString(candidates[j]) << "; ";
  }
  
  return os.str();
}

string FinallySatisfier::asString() {
  ostringstream os;
  if (resolved) os << "Resolved ";
  os << "fact: " << ActionGraph::gproc->getFact(fact) << "=" << ts << " at " << timeNeeded << " by\n";
  for (unsigned i = 0; i < satisfiers.size(); i++) {
    os << satisfiers[i].asString() << "\n";
  }
  return os.str();
}

void setMinus(SetInt& result, VecInt& candidates)
{
  for (VecInt::const_iterator itr = candidates.begin(); itr != candidates.end(); ++itr) {
    result.erase(*itr);
  }
}

void setIntersection(SetInt& a, SetInt& b) // finds the intersection and puts it back in a
{
  SetInt tmp;
  set_intersection(a.begin(),a.end(),
		   b.begin(),b.end(),
		   std::insert_iterator<SetInt>(tmp,tmp.begin()));
  a = tmp;
}

void setIntersection(SetInt& a, VecInt& b) // finds the intersection and puts it back in a
{
  SetInt tmp;
  tmp.insert(b.begin(),b.end());
  setIntersection(a,tmp);
}

const bool POST_CHECKS = true;
const bool NO_POST_CHECKS = false;
ActionGraphStage::ActionGraphStage()
  : isValid(false)
{

}

ActionGraph::ActionGraph(Processor* proc_, const VecVecVecKey& kb, const VecSetVecInt& opsByRev_, const WorldState& initialWorld_, unsigned k_)
  : proc(proc_), obs(kb), opsByRev(opsByRev_), initialWorld(initialWorld_), k(k_)
{
  assert (proc);
  gproc = proc;
  //k = 1;  // for now, do everything from player 1's perspective
}

// I/O
string ActionGraph::asString(bool brief)
{
  ostringstream os;
  for (unsigned i = 0; i < stages.size(); i++) {
    os << i << "\n" << asString(stages[i],brief) ;
  }
  if (!brief) {
    os << enumerateFinallySatisfiers() << "\n";
  }
  return os.str();
}

string ActionGraph::partialOpString(const VecInt& possOp)
{
  ostringstream os;
  if (possOp.empty()) {
    os << "(---) ";
  } else {
    os << "(" << proc->asString(possOp,OPERATOR) << ") ";
  }
  return os.str();
}

string ActionGraph::asString(const ActionGraphStage& ags, bool brief)
{
  ostringstream os;
  if (!brief) {
    if (!ags.prePos.empty()) 
       os << "PrePos:\n" << factSetString(ags.prePos) << "\n";
    if (!ags.preNeg.empty()) 
       os << "PreNeg:\n" << factSetString(ags.preNeg) << "\n";
  }
  os << "PartialOps: ";
  for (unsigned i = 0; i < ags.possOps.size(); i++) {
    os << partialOpString(ags.possOps[i]) << "; ";
  }
  os << "\n";
  os   << possibleActionsString(ags.possActions) << "\n";
  if (!brief) {
    if (!ags.postPos.empty()) 
       os << "Adds:\n" << factSetString(ags.postPos) << "\n";
    if (!ags.postNeg.empty()) 
       os << "Dels:\n" << factSetString(ags.postNeg) << "\n";
    if (!ags.possPostPos.empty()) 
       os << "Poss Adds:\n" << factSetString(ags.possPostPos) << "\n";
    if (!ags.possPostNeg.empty()) 
       os << "Poss Dels:\n" << factSetString(ags.possPostNeg) << "\n";
    if (!ags.flipPos.empty()) 
       os << "Flip Pos:\n" << factSetString(ags.flipPos) << "\n";
    if (!ags.flipNeg.empty()) 
       os << "Flip Neg:\n" << factSetString(ags.flipNeg) << "\n";
  }
  if (!brief) {
    //os << proc->printState(ags.partialWorld,false) << "\n";
    os << proc->printPartialState(ags.partialWorld) << "\n";
  }
  return os.str();
}

string ActionGraph::abbreviatedStagesAsStrings()
{
  ostringstream os;
  const bool PRINT_KNOWLEDGE_BASES = false;
  for (unsigned t = 1; t < stages.size(); t++) {
    os << "Stage " << t << "\n";
    os << proc->printState(stages[t].partialWorld, PRINT_KNOWLEDGE_BASES) << "\n";
    os << "Poss action as stage " << t << "\n";
    os << possibleActionsString(stages[t].possActions) << "\n";
  }
  return os.str();
}

string ActionGraph::possibleActionsString(const SetInt& possActions)
{
  ostringstream os;
  for (SetInt::const_iterator itr = possActions.begin(); itr != possActions.end(); itr++) {
     os << "  Legal: " << proc->operatorIndexToString(*itr) << "\n";
  }
  return os.str();
}

string ActionGraph::factVecSetString(const VecSetInt& facts)
{
  ostringstream os;
  for (unsigned i = 0; i < facts.size(); i++) {
    os << i << ":\n" << factSetString(facts[i]) << "\n";
  }
  return os.str();
}

string ActionGraph::factVecString(const VecInt& facts)
{
  ostringstream os;
  for (VecInt::const_iterator itr = facts.begin(); itr != facts.end(); ++itr) {
      os << "    " << proc->getFact(*itr) << "\n";
  }
  return os.str();
}

string ActionGraph::factSetString(const SetInt& facts)
{
  ostringstream os;
  for (SetInt::const_iterator itr = facts.begin(); itr != facts.end(); ++itr) {
      os << "    " << proc->getFact(*itr) << "\n";
  }
  return os.str();
}

void ActionGraph::markStageForReevaluation(unsigned i)
{
    if (i == 0 || i >= stages.size()) return; // Stage 0 is set in stone, but it's easier to check this here than to check it everywhere else to prevent the call to here
    assert (i < stages.size());
    this->stages[i].needsReevaluation = true; 
    this->atLeastOneStageNeedsReevaluation = true;
}

void ActionGraph::markAllStagesForReevaluation()
{
  for (unsigned i = 1; i < stages.size(); i++) {
    markStageForReevaluation(i);
  }
}

// This assumes that conflicts are not possible -- i.e., during the initialization phase when no assumptions are made
void ActionGraph::reevaluateUntilFixedPoint()
{
  static const bool verbose = false;
  unsigned outerLoopIterCount = 0;
  while (this->atLeastOneStageNeedsReevaluation) {
    ++outerLoopIterCount;
    this->atLeastOneStageNeedsReevaluation = false;
    WorldState currentPartial;
    //bool result;
    for (unsigned t = 1; t < stages.size(); t++) {
      if (stages[t].needsReevaluation) {
        stages[t].needsReevaluation = false;
        if (verbose) cout << outerLoopIterCount << " AAA Reevaluating: " << t << std::endl;
        ActionGraphStage& a = stages[t]; 
        currentPartial = stages[t-1].partialWorld; // Need to make a copy so I don't overwrite the one that's there
        if (a.possActions.size() >= 1) {
          identifyNonConflictingActions(a.possOps, currentPartial, a);
        }
        assert (!a.possOps.empty());
        compareKnownTruthWithNeighbors(t);
      }
    }
  }
}
// Perform initialization according to thesis proposal pseudocode
// without explicitly worrying about the possible candidates
// for "finally satisfying" a transition
void ActionGraph::initializeGraphSimple()
{
  static const bool verbose = false;
  initStages(obs,opsByRev,initialWorld);
  //sandbox(obs,opsByRev);
  if (verbose) cout << "Initial Action Graph:\n" 
       << asString() << "\n";
  markAllStagesForReevaluation();
  reevaluateUntilFixedPoint();
  if (verbose) cout << "Final Action Graph:\n" 
       << asString(false) << "\n";
  //exit(-1); // For debugging purposes; I want the program to stop after the action graph is initialized
}

void ActionGraph::initializeGraph()
{
  // This version assumes that there can be no conflicts and does not do anything about
  // finally satisfiers
  initializeGraphSimple();
  return;
  initStages(obs,opsByRev,initialWorld);
  //sandbox(obs,opsByRev);
  if (agverbose) cout << "Initial Action Graph:\n" 
       << asString() << "\n";
  int iteration = 0;
  while (foundNewRamifications()) {
    if (agverbose) cout << "FOUND NEW RAMIFICATIONS ON ITERATION " << iteration << "; CONTINUING...\n";
	//break;
    resolveFinallySatisfiers();
    unsigned timesPruned = 1;
    while (pruneFinallySatisfiers()) {
      if (agverbose) cout << "After pruning " << timesPruned << "\n";
      timesPruned++;
      resolveFinallySatisfiers();
    }
    reevaluatelegalActions();
    //cout << "Situation after iteration " << iteration << "\n";
    //cout << asString() << "\n";
    iteration++;
    //if (iteration > 2) {
    //  cout << "EMERGENCY BRAKE\n";
    //  break;
    //}
  }
  //findAllUnknownToKnownTransitions();
  //findAllUnknowns();
  
  if (agverbose) cout << "Final Action Graph:\n" 
       << asString(false) << "\n";
}

void ActionGraph::identifySatisfiers(int fact, TruthState ts, unsigned t, unsigned current)
{
  unsigned candt = current;
  FinallySatisfier fs(fact,ts,t);
  while(true) {
    assert (candt < stages.size()); 
    //VecInt tSatisfiers = getSatisfiers(fact,ts,candt);  
    if (candt == 0) {
      if (stages[0].partialWorld.getTruthValue(fact) == ts) {
        if (agverbose) cout << "Fact " << proc->getFact(fact) << " can be made " << ts << " at start state\n";
        Satisfiers s(0,VecInt());
        fs.satisfiers.push_back(s);
      }
      break;
    } else {
      TruthState candTS = stages[candt].partialWorld.getTruthValue(fact);
      if (candTS != UNKNOWN && candTS != ts) { 
        // Then it is known to be at candt exactly the opposite of what it needs to be
        // at t.  So the finally satisfier cannot be any futher back than this.
	break;
      } else {
        VecInt cands = getSatisfiers(fact,ts,candt);
        if (!cands.empty()) {
          if (agverbose) cout << "Fact " << proc->getFact(fact) << " can be made " << ts << " at time " << candt << " by " << cands.size() << " actions\n";
          fs.satisfiers.push_back(Satisfiers(candt,cands));
        }
      }
      candt--; 
    }
  }
  this->finallySatisfiers.push_back(fs);
}

bool ActionGraph::reevaluatelegalActions()
{
  static const bool verbose = false;
  WorldState currentPartial;
  bool result;
  if (verbose) cout << "Reevaluating; " << stages.size() << " stages.\n";
  for (unsigned t = 1; t < stages.size(); t++) {
    ActionGraphStage& a = stages[t]; 
    currentPartial = stages[t-1].partialWorld; // Need to make a copy so I don't overwrite the one that's there
    if (a.possActions.size() == 1) {
      if (verbose) cout << t << " Single Op (" << proc->operatorIndexToString(*a.possActions.begin()) << ")\n";
      result = oneLegalMove(currentPartial,a);
      //derivePreservation(t);
      //continue; // I already know exactly which action happened at this step
    } else {
      //VecInt& possOp = stages[t].possOp;
      //assert(!possOp.empty()); // should never have been set to this if it's empty 
      //if (verbose) cout << t << " Reconsidering (" << proc->asString(possOp,OPERATOR) << ")\n";
      result = legalOperators(stages[t].possOps, currentPartial, a);
    }
    if (!result) {
      if (verbose) cout << "Conflict detected in reevaluateLegalActions\n";
      return false; // Some kind of conflict was generated
    }
  }
  return true; // No conflicts were found
}

bool ActionGraph::ramify(Ramification r)
{
  unsigned nextTime = r.time;
  while(true) {
    assert (r.time < stages.size());
    if (agverbose) cout << "Ramifying: " << proc->getFact(r.fact) << " = " << r.truthState << " at time " << nextTime << "\n";
    ActionGraphStage& ags = stages[nextTime];
    ags.partialWorld.setTruthValue(r.fact,r.truthState);
    // If the fact can be satisfied at this stage, figure out the candidates for finally satisfying it
    if (r.truthState == KNOWN_TRUE && ags.possPostPos.find(r.fact) != ags.possPostPos.end()) {
      if (agverbose) cout << "Stopping because this fact could be made true at time " << r.time << "\n";
      identifySatisfiers(r.fact,r.truthState,r.time,nextTime);
      break;
    } else if (r.truthState == KNOWN_FALSE && ags.possPostNeg.find(r.fact) != ags.possPostNeg.end()) {
      identifySatisfiers(r.fact,r.truthState,r.time,nextTime);
      if (agverbose) cout << "Stopping because this fact could be made false at time " << r.time << "\n";
      break;
    }
    // Figure out if we need to recurse
    nextTime = (r.direction == BACKWARD) ? nextTime - 1 : nextTime + 1;
    if (nextTime == 0 || nextTime >= stages.size()) {
      break; 
    }
    ActionGraphStage& ngs = stages[nextTime];
    WorldState& ws = ngs.partialWorld;
    TruthState nextTruthState = ws.getTruthValue(r.fact);
    if (nextTruthState == r.truthState) {
      break;
      // We've backed up to a point where the truth state is known and consistent with what we need
    } else if (nextTruthState == UNKNOWN) {
      continue; // Keep propagating what we know
    } else {
      cout << "Found conflict\n"; // This could be a bug or could just mean that an assumption made elsewhere has turned out to be incorrect, since if we get here it should mean that no opportunity to flip this fact to the way it needs to be at r.time has presented itself up to this point, and now we have hard evidence that it should be the other value.
      return false;
    }
  } 
  return true; // Did not find any evidence that conflicts with what we're proopagated
}
bool ActionGraph::foundNewRamifications()
{
  bool foundAtLeastOne = false;
  if (agverbose) cout << "ENUMERATING RAMIFICATIONS\n"; 
  for (unsigned t = 1; t < stages.size(); t++) {
    ActionGraphStage& s = stages[t];
    //WorldState& after = s.partialWorld;
    WorldState& before = stages[t-1].partialWorld;
    // Find any known preconditions for which the state is UNKNOWN at t-1
    for (SetInt::const_iterator itr = s.prePos.begin(); itr != s.prePos.end(); ++itr) {
      int factNum = *itr;
      if (before.getTruthValue(factNum) == UNKNOWN) {
        foundAtLeastOne = true;
        if (agverbose) cout << "T: " << t << " PosPre: " << proc->getFact(factNum) << "\n";
	ramify(Ramification(factNum,KNOWN_TRUE,BACKWARD,t-1));
      }
    }
    // TODO also propagate negative ramifications
    for (SetInt::const_iterator itr = s.preNeg.begin(); itr != s.preNeg.end(); ++itr) {
      int factNum = *itr;
      if (before.getTruthValue(factNum) == UNKNOWN) {
        foundAtLeastOne = true;
        if (agverbose) cout << "T: " << t << " NegPre: " << proc->getFact(factNum) << "\n";
	ramify(Ramification(factNum,KNOWN_FALSE,BACKWARD,t-1));
      }
    }
  } 
  return foundAtLeastOne;
}

void ActionGraph::initStages(const VecVecVecKey& obs, const VecSetVecInt& opsByRev, const WorldState& initialWorld)
{
  assert (k > 0);
  assert (k < initialWorld.getNRoles());
  unsigned T = obs.size();
  WorldState currentPartial = initialWorld;
  // Initialize a stage for the initial state, s0.  There is no action 0, and so all the effects and preconditions are empty
  stages.resize(T+1); // +1 to support a stage for the initial state
  stages[0].partialWorld = initialWorld; // copy the initial world state here
  stages[0].isValid = true; // The intial stage only needs the s0 partial World (root state) to be valid
  stages[0].timeStep = 0;
  for (unsigned t = 1; t <= T; t++) {
    //if (t==5) exit(0);
    ActionGraphStage& a = stages[t]; // Just to reduce verbosity on next line
    a.timeStep = t;
    a.partialWorld.initUnknown(initialWorld.maxPred,initialWorld.getNRoles());
    assert (t < ActionGraph::fluentHistory.size());
    a.partialWorld.setFluents(ActionGraph::fluentHistory[t]);
    
    const VecKey& currentObs = obs[t-1][k]; // t - 1 because the observation list is 0 based -- Sorry!
    assert (currentObs.size() == 1); // Only deal with one observation per time step right now
    unsigned revId = currentObs[0][0];
    assert (revId < opsByRev.size()); //
      if (agverbose) cout << "Verbose Possibilities: ";
      for (SetVecInt::const_iterator itr = opsByRev[revId].begin(); itr != opsByRev[revId].end(); ++itr) {
        VecInt possOp = proc->getPartialOperator(currentObs[0],*itr);
        if (!possOp.empty()) { 
          stages[t].possOps.push_back(possOp);
          if (agverbose) cout << "(" << proc->asString(possOp,OPERATOR) << ") ";
        }
      }
      if (agverbose) cout << std::endl;
    if (agverbose) cout << "k" << k << " t" << t << ": [" << proc->asString(currentObs[0],NUMERIC) << "]\t\t\t Possibilities: ";
    //assert (opsByRev[revId].size() == 1); // I cannot yet handle the case where there is more than one possible operator schema for an obs; basically you need to run legalOps on both and take intersections and unions of things
    //for (SetVecInt::const_iterator itr = opsByRev[revId].begin(); itr != opsByRev[revId].end(); ++itr) {
    //  stages[t].possOp = proc->getPartialOperator(currentObs[0],*itr);
    //  VecInt& possOp = stages[t].possOp;
    //  if (possOp.empty()) continue; // Right now I can only handle one NON-EMPTY possOp per rev, but there could be some bogus ones that don't work
    //  if (agverbose) cout << "(" << proc->asString(possOp,OPERATOR) << ") ";
      currentPartial = stages[t-1].partialWorld; // Need to make a copy so I don't overwrite the one that's there
      //ActionGraph::externallySetSuperVerbose = false; //(t == 41);
      bool consistent = legalOperators(stages[t].possOps, currentPartial,a);
      if (!consistent) {
	cout << "Not consistent in legalOperators in initStages" << std::endl;
	exit(-1);
      }
      assert (consistent);
      if (agverbose) cout << "INITIAL SETTINGS FOR STAGE " << t << "\n" 
	   << asString(a,false) << "\n";
      assert(!a.possActions.empty());
      assert(!a.prePos.empty());
    //  break; // If I get this far, I have found one possOp that works; TODO: be able to handle more than one
    //}
  }
}

void collapsePreconditions(const VecSetInt& multivec, const SetInt& universal, VecInt& vec)
{
  vec.clear();
  vec.insert(vec.end(),universal.begin(),universal.end());
  for (unsigned i = 0; i < multivec.size(); i++) {
    vec.insert(vec.end(),multivec[i].begin(),multivec[i].end());
  }
}

bool ActionGraph::oneLegalMove(WorldState& preWorld, ActionGraphStage& ags)
{
  assert (ags.possActions.size() == 1);
  assert (ags.isValid);
  bool result;
  // sanity checks

  NewConditionController ncc(proc,preWorld);
  ncc.tracking = true;
  int index = *ags.possActions.begin();
  operator_* op = proc->getOpFromIndex(index);
  ncc.args = VecInt (op->getArgSizeNeeded(),-1);
  string dummy;
  proc->decodeOperator(index,dummy,ncc.args);
  assert (ncc.passesScreeningChecks(op,-1));
  assert (ags.possOps.size() > 0);
  // Process preconditions
  op->precondition->visit(&ncc);
  if (ncc.truthValue == KNOWN_FALSE) return false; // It doesn't work to do this action here, some other action assumption is wrong
  if (!ags.possPostPos.empty() || !ags.possPostNeg.empty()) { // Then there were some previously ambiguous effects at this action slot; once we fix one action, everything is deterministic
    ags.prePos.clear();
    ags.preNeg.clear();
    //for (unsigned i = 0; i < ags.possOp.size()-1; i++) {
    for (unsigned i = 0; i < ncc.args.size(); i++) { // Changed reference for #args from possOp to args
      if (!ncc.passesScreeningChecks(op,i)) return false;
      ags.prePos.insert(ncc.posScreeners.begin(),ncc.posScreeners.end());
      ags.preNeg.insert(ncc.negScreeners.begin(),ncc.negScreeners.end());
    }
    preWorld.clearUpdates();
    proc->apply(index,preWorld);
    ags.postPos.clear();
    ags.postPos.insert(preWorld.addFacts.begin(),preWorld.addFacts.end());
    ags.postNeg.clear();
    ags.postNeg.insert(preWorld.delFacts.begin(),preWorld.delFacts.end());
    ags.possPostPos.clear();
    ags.possPostNeg.clear();
  }
  result = applyEffects(preWorld, ags);
  return result;
}

// Identify possible actions at ags -- those whose preconditions to not conflict with what is known at preWorld
// and whose effects do not conflict with what is known at ags
void ActionGraph::identifyNonConflictingActions(VecVecInt& partialOps, WorldState& preWorld, ActionGraphStage& ags)
{
  ++PerformanceCounters::graphNonConflicting;
  static const bool verbose = false;
  bool superVerbose = false;
  static int timesCalled = 0;
  ++timesCalled;
  if (verbose) cout << "identifyNonConflicting called " << timesCalled << std::endl;
  //if (timesCalled == 740) {
  //  cout << "Problematic iteration" << std::endl;
  //  superVerbose = true;
  //}
  // sanity checks
  WorldState& resultingState = ags.partialWorld;
  assert (ags.isValid);

  NewConditionController ncc(proc,preWorld);
  ncc.tracking = true;
  SetInt unionPostPos;
  SetInt unionPostNeg;
    
  bool firstLegalOp = true;
  VecInt legalActions; 
  for (unsigned possIndex = 0; possIndex < partialOps.size(); possIndex++) {
    VecInt& partialOp = partialOps[possIndex];
    assert(!partialOp.empty());
    assert(partialOp[0] < (int)proc->vecOps.size());
    // initializers
    operator_* op = proc->vecOps[partialOp[0]];
    VecInt& argMaxes = op->argMaxes;
    int nParams = (int)op->parameters->size();
    assert (nParams == (int)(partialOp.size() -1));
    VecSetInt currentPrePos(nParams);
    VecSetInt currentPreNeg(nParams);
    ncc.args = VecInt (op->getArgSizeNeeded(),-1);
    assert (ncc.passesScreeningChecks(op,-1));
  
    int ptr = 0;
    while (ptr >= 0) {
      //assert (NewConditionController::tracking);
      if (ptr >= (int)nParams) {
        ncc.truthValue = KNOWN_FALSE;
        op->precondition->visit(&ncc);
        if (ncc.truthValue != KNOWN_FALSE) { // We cannot prove that it is impossible to execute this action at this stage
          int index = proc->getOperatorIndex(op->name->getName(),ncc.args);
	  if (superVerbose) cout << "Action considered: " << index << " " << proc->operatorIndexToString(index) << "\n";
          if (ags.isValid && ags.possActions.find(index) == ags.possActions.end()) {  // This action was somehow disqualified earlier.  Restrictions can only increase
            if (agverbose) cout << "Action disqualified earlier: " << proc->operatorIndexToString(index) << "\n";
            ptr--;
            continue;
          }
          preWorld.clearUpdates();
          proc->apply(index,preWorld);
          bool applicable = true;
          if (ags.isValid) { // This means that ags holds legitimate partial state information, and we need to check the post conditions of this action to see if they conflict
  	    assert (resultingState.maxPred == preWorld.maxPred);
            applicable = true; // postconditions do not conflict with ags.partialWorld
            for (VecInt::const_iterator itr = preWorld.addFacts.begin(); itr != preWorld.addFacts.end(); ++itr) {
              if (resultingState.getTruthValue(*itr) == KNOWN_FALSE) {
                if (verbose) cout << "Add effect Conflict detected: " << proc->getFact(*itr) << "\n";
  		applicable = false;
		//assert(false);
                break;
              } 
            }
            if (applicable) {
              for (VecInt::const_iterator itr = preWorld.delFacts.begin(); itr != preWorld.delFacts.end(); ++itr) {
  	        if (resultingState.getTruthValue(*itr) == KNOWN_TRUE) {
                  if (verbose) {
  	            cout << "Del effect Conflict detected: " << proc->getFact(*itr) << "\n";
  	            cout << "PREWORLD\n" << proc->printState(preWorld, false) << "\nPOSTWORLD\n" << proc->printState(resultingState, false) << std::endl;
  	            //exit(-1);
    	          }
  	          applicable = false;
	            //assert(false);
                    break;
                }
              }
            }
	    if (applicable) {
		applicable = isSubSetOf(ags.flipPos,preWorld.addFacts);
		if (verbose && !applicable) {
                  cout << "Eliminating " << proc->operatorIndexToString(index) << " because of missing add" << std::endl;
		  cout << "FlipPos: " << factSetString(ags.flipPos) << std::endl;
                }
            }
	    if (applicable) {
		applicable = isSubSetOf(ags.flipPos,preWorld.addFacts);
		if (verbose && !applicable) {
		  cout << "Eliminating " << proc->operatorIndexToString(index) << " because of missing del" << std::endl;
		  cout << "FlipNeg: " << factSetString(ags.flipNeg) << std::endl;
		}
            }
          }
          // Make sure that additional constraints for finally satisfiers are also fulfilled
          SetInt tmp;
          set_intersection(ags.otherPos.begin(),ags.otherPos.end(),
  			 preWorld.addFacts.begin(),preWorld.addFacts.end(),
  			 std::insert_iterator<SetInt>(tmp,tmp.begin()));
  	if (tmp != ags.otherPos) {
  	  if (agverbose) cout << "Required condition not satisfied for " << proc->operatorIndexToString(index) << "\n";
            applicable = false;
          }
          // for each positive addtl constraint
          //	Make sure it is NOT in preWorld.delFacts AND that it is in preWorld.addFacts || is preWorld
          // for each negative addtl constraint
  	// 	Make sure it is NOT in preWorld.addFacts AND that it is in preWorld.delFacts || is in preWorld
          if (applicable) {
            legalActions.push_back(index);
  	    VecInt collapsedPos, collapsedNeg;
            //cout << "Uncollapsed positive preoconditions\n" << factVecSetString(currentPrePos) << "\n";
            collapsePreconditions(currentPrePos,op->prePos,collapsedPos);
  	  //cout << "Collapsed positive preconditions\n" << factVecString(collapsedPos) << "\n";
            collapsePreconditions(currentPreNeg,op->preNeg,collapsedNeg);
            if (firstLegalOp) {
  	      ags.prePos.clear();
              ags.prePos.insert(collapsedPos.begin(),collapsedPos.end());
  	      ags.preNeg.clear();
              ags.preNeg.insert(collapsedNeg.begin(),collapsedNeg.end());
              ags.postPos.insert(preWorld.addFacts.begin(),preWorld.addFacts.end());
              ags.postNeg.insert(preWorld.delFacts.begin(),preWorld.delFacts.end());
              unionPostPos = ags.postPos;
              unionPostNeg = ags.postNeg;
  	    //union of possible effects
  
            } else {
              setIntersection(ags.prePos, collapsedPos);
              setIntersection(ags.preNeg, collapsedNeg); 
              setIntersection(ags.postPos, preWorld.addFacts);
              setIntersection(ags.postNeg, preWorld.delFacts);
              unionPostPos.insert(preWorld.addFacts.begin(),preWorld.addFacts.end());
              unionPostNeg.insert(preWorld.delFacts.begin(),preWorld.delFacts.end());
            }
            if (agverbose) cout << ags.timeStep << " Identify Legal: " << proc->operatorIndexToString(index) << "\n";
  	    firstLegalOp = false;
          } else {
  	    if (verbose) {
  	    cout << "Preconditions satisfied but something else wrong for " << proc->operatorIndexToString(index) << "\n";
  	    
  	    
            }
          }
        } else {
	        //if (superVerbose) cout << "Not Legal: " << index << " " << proc->operatorIndexToString(index) << "\n";
        } 
        ptr--;
      } else if ((partialOp[ptr+1] < 0 && ncc.args[ptr]+1 >= argMaxes[ptr]) || 
                 (partialOp[ptr+1] >= 0 && ncc.args[ptr] == partialOp[ptr+1])) {
        ncc.args[ptr] = -1;
        ptr--;
      } else {
        if (partialOp[ptr+1] < 0) {
          ncc.args[ptr]++;
        } else {
          ncc.args[ptr] = partialOp[ptr+1];
        }
        // If the minimum conditions are met, then...
        if (ncc.passesScreeningChecks(op,ptr)) {
          currentPrePos[ptr] = ncc.posScreeners;
          //cout << "PosScreeners: " << factSetString(ncc.posScreeners) << "\n";
          currentPreNeg[ptr] = ncc.negScreeners;
          ptr++;
        } 
      }
    } //while ptr > 0
  } // for each poss ops
  // compute the set difference of the possible effects and common effects
  if (legalActions.empty()) {
    if (ActionGraph::externallySetSuperVerbose) {
      cout << "FailureComparison\nBefore\n" << proc->printPartialState(preWorld) << "\nAfter\n" << proc->printPartialState(ags.partialWorld) << std::endl;
    }
    assert (false); //<-- Uncomment this line if the algorithm should fail irrepairably when assumptions are false
    //return false; // This means we have made a bad assumption somewhere along the line 
  }
  ags.possActions.clear(); 
  ags.possActions.insert(legalActions.begin(),legalActions.end());
  if (verbose) {
    cout << "Non-conflicting: ";
    for (unsigned i = 0; i < legalActions.size(); i++) {
      cout << proc->operatorIndexToString(legalActions[i]) << "; ";
    }
    cout << "\n";
  }
  ags.possPostPos.clear();
  ags.possPostNeg.clear();
  set_difference(unionPostPos.begin(),unionPostPos.end(),
		 ags.postPos.begin(),ags.postPos.end(),
		 std::insert_iterator<SetInt>(ags.possPostPos,ags.possPostPos.begin()));
  set_difference(unionPostNeg.begin(),unionPostNeg.end(),
		 ags.postNeg.begin(),ags.postNeg.end(),
		 std::insert_iterator<SetInt>(ags.possPostNeg,ags.possPostNeg.begin()));
}
bool ActionGraph::legalOperators(VecVecInt& partialOps, WorldState& preWorld, ActionGraphStage& ags)
{
  ++PerformanceCounters::graphLegalOperators;
  static int timesCalled = 0;
  static const bool verbose = false;
  ++timesCalled;
  if (verbose) cout << "LegalOperators called " << timesCalled << std::endl;
  bool result;
  // sanity checks
  WorldState& resultingState = ags.partialWorld;

  NewConditionController ncc(proc,preWorld);
  ncc.tracking = true;
  SetInt unionPostPos;
  SetInt unionPostNeg;
    
  bool firstLegalOp = true;
  VecInt legalActions; 
  for (unsigned possIndex = 0; possIndex < partialOps.size(); possIndex++) {
    VecInt& partialOp = partialOps[possIndex];
    assert(!partialOp.empty());
    assert(partialOp[0] < (int)proc->vecOps.size());
    // initializers
    operator_* op = proc->vecOps[partialOp[0]];
    VecInt& argMaxes = op->argMaxes;
    int nParams = (int)op->parameters->size();
    assert (nParams == (int)(partialOp.size() -1));
    VecSetInt currentPrePos(nParams);
    VecSetInt currentPreNeg(nParams);
    ncc.args = VecInt (op->getArgSizeNeeded(),-1);
    assert (ncc.passesScreeningChecks(op,-1));
  
    int ptr = 0;
    while (ptr >= 0) {
      //assert (NewConditionController::tracking);
      if (ptr >= (int)nParams) {
        ncc.truthValue = KNOWN_FALSE;
        op->precondition->visit(&ncc);
        if (ncc.truthValue != KNOWN_FALSE) { // We cannot prove that it is impossible to execute this action at this stage
          int index = proc->getOperatorIndex(op->name->getName(),ncc.args);
          if (ags.isValid && ags.possActions.find(index) == ags.possActions.end()) {  // This action was somehow disqualified earlier.  Restrictions can only increase
            if (agverbose) cout << "Action disqualified earlier: " << proc->operatorIndexToString(index) << "\n";
            ptr--;
            continue;
          }
          preWorld.clearUpdates();
          proc->apply(index,preWorld);
          bool applicable = true;
          if (ags.isValid) { // This means that ags holds legitimate partial state information, and we need to check the post conditions of this action to see if they conflict
  	  assert (resultingState.maxPred == preWorld.maxPred);
            applicable = true; // postconditions do not conflict with ags.partialWorld
            for (VecInt::const_iterator itr = preWorld.addFacts.begin(); itr != preWorld.addFacts.end(); ++itr) {
              if (resultingState.getTruthValue(*itr) == KNOWN_FALSE) {
                  if (verbose) cout << "Add effect Conflict detected: " << proc->getFact(*itr) << "\n";
  		applicable = false;
                  break;
              } 
            }
            if (applicable)
            for (VecInt::const_iterator itr = preWorld.delFacts.begin(); itr != preWorld.delFacts.end(); ++itr) {
  	    if (resultingState.getTruthValue(*itr) == KNOWN_TRUE) {
                  if (verbose) {
  		  cout << "Del effect Conflict detected: " << proc->getFact(*itr) << "\n";
  		  cout << "PREWORLD\n" << proc->printState(preWorld, false) << "\nPOSTWORLD\n" << proc->printState(resultingState, false) << std::endl;
  		  //exit(-1);
    		}
  		applicable = false;
                  break;
              }
            }
          }
          // Make sure that additional constraints for finally satisfiers are also fulfilled
          SetInt tmp;
          set_intersection(ags.otherPos.begin(),ags.otherPos.end(),
  			 preWorld.addFacts.begin(),preWorld.addFacts.end(),
  			 std::insert_iterator<SetInt>(tmp,tmp.begin()));
  	if (tmp != ags.otherPos) {
  	  if (agverbose) cout << "Required condition not satisfied for " << proc->operatorIndexToString(index) << "\n";
            applicable = false;
          }
          // for each positive addtl constraint
          //	Make sure it is NOT in preWorld.delFacts AND that it is in preWorld.addFacts || is preWorld
          // for each negative addtl constraint
  	// 	Make sure it is NOT in preWorld.addFacts AND that it is in preWorld.delFacts || is in preWorld
          if (applicable) {
            legalActions.push_back(index);
  	  VecInt collapsedPos, collapsedNeg;
            //cout << "Uncollapsed positive preoconditions\n" << factVecSetString(currentPrePos) << "\n";
            collapsePreconditions(currentPrePos,op->prePos,collapsedPos);
  	  //cout << "Collapsed positive preconditions\n" << factVecString(collapsedPos) << "\n";
            collapsePreconditions(currentPreNeg,op->preNeg,collapsedNeg);
            if (firstLegalOp) {
  	    ags.prePos.clear();
              ags.prePos.insert(collapsedPos.begin(),collapsedPos.end());
  	    ags.preNeg.clear();
              ags.preNeg.insert(collapsedNeg.begin(),collapsedNeg.end());
              ags.postPos.insert(preWorld.addFacts.begin(),preWorld.addFacts.end());
              ags.postNeg.insert(preWorld.delFacts.begin(),preWorld.delFacts.end());
              unionPostPos = ags.postPos;
              unionPostNeg = ags.postNeg;
  	    //union of possible effects
  
            } else {
              setIntersection(ags.prePos, collapsedPos);
              setIntersection(ags.preNeg, collapsedNeg); 
              setIntersection(ags.postPos, preWorld.addFacts);
              setIntersection(ags.postNeg, preWorld.delFacts);
              unionPostPos.insert(preWorld.addFacts.begin(),preWorld.addFacts.end());
              unionPostNeg.insert(preWorld.delFacts.begin(),preWorld.delFacts.end());
            }
            if (agverbose) cout << ags.timeStep << "  Legal: " << proc->operatorIndexToString(index) << "\n";
  	  firstLegalOp = false;
          } else {
  	  if (verbose) {
  	    cout << "Preconditions satisfied but something else wrong for " << proc->operatorIndexToString(index) << "\n";
  	    
  	    
            }
          }
        } 
        ptr--;
      } else if ((partialOp[ptr+1] < 0 && ncc.args[ptr]+1 >= argMaxes[ptr]) || 
                 (partialOp[ptr+1] >= 0 && ncc.args[ptr] == partialOp[ptr+1])) {
        ncc.args[ptr] = -1;
        ptr--;
      } else {
        if (partialOp[ptr+1] < 0) {
          ncc.args[ptr]++;
        } else {
          ncc.args[ptr] = partialOp[ptr+1];
        }
        // If the minimum conditions are met, then...
        if (ncc.passesScreeningChecks(op,ptr)) {
          currentPrePos[ptr] = ncc.posScreeners;
          //cout << "PosScreeners: " << factSetString(ncc.posScreeners) << "\n";
          currentPreNeg[ptr] = ncc.negScreeners;
          ptr++;
        } 
      }
    } //while ptr > 0
  } // for each poss ops
  // compute the set difference of the possible effects and common effects
  if (legalActions.empty()) {
    //assert (false); <-- Uncomment this line if the algorithm should fail irrepairably when assumptions are false
    return false; // This means we have made a bad assumption somewhere along the line 
  }
  ags.possActions.clear(); 
  ags.possActions.insert(legalActions.begin(),legalActions.end());
  ags.possPostPos.clear();
  ags.possPostNeg.clear();
  set_difference(unionPostPos.begin(),unionPostPos.end(),
		 ags.postPos.begin(),ags.postPos.end(),
		 std::insert_iterator<SetInt>(ags.possPostPos,ags.possPostPos.begin()));
  set_difference(unionPostNeg.begin(),unionPostNeg.end(),
		 ags.postNeg.begin(),ags.postNeg.end(),
		 std::insert_iterator<SetInt>(ags.possPostNeg,ags.possPostNeg.begin()));
  result = applyEffects(preWorld, ags);
  ags.isValid = true;
  return result;
}

VecInt ActionGraph::getSatisfiers(int fact, TruthState ts, unsigned t)
{
  assert ( t < stages.size());
  assert (stages[t].isValid);
  assert (stages[t-1].isValid);
  WorldState& before = stages[t-1].partialWorld;
  SetInt& actions = stages[t].possActions;
  VecInt result;
  for (SetInt::const_iterator itr = actions.begin(); itr != actions.end(); ++itr) {
    before.clearUpdates();
    proc->apply(*itr,before);
    //cout << "Applying : " << proc->operatorIndexToString(*itr) << "\n";
    if (before.modifyingFact(fact,ts)) {
      result.push_back(*itr); 
    }
  }
  return result;
}
void ActionGraph::compareKnownTruthWithNeighbors(unsigned t)
{
  ++PerformanceCounters::graphCompareValues;
  //ags.partialWorld = preWorld; // Copy it over to initialize it to have the right length and 
  static const bool verbose = false;
  if (verbose) cout << "Comparing: " << t << std::endl;
  assert (t > 0);
  ActionGraphStage& ags = stages[t];
  WorldState& preWorld = stages[t-1].partialWorld;
  WorldState& effWorld = stages[t].partialWorld;

  //WorldState addEffs;
  //for (
  //WorldState delEffs;
  //WorldState posPres;
  //WorldState negPres;
  WorldState& result = ags.partialWorld;
  assert (preWorld.maxPred == result.maxPred);
  // Efficiency improvement: have a vector for these values
  // Iterate through each list once and fill in the corresponding vector
  // Then either iterate over all the facts after that (with all flags known already)
  // or perform vector operations
  for (unsigned i = 0; i <= preWorld.maxPred; i++) {
    bool inPrePos = (ags.prePos.find(i) != ags.prePos.end());
    bool inPreNeg = (ags.preNeg.find(i) != ags.preNeg.end());
    bool inAdds = (ags.postPos.find(i) != ags.postPos.end());
    bool inDels = (ags.postNeg.find(i) != ags.postNeg.end());
    bool inPossAdds = (ags.possPostPos.find(i) != ags.possPostPos.end());
    bool inPossDels = (ags.possPostNeg.find(i) != ags.possPostNeg.end());
    bool inNone = (!inAdds && !inDels && !inPossAdds && !inPossDels);
    assert (!inDels || !inPossAdds);
    assert (!inAdds || !inPossDels);
    TruthState after = result.getTruthValue(i);
    TruthState before = preWorld.getTruthValue(i);
    if (ActionGraph::externallySetSuperVerbose) {
      printf("%d Fact: %s PreP: %d PreNeg: %d Adds: %d Dels: %d P+: %d P-: %d N: %d before: %d after: %d\n",i,proc->getFact(i).c_str(),inPrePos,inPreNeg,inAdds,inDels,inPossAdds,inPossDels,inNone,before,after);
    }
    // consistency checks -- if we've made a bad assumption about what a certain action must/must not be, we could discover the conflict here
    if (after != UNKNOWN && before != UNKNOWN) {
      if (before == KNOWN_TRUE) {
        if (after == KNOWN_TRUE) {
	  assert(!inDels); // need to stay true but we aren't
        } else { // (after == KNOWN_FALSE) 
	  assert(inDels || inPossDels);
	  if (!inDels) {
            ags.flipNeg.insert(i); 
	    if (verbose) cout << "FlipNeging " << proc->getFact(i) << std::endl;
	    markStageForReevaluation(t);
          }
        }
      } else { // befor == KNOWN_FALSE
        if (after == KNOWN_TRUE) {
	  assert(inAdds || inPossAdds);
	  if (!inAdds) {
            ags.flipPos.insert(i); 
	    if (verbose) cout << "FlipPosing" << proc->getFact(i) << std::endl;
	    markStageForReevaluation(t);
          }
        } else { // after == KNOWN_FALSE
	  assert(!inAdds);
        }
      }
    } // end of consistency check
	// Update knowledge gleaned from add and delete lists
    if (inDels && after == UNKNOWN) {
	  effWorld.setTruthValue(i,KNOWN_FALSE);      
	  markStageForReevaluation(t+1);
	  if (verbose) cout << "inDels implies that " << proc->getFact(i) << " must be FALSE at time " << t << std::endl;
    }
    if (inAdds && after == UNKNOWN) {
	  effWorld.setTruthValue(i,KNOWN_TRUE);      
	  markStageForReevaluation(t+1);
	  if (verbose) cout << "inAdds implies that " << proc->getFact(i) << " must be TRUE at time " << t << std::endl;
    }

    assert (!(inAdds && inDels));
    if (before == UNKNOWN) {
	// If the fact at t-1 is unknown but it's a known positive precondition at t, then it must be true at t-1
	// Also if it's unknown at t-1 but it is known at t and this action doesn't change it, then at t-1 it should get the
        // value at t
	if (inPrePos || (after == KNOWN_TRUE && !inAdds && !inPossAdds)) {
	  preWorld.setTruthValue(i,KNOWN_TRUE);      
	  markStageForReevaluation(t-1);
	  if (verbose) cout << "BInferring that " << proc->getFact(i) << " must be TRUE at time " << t-1 << std::endl;
	} else if (inPreNeg || (after == KNOWN_FALSE && !inDels && !inPossDels)) {
	  preWorld.setTruthValue(i,KNOWN_FALSE);      
	  markStageForReevaluation(t-1);
	  if (verbose) cout << "BInferring that " << proc->getFact(i) << " must be FALSE at time " << t-1 << std::endl;
	}
    } 
    if (after == UNKNOWN) {
	if (before == KNOWN_TRUE && (!inDels && !inPossDels)) {
	  effWorld.setTruthValue(i,KNOWN_TRUE);      
	  if (verbose) cout << "FInferring that " << proc->getFact(i) << " must be TRUE at time " << t << std::endl;
	  markStageForReevaluation(t+1);
	} else if (before == KNOWN_FALSE && (!inAdds && !inPossAdds)) {
	  effWorld.setTruthValue(i,KNOWN_FALSE);      
	  markStageForReevaluation(t+1);
	  if (verbose) cout << "FInferring that " << proc->getFact(i) << " must be FALSE at time " << t << std::endl;
	}
    }
  }
  if (ActionGraph::externallySetSuperVerbose) {
    cout << "In CompareValues\nBefore\n" << proc->printPartialState(preWorld) << "\nAfter\n" << proc->printPartialState(effWorld) << std::endl;
  }
}


bool ActionGraph::applyEffects(WorldState& preWorld, ActionGraphStage& ags)
{
  ++PerformanceCounters::graphApplyEffects;
  //ags.partialWorld = preWorld; // Copy it over to initialize it to have the right length and 
  WorldState& result = ags.partialWorld;
  assert (preWorld.maxPred == result.maxPred);
  for (unsigned i = 0; i <= preWorld.maxPred; i++) {
    bool inAdds = (ags.postPos.find(i) != ags.postPos.end());
    bool inDels = (ags.postNeg.find(i) != ags.postNeg.end());
    bool inPossAdds = (ags.possPostPos.find(i) != ags.possPostPos.end());
    bool inPossDels = (ags.possPostNeg.find(i) != ags.possPostNeg.end());
    bool inNone = (!inAdds && !inDels && !inPossAdds && !inPossDels);
    TruthState after = result.getTruthValue(i);
    TruthState before = preWorld.getTruthValue(i);
    if (ActionGraph::externallySetSuperVerbose) {
      printf("%d Fact: %s Adds: %d Dels: %d P+: %d P-: %d N: %d before: %d after: %d",i,proc->getFact(i).c_str(),inAdds,inDels,inPossAdds,inPossDels,inNone,before,after);
    }
    // consistency checks -- if we've made a bad assumption about what a certain action must/must not be, we could discover the conflict here
    if (after != UNKNOWN && before != UNKNOWN) {
      if (before == KNOWN_TRUE) {
        if (after == KNOWN_TRUE) {
	  if (inDels) return false; // need to stay true but we aren't
        } else { // (after == KNOWN_FALSE) 
          if (!inDels && !inPossDels) return false; // need to go from T to F and no way to get there
        }
      } else { // befor == KNOWN_FALSE
        if (after == KNOWN_TRUE) {
          if (!inAdds && !inPossAdds) return false; // need to go from F to T and no way to get there
        } else { // after == KNOWN_FALSE
          if (inAdds) return false;
        }
      }
    } // end of consistency check
    TruthState newts;
    assert (!(inAdds && inDels));
    if (before == KNOWN_TRUE) {
      if (inNone || inAdds || inPossAdds) {
        newts = KNOWN_TRUE;
      } else if (inDels) {
        newts = KNOWN_FALSE;
      } else {
        newts = after;
      }
    } else if (before == KNOWN_FALSE) {
      if (inNone || inDels || inPossDels) {
        newts = KNOWN_FALSE;
      } else if (inAdds) {
        newts = KNOWN_TRUE;
      } else {
        newts = after;
      }
    } else {  // before == UNKNOWN
      if (inAdds) {
        newts = KNOWN_TRUE;
      } else if (inDels) {
        newts = KNOWN_FALSE;
      } else {
        newts = after;
      }
    }
    result.setTruthValue(i,newts);
    if (ActionGraph::externallySetSuperVerbose) {
      printf(" Final Outcome: %d when I read it: %d\n",newts,result.getTruthValue(i));
    }
  }
  if (ActionGraph::externallySetSuperVerbose) {
    cout << "In ApplyEffects\nBefore\n" << proc->printPartialState(preWorld) << "\nAfter\n" << proc->printPartialState(result) << std::endl;
    cout << "Delete Effects: " << proc->factSetAsString(ags.postNeg) << std::endl;
  }
  return true;
}

void ActionGraph::findAllUnknowns()
{
  for (unsigned f = 0; f <= stages[0].partialWorld.maxPred; f++) {
    for (unsigned t = 0; t < stages.size(); t++) {
      if (stages[t].partialWorld.getTruthValue(f) == UNKNOWN ) {
        cout << proc->getFact(f) << " is unknown at time " << t << "\n";
        if (t+1 < stages.size()) {
          if (stages[t+1].prePos.find(f) != stages[t+1].prePos.end()) {
		cout << "Should know " << proc->getFact(f) << " at time " << t << "\n";
          }
          if (stages[t+1].preNeg.find(f) != stages[t+1].preNeg.end()) {
		cout << "Should know " << proc->getFact(f) << " at time " << t << "\n";
          }
        }
      }
    }
  }
}

void ActionGraph::findAllUnknownToKnownTransitions()
{
  for (unsigned f = 0; f <= stages[0].partialWorld.maxPred; f++) {
    for (unsigned t = 1; t < stages.size(); t++) {
      if (stages[t-1].partialWorld.getTruthValue(f) == UNKNOWN && stages[t].partialWorld.getTruthValue(f) != UNKNOWN) {
        cout << proc->getFact(f) << " is known " << (stages[t].partialWorld.getTruthValue(f) == KNOWN_TRUE ? "TRUE" : "FALSE") << " at time " << t << "\n";
      }
    }
  }
}

void ActionGraph::derivePreservation(unsigned t)
{
  assert (stages[t].isValid);
  assert (t > 0);
  assert (t < stages.size());
  WorldState& before = stages[t-1].partialWorld;
  WorldState& after = stages[t].partialWorld;
  ActionGraphStage& bStage = stages[t-1];
  ActionGraphStage& aStage = stages[t];
  assert (before.maxPred == after.maxPred);
  for (unsigned f = 0; f < after.maxPred; f++) {
    if (after.getTruthValue(f) == UNKNOWN && before.getTruthValue(f) != UNKNOWN) {
      if ((before.getTruthValue(f) == KNOWN_TRUE) && aStage.postNeg.find(f) == aStage.postNeg.end()) {
	after.setTruthValue(f,KNOWN_TRUE);
      }
      else if ((before.getTruthValue(f) == KNOWN_FALSE) && bStage.postPos.find(f) == bStage.postPos.end()) {
	after.setTruthValue(f,KNOWN_FALSE);
      }
    }
  }

}

bool isNotDoable(const Satisfiers& s)
{
  return !s.doable;
}

bool ActionGraph::pruneFinallySatisfiers()
{
  bool pruned = false;
  for (unsigned i = 0; i < finallySatisfiers.size(); i++) {
    FinallySatisfier& fs = finallySatisfiers[i];
    if (fs.resolved) continue;
    for (unsigned p = 0; p < fs.satisfiers.size(); p++) {
      Satisfiers& s = fs.satisfiers[p];
      unsigned time = s.t;
      // Now determine whether it's still possible for the condition specified by fs to be satisfied at time time by s.
      // If, since this item was created, the action for time time has been determined, and the condition of fs is NOT satisfied
      // then this satisfier should be removed from the list.
      TruthState tsAtTime = stages[time].partialWorld.getTruthValue(fs.fact);
      const SetInt& possActions = stages[time].possActions;
      if (possActions.size() == 1) {
        if (tsAtTime != fs.ts) { // Whatever action was determined to have taken place at time time, it did not satisfy the fact of of fs in this way
          s.doable = false;
          if (agverbose) cout << "Pruning satisfier: " << proc->getFact(fs.fact) << " will not be made " << fs.ts << " at time " << time << "\n";
          pruned = true;
        }
      }
    }
  }
  if (pruned) {
    if (agverbose) cout << "Before pruning\n";
    if (agverbose) cout << enumerateFinallySatisfiers();
  }
  for (unsigned i = 0; i < finallySatisfiers.size(); i++) {
    FinallySatisfier& fs = finallySatisfiers[i];
    fs.satisfiers.erase(remove_if(fs.satisfiers.begin(),fs.satisfiers.end(),isNotDoable),fs.satisfiers.end());
  }
  if (pruned) {
    if (agverbose) cout << "After pruning\n";
    if (agverbose) cout << enumerateFinallySatisfiers();
    //assert (false);
  }
  return pruned;
}

void ActionGraph::resolveFinallySatisfiers()
{
  for (unsigned i = 0; i < finallySatisfiers.size(); i++) {
    FinallySatisfier& fs = finallySatisfiers[i];
    if (fs.resolved) continue;
    if (agverbose) cout << fs.asString(); 
    if (fs.satisfiers.size() == 1) {
      Satisfiers& s = fs.satisfiers[0];
      if (s.candidates.size() == 1) {
        if (agverbose) cout << "Resolving that " << proc->getFact(fs.fact) << " will be satisfied by " << proc->operatorIndexToString(s.candidates[0]) << " at " << s.t << "\n";
        assert (s.t < stages.size());
        assert (stages[s.t].partialWorld.getTruthValue(fs.fact) == UNKNOWN || stages[s.t].partialWorld.getTruthValue(fs.fact) == fs.ts);
        stages[s.t].possActions.clear();
        stages[s.t].possActions.insert(s.candidates[0]);
        stages[s.t].partialWorld.setTruthValue(fs.fact,fs.ts);
        if (fs.ts == KNOWN_TRUE) {
          stages[s.t].otherPos.insert(fs.fact);
        } else {
          stages[s.t].otherNeg.insert(fs.fact);
        }
      }
      fs.resolved = true;
    }
  }
}

string ActionGraph::enumerateFinallySatisfiers()
{
  ostringstream os;
  for (unsigned i = 0; i < finallySatisfiers.size(); i++) {
    FinallySatisfier& fs = finallySatisfiers[i];
    if (fs.resolved) continue;
    os << fs.asString(); 
    if (fs.satisfiers.size() == 1) {
      Satisfiers& s = fs.satisfiers[0];
      if (s.candidates.size() == 1) {
        assert (s.t < stages.size());
        assert (stages[s.t].partialWorld.getTruthValue(fs.fact) == UNKNOWN || stages[s.t].partialWorld.getTruthValue(fs.fact) == fs.ts);
        stages[s.t].possActions.clear();
        stages[s.t].possActions.insert(s.candidates[0]);
        stages[s.t].partialWorld.setTruthValue(fs.fact,fs.ts);
        if (fs.ts == KNOWN_TRUE) {
          stages[s.t].otherPos.insert(fs.fact);
        } else {
          stages[s.t].otherNeg.insert(fs.fact);
        }
      }
      fs.resolved = true;
    }
  }
  return os.str();
}

unsigned ActionGraph::mostConstrained()
{
  assert (stages.size() > 1);
  unsigned result = 1;
  unsigned size = INT_MAX; 
  for (unsigned t = 1; t < stages.size(); t++) {
    unsigned currSize = stages[t].possActions.size();
    assert (currSize > 0);
    if (currSize > 1 && currSize < size) {
      result = t;
      size = currSize;
    }
  }
  return result;
}

// Return the product of the number of legal moves for each stage.
// This represents the maximum size of the information set
unsigned ActionGraph::totalPossibilities(bool& exceedsThreshold, unsigned cap)
{
  unsigned result = 1;
  exceedsThreshold = false;
  //cout << "AG TOTAL POSS:\n";
  for (unsigned t = 1; t < stages.size(); t++) {
    //cout << "t " << t << " possActions: " << stages[t].possActions.size() << " results: " << result << std::endl;
    unsigned currSize = stages[t].possActions.size();
    if (result > cap || (result*currSize < result)) {
      exceedsThreshold = true;
	//cout << "Overflow\n";
    }
    assert (currSize > 0);
    result *= currSize;
  }
  return result;
}

unsigned ActionGraph::numVals(unsigned i)
{
  assert (i < stages.size());
  return stages[i].possActions.size();
}

SetInt& ActionGraph::getActions(unsigned i)
{
  assert (i < stages.size());
  return stages[i].possActions;
}

bool ActionGraph::allActionSetsAreSingletons()
{
  for (unsigned t = 1; t < stages.size(); t++) { // start at 1 to avoid stage with initial state and no action
    if (stages[t].possActions.size() != 1) return false;
  }
  return true;
}

VecInt ActionGraph::extractPath() const
{
  VecInt result(stages.size());
  for (unsigned t = 1; t < stages.size(); t++) {
    assert (stages[t].possActions.size() == 1);
    result[t] = *stages[t].possActions.begin();
  }
  return result;
}

}
