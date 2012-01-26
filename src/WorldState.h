#ifndef __WORLD_STATE_H
#define __WORLD_STATE_H

#include "ptree.h" // Should be just for typdefs and other #includes, which could/should be refactored into their own file

//const int KNOWN_FALSE = 0;
//const int KNOWN_TRUE = 1;
//const int UNKNOWN = 2;

namespace VAL {
enum TruthState {KNOWN_FALSE, KNOWN_TRUE, UNKNOWN};
class WorldState
{
public:
  //static const int intsize = 22; // Game 14
  static const int intsize = 905; // Game 14
  //static const int intsize = 314; // Game 1
  static int intLength(unsigned maxBit);
  static StringToInt predHeadTbl;
  static VecVecInt fastPredMults;
  static StringToInt funcHeadTbl;
  static VecInt fastOffsets;
  static int fastNumLookupByArgIndex(unsigned predId, const VecInt& indices, const VecInt& args);
  static int getIndex(const VecInt& args, int headId);
  static int lastChecked;
  static VecInt roleIds; // roleIds[i] gives the factId for (whoseturn i)

  unsigned maxPred;
  unsigned nRoles;
  //int* stateBits;
  //int* knownFlags;
  int stateBits[intsize];
  int knownFlags[intsize];
  FluentTable fluentVals;
  FluentTable fluentUpdates;
  VecInt addFacts;
  VecInt delFacts;
  VecVecKey obsListByPlyr; // obListByPlyr[k][i] is the ith part of the observation list for player k

  WorldState(unsigned maxPred_, unsigned nRoles_);
  void initUnknown(unsigned maxPred_, unsigned nRoles_);
  WorldState(const WorldState& other);
  WorldState();
  const WorldState& operator=(const WorldState& other);
  ~WorldState();
  NumScalar fastFuncLookupByArgIndex(VecInt& intArgs, unsigned funcId, const VecInt& indices, const VecInt& args);
  TruthState getTruthValue(unsigned predId, const VecInt& indices, const VecInt& args);
  TruthState getTruthValue(unsigned index) const; // boolean state
  NumScalar getValue(const VecInt& args) const ; // numerical state
  FluentTable getFluents() const;
  void setFluents(const FluentTable& ft) ;
  void setValue(const VecInt& args, NumScalar value); // numerical state
  void setTruthValue(unsigned index, TruthState val);
  //void setKnownValue(unsigned index, bool val);
  void add(unsigned index);
  void del(unsigned index);
  bool modifyingFact(unsigned index, TruthState ts);
  void applyUpdates();
  void clearUpdates();
  int getNRoles() const;
  int getWhoseTurn() const;
  
  // API for use by Formatters and StaticEvaluators
  bool getTruthValue(int headId, int arg1);
  bool getTruthValue(int headId, int arg1, int arg2);
  NumScalar getFluentValue(int headId);
  NumScalar getFluentValue(int headId, int arg1);





};
};

#endif
