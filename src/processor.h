#ifndef PROCESSOR_H
#define PROCESSOR_H

#define IsSet(a,ind)  ( (a[((ind) / 32)]) & (1<<((ind) % 32)) )
#define Set(a,ind) a[((ind)/32)] = ( a[((ind)/32)] | (1<<((ind) % 32)) )
#define Reset(a,ind) a[((ind)/32)] = ( a[((ind)/32)] & (~(1<<((ind) % 32))) )

#include "ptree.h"
#include "WorldState.h"
namespace VAL {

enum ListType  {NUMERIC,OPERATOR,PREDICATE,REVELATION};
class ConditionController;
class NewConditionController;
class WorldStateFormatter;
class Processor
{
public:
  static bool superVerbose;
  static bool checkPayoffs;
  static double clockTotal;
  unsigned maxPred;
  unsigned maxOperator;
  unsigned nRoles; // number of players, including chance (nRoles - 1 is number of players who receive messages)

// State
  //int* stateBits;
  FluentTable fluentVals;
// Types
  TypeTable typeTbl;  // maps a string (that names a type) to a set of strings that name the items of that type
  StringToInt typeNameIds; // maps a string that names a type to an integer index (so that the type information can be looked up random access)
  VecStr typeIdNames;
  VecInt typeCardinalities; // typeCardinalties[i] = j means that the type with integer index i has j possible objects

// Predicates
  bool isPredAlreadyPresent(const string& name);
  unsigned newPredHeadId(const string& name, unsigned nArgs);
  StringToInt predHeadTbl;
  //VecStr inversePreds;
  VecVecInt predParamTypeTbl;
  VecVecInt predParamTypeCardTbl;
  PredicateTable predTbl; // maps a predicate name to a list of strings that specify the types of this preds args
  PredicateMultTable predMults;
  VecVecInt fastPredMults;

// Functions
  StringToInt funcHeadTbl;
  VecStr inverseFuncs;
  int fastFuncLookupByArgIndex(VecInt& intArgs, unsigned funcId, const VecInt& indices, const VecInt& args);

// Facts
  StringToInt offsets;
  StringToBounds factsOffsets;
  VecInt fastOffsets;
  StringToInt counts;

// Objects
  ObjectToInt objectTbl; // maps the name of a constant to the number assigned to it
  map<string,string> objectTypes;
  // map string:VecStr
  TypedIntToObject invObjectTbl;  // figure out what the name of an argument should be from its type and index#

// Operators 
  PredicateTable opsTbl; // maps a predicate name to a VecStr that lists the types of the arguments
  PredicateMultTable opsMults;
  StringToBounds opsOffsets;
  StringToOperator opsFromParser;
  StringToVecStrMap actionArgListTypes; // maps the name of an action to a list of strings that give the type of the action params
  VecVecKey fullApply(int index, WorldState& ws);

// Revelations
  StringToInt revHeadTbl;
  VecStr inverseRevs;
  VecVecInt revParamTypeTbl; // revParamTypeTbl[i][j] gives the index of the type of the jth parameter to the the observations indexed by i
  VecVecInt revParamTypeCardTbl; 
  bool isRevAlreadyPresent(const string& name);
  unsigned newRevelationHeadId(const string& name, unsigned nArgs);
  VecVecVecKey kb; // kb = knowledge base;  kb[t][k][i] is the ith part of the observation for player k at time t 
  // (In nearly all cases, i == 0, and we could constrain things conceptually so that is always the case.
  // A contrary example is in go fish, where the opponent draws a card and it turns out that it matches.
  // The action spec always requires that the opponent's are told that some card was drawn. 
  // If that card happens to match a card already held by the opponent, a second observation is made that there 
  // is a match for the opponent.  We could modify the action so that the message about "some card" being drawn
  // is conditioned on a non-match.  In other words, a player will be told either that some card was drawn or that 
  // the opponent drew a matching card, but not both. 

// payoffs
  VecPayoff payoffs; // lazily filled in
  bool alwaysCheckPayoffs();
  bool isTerminal(WorldState& ws);

private:
  WorldStateFormatter* formatter;



public:
  WorldState worldState;
  WorldState initialWorld;
  static int intLength(unsigned maxBit);
  bool hasPred(const string& name);
  void enumerateLegalActions(int* state, unsigned maxFact, unsigned maxAction);

// Fact/Action Relationships
  VecSetInt factsPrereqsTo;
  VecSetInt factsEffectsTo;  
  VecSetInt opsPrereqsOf;
  VecSetInt opsEffectsOf;
  static const StringToBoundsEntry getPredName(const StringToBounds& map, unsigned index);
  static void ensureCapacity(VecSetInt& vss, int neededIndex);
  static void add(VecSetInt& vss, int key, int value);
  string asString(const VecInt& vi, ListType lt = NUMERIC);
  static string asString(const VecPayoff& vi);
  static string asString(const VecSetInt& vsi);
  static string asString(const VecSetVecInt& vsvi);
  static string asString(const SetInt& vsi);
  static string asString(const StringToInt& si);
  string factSetAsString(const SetInt& facts);
  void showLookupTables();

