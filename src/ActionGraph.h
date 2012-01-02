#ifndef __ACTION_GRAPH_H
#define __ACTION_GRAPH_H

#include "WorldState.h"
#include "processor.h"
#include "ptree.h"

namespace VAL {

enum RPolarity {NEGATED,POSITIVE};
enum RDirection {BACKWARD,FORWARD};

struct Satisfiers {
  unsigned t; // The time at which something WILL be satisfied (finally)
  bool doable; // Gets flipped to false if it is later discovered that the item can't become true at t because the action there has been determined (now), and it doesn't satisify it.
  VecInt candidates; // indices of actions (action ids) that will result in the necessary fact being satisfied at t
  Satisfiers(int t_, VecInt cands) : t(t_), doable(true), candidates(cands) {} 
  string asString();
  
};
  
struct FinallySatisfier
{
  bool resolved;
  int fact;
  TruthState ts;
  unsigned timeNeeded;
  vector< Satisfiers > satisfiers;
  FinallySatisfier (int f, TruthState ts_, unsigned needed) : resolved(false), fact(f), ts(ts_), timeNeeded(needed) {}
  string asString();
};

struct Ramification
{
  int fact;
  TruthState truthState;
  RDirection direction;
  unsigned time;
  Ramification (int f, TruthState ts, RDirection d, unsigned t) : fact(f),truthState(ts),direction(d),time(t) {}
};

class ActionGraphStage
{
public:
  bool isValid;			// Initially set to false in default constructor; should be set when enough structures are filled in and remain true thereafter
  bool needsReevaluation;	// If truth values of neighboring stages are updated, this stage will need to be reevaluated
  unsigned timeStep;		// keep track of which time step this stage refers to
  VecVecInt possOps;		// A so-called "Partial-operator," gleaned from an observation, with some of the arguments filled in
  SetInt possActions;		// All actions currently considered possible at this stage
  WorldState partialWorld;	// The best information we have for the state of the AFTER whichever of the possActions is executed

  SetInt prePos;		// Common pos preconditions
  SetInt preNeg;		// Common neg preconditions
  SetInt postPos;		// Sure add effects of the action mentioned here, leading to the partialWorld here
  SetInt postNeg;		// "	del "	"	
  SetInt possPostPos;		// Possible add effects " "
  SetInt possPostNeg;		// POssible delete effects
  SetInt otherPos;
  SetInt otherNeg;
  SetInt flipPos;
  SetInt flipNeg;

  ActionGraphStage();
};

class ActionGraph
{
public:
  static bool vvvv;
  static bool externallySetSuperVerbose;
  static Processor* gproc;
  static VecFluents fluentHistory;
  bool atLeastOneStageNeedsReevaluation;
  string partialOpString(const VecInt& possOp);
  string asString(const ActionGraphStage& ags, bool brief = false);
  string possibleActionsString(const SetInt& possActions);
  string factSetString(const SetInt& facts);
  string factVecString(const VecInt& facts);
  string factVecSetString(const VecSetInt& facts);
  string enumerateFinallySatisfiers();
  string abbreviatedStagesAsStrings();
  Processor* proc;
  VecVecVecKey obs;
  VecSetVecInt opsByRev;
  vector<ActionGraphStage> stages;
  vector<FinallySatisfier> finallySatisfiers;
  WorldState initialWorld;
  int k; // Which player's perspective
  ActionGraph(Processor* proc, const VecVecVecKey& kb, const VecSetVecInt& opsByRev_, const WorldState& initialWorld, unsigned k_);
  void initStages(const VecVecVecKey& obs, const VecSetVecInt& opsByRev, const WorldState& initialWorld);
  //void sandbox(const VecVecVecKey& kb, const VecSetVecInt& opsByRev);
  void markStageForReevaluation(unsigned i);
  void markAllStagesForReevaluation();
  void reevaluateUntilFixedPoint();
  void initializeGraph();
  void initializeGraphSimple();
  bool foundNewRamifications();
  bool ramify(Ramification r);
  void identifySatisfiers(int fact, TruthState ts, unsigned t, unsigned current);
  bool reevaluatelegalActions();
  bool legalOperators(VecVecInt& partialOp, WorldState& preWorld, ActionGraphStage& ags);
  void identifyNonConflictingActions(VecVecInt& partialOps, WorldState& preWorld, ActionGraphStage& ags);
  void compareKnownTruthWithNeighbors(unsigned t);
  bool oneLegalMove(WorldState& preWorld, ActionGraphStage& ags);
  VecInt getSatisfiers(int fact, TruthState ts, unsigned t);
  bool applyEffects(WorldState& preWorld, ActionGraphStage& ags);
  void findAllUnknownToKnownTransitions();
  void findAllUnknowns();
  void derivePreservation(unsigned t);
  void resolveFinallySatisfiers();
  bool pruneFinallySatisfiers();
  unsigned mostConstrained();
  unsigned totalPossibilities(bool& exceedsThreshold, unsigned cap);
  unsigned numVals(unsigned i);
  SetInt& getActions(unsigned i);
  bool allActionSetsAreSingletons();
  VecInt extractPath() const;
  
//I/O
  string asString(bool brief = false);
  
};
};
#endif