  Processor(const gain_list& gains_, const operator_list& operators_);
  Processor(VAL::analysis* parsedStructures);
  ~Processor() {  };
  void processTypes(const pddl_type_symbol_table& types);
  void processConstants(const const_symbol_list& constants);
  void processConstants(const const_symbol_table& constants);
  void processOperators();
  void processGains();
  VecInt legalOperators(WorldState& partialWorld);
  //VecInt legalOperators(VecInt& partialOp, WorldState& partialWorld, VecInt& prePos, VecInt& preNeg);
  void processPredicates(const pred_decl_list& predicates);
  void processFunctions(const func_decl_list& functions);
  void processConstants(domain* p);
  void enumerateFacts();
  void enumerateInitFacts(effect_lists* initFacts);
  void preWrite();
  void write2(ostream& o);
  //int getNum(string, string);
  //int getNum(string, string, string);
  //int getNum(string, string, string, string);
  int getNum(string pred, int arg0);
  int getNum(string pred, int arg0, int arg1);
  int getNum(string pred, int arg0, int arg1, int arg2);
  void generateFacts(const string& pred, const VecStr& argTypes, VecInt& args, unsigned level);
  void generateOffsets();

// Fluents
  int getValue(const FunctionKey& key);
  void setValue(const FunctionKey& key, int value);

// Facts
  string getFact(int index, bool& canChange);
  string getFact(int index);
  int getIndex(const string& name, const VecInt& args);
  int getIndex(const VecInt& args, int headId);
  StringToIntEntry getPredName(int index);
  int factNumLookupByArgIndex(const string& predName, const VecInt& indices, const VecInt& args);
  int fastNumLookupByArgIndex(unsigned predId, const VecInt& indices, const VecInt& args);

// operators
  const gain_list& gains;
  const operator_list& operators;
  vector<operator_*> vecOps;
  StringToInt opHeadTbl;
  VecStr getOperatorString(int index, const string& name, const VecStr& vars, const VecInt& mults); // deprecate?
  string getOperatorString(const string& name, const VecInt& args);
  int getOperatorIndex(const string& opName, const VecStr& args);
  int getOperatorIndex(const string& opName, const VecInt& args);
  int getOperatorIndex(int offset, const VecInt& mults, const VecInt& args);
  int getIndex(const StringToBounds& offsets, const PredicateMultTable& mults, const string& name, const VecInt& args);
  void decodeOperator(int index, string& oName, VecInt& oArgs, bool resize = true); // Convert the index (input) into the corresponding opName and integer args (output)
  string operatorIndexToString(int index);
  operator_* getOpFromIndex(int index);
  VecInt getOpArgs(int index);

// applying actions
  void apply(int opIndex, WorldState& ws);
  void apply(const VecInt& sequence, WorldState& ws);
  void shortApply(int index, WorldState& ws, NewConditionController& ncc);
  VecVecVecKey getObs(const VecInt& sequence, WorldState& ws);
  void apply(const string& name, const VecInt& args, WorldState& ws);
  void finalizeApply(WorldState& ws, bool readObs = true);

  void enumerateOperators();
  void generateOps(const string& opName, const VecStr& argTypes, VecInt& args, unsigned level);
  //void operatorsByIndex();

  //VecInt paramsToInts(para
  int encodeGroundedFact(const proposition* p);
  int encodeFact(const proposition* p, const StringToInt& params);

  void randomTests();

// Derived predicates
  void computeDerivedPredClosures(derivations_list&) ;
  void derive(ConditionController& cc, derivation_rule& dr, VecInt& vi, unsigned ind);
  bool checkTruth(ConditionController& cc, derivation_rule& dr);
  bool satisfiesCondition(ConditionController& cc, goal* g);

// State representation
  void initializeStaticWorldState();
  string printState(int* x, int max);
  string printState(const WorldState& ws, bool includeKnowledgeBases = true);
  string printPartialState(const WorldState& ws);
  string printKnowledgeState(int player);
  void printState();

// Game specific formatters
  void setFormatter(WorldStateFormatter* formatter);
  string getFormattedState(WorldState& ws);

// payoffs
  void computePayoffs(WorldState& ws);
  NumScalar sumPayoffs();

// Infoset stuff
  VecInt mutableFuncs; // Identify wich functions can be changed by operators
  VecInt mutablePreds; // Identify which predicates can be changed by operators
  VecSetVecInt opsByRev; 
  VecInt getPartialOperator(const VecInt& observation, const VecInt& argMapper);
//WorldState getDisjunctionConsequences(VecInt& legalActions, WorldState partialWorld, 
  //SetInt& addEffectsInter, SetInt& delEffectsInter, SetInt& addSymDiff, SetInt& delSymDiff);
  //WorldState getDisjunctionConsequences(VecInt& legalActions, WorldState partialWorld, SetInt& adds, SetInt& dels, SetInt& possAdds, SetInt& possDels);
  //WorldState getDisjunctionConsequences(const WorldState& oldPartial, const SetInt& adds, const SetInt& dels, const SetInt& possAdds, const SetInt& possDels);
  //void sandbox(const VecVecVecKey& kb, const VecSetVecInt& opsByRev);
  void sandbox2(const VecVecVecKey& kb, const VecSetVecInt& opsByRev);
};
}
#endif 