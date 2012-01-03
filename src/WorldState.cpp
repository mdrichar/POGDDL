#include "WorldState.h"
#include "PerformanceCounters.h"
#include <climits>
#include <string.h>
#define IsSet(a,ind)  ( (a[((ind) / 32)]) & (1<<((ind) % 32)) )
#define Set(a,ind) a[((ind)/32)] = ( a[((ind)/32)] | (1<<((ind) % 32)) )
#define Reset(a,ind) a[((ind)/32)] = ( a[((ind)/32)] & (~(1<<((ind) % 32))) )


//int WorldState::intsize = 50;
namespace VAL {
StringToInt WorldState::predHeadTbl;
VecVecInt WorldState::fastPredMults;
StringToInt WorldState::funcHeadTbl;
VecInt WorldState::fastOffsets;
VecInt WorldState::roleIds;
int WorldState::lastChecked;

int WorldState::intLength(unsigned maxBit)
{

  int result = (maxBit / (sizeof(int) * 8));
  if (result * (sizeof(int)*8) != maxBit) {
    ++result; // computation of result rounded down, so we need one more
  }
  return result;
}

WorldState::WorldState(unsigned maxPred_, unsigned nRoles_)
  : maxPred(maxPred_), nRoles(nRoles_)
{
  this->nRoles = nRoles;
  assert (intsize >= intLength(maxPred));
  //int intsize = intLength(maxPred);
  //stateBits = new int[intsize];
  //knownFlags = new int[intsize];
  memset(stateBits,0,sizeof(int)*intsize);
  memset(knownFlags,0xff,sizeof(int)*intsize);
  obsListByPlyr.resize(this->nRoles);	
}

void WorldState::initUnknown(unsigned maxPred_, unsigned nRoles_)
{
  //delete[] stateBits;
  //delete[] knownFlags;
  maxPred = maxPred_;
  //int intsize = intLength(maxPred);
  this->nRoles = nRoles_;
  //stateBits = new int[intsize];
  //knownFlags = new int[intsize];
  assert (intsize >= intLength(maxPred));
  memset(stateBits,0,sizeof(int)*intsize);
  memset(knownFlags,0,sizeof(int)*intsize);
  obsListByPlyr.resize(this->nRoles);	
}

WorldState::WorldState()
 : maxPred(0), nRoles(0) //, stateBits(0), knownFlags(0)
{

}

WorldState::WorldState(const WorldState& other)
  : maxPred(other.maxPred), nRoles(other.nRoles), fluentVals(other.fluentVals),
	fluentUpdates(other.fluentUpdates), obsListByPlyr(other.obsListByPlyr)
{
  //int intsize = intLength(maxPred);
  //stateBits = new int[intsize];
  //knownFlags = new int[intsize];
  assert (intsize >= intLength(maxPred));
  memcpy(this->stateBits,other.stateBits,sizeof(int)*intsize);
  memcpy(this->knownFlags,other.knownFlags,sizeof(int)*intsize);
}

const WorldState& WorldState::operator=(const WorldState& other)
{
  //delete [] stateBits;
  //delete [] knownFlags;
  maxPred = other.maxPred;
  nRoles = other.nRoles;
  //int intsize = intLength(maxPred);
  //stateBits = new int[intsize];
  //knownFlags = new int[intsize];
  assert (intsize >= intLength(maxPred));
  memcpy(this->stateBits,other.stateBits,sizeof(int)*intsize);
  memcpy(this->knownFlags,other.knownFlags,sizeof(int)*intsize);
  this->fluentVals = other.fluentVals;
  this->fluentUpdates = other.fluentUpdates;
  this->obsListByPlyr.resize(nRoles);
  //this->tmpFluentVals = other.tmpFluentVals;
  return *this;
}

WorldState::~WorldState()
{
  //delete [] stateBits;
  //delete [] knownFlags;
}

TruthState WorldState::getTruthValue(unsigned index) const // boolean state
{
  assert (index <= maxPred);
  if (!IsSet(knownFlags,index)) return UNKNOWN;
  return (IsSet(stateBits,index) ? KNOWN_TRUE : KNOWN_FALSE);
}

NumScalar WorldState::getValue(const VecInt& key) const // numerical state
{
  NumScalar result;
  FTitr itr = fluentVals.find(key);
  if (itr != fluentVals.end()) {
    result = itr->second;
  } else {
    result = INT_MAX; // TODO: robustify against changes to NumScalar type 
  }
  return result;
}

FluentTable WorldState::getFluents() const
{
  return fluentVals;
}

void WorldState::setFluents(const FluentTable& ft)
{
  fluentVals = ft;
}

void WorldState::setValue(const VecInt& args, NumScalar value) // numerical state
{
  //tmpFluentVals[args] = value;
  fluentVals[args] = value;
}

void WorldState::setTruthValue(unsigned index, TruthState val)
{
  assert (index <= maxPred);
  switch (val) {
    case KNOWN_TRUE:
      Set(knownFlags,index);
      Set(stateBits,index);
      break;
    case KNOWN_FALSE:
      Set(knownFlags,index);
      Reset(stateBits,index);
      break;
    case UNKNOWN:
      Reset(knownFlags,index);
  }
}

//void WorldState::setKnownValue(unsigned index, bool val)
//{
//  assert (index <= maxPred);
//  if (val) {
//    Set(knownFlags,index);
//  } else {
//    Reset(knownFlags,index);
//  }
//}

void WorldState::add(unsigned index)
{
  addFacts.push_back(index);
}

void WorldState::del(unsigned index)
{
  delFacts.push_back(index);
}

bool WorldState::modifyingFact(unsigned index, TruthState ts)
{
  assert (ts != UNKNOWN);
  if (ts == KNOWN_TRUE) {
    return (find(addFacts.begin(),addFacts.end(),index) != addFacts.end());
  }
  // so ts = false
  return (find(delFacts.begin(),delFacts.end(),index) != delFacts.end());
}

void WorldState::applyUpdates()
{
  ++PerformanceCounters::worldStateApplyUpdates;
  for (VecInt::const_iterator itr = addFacts.begin(); itr != addFacts.end(); ++itr) {
    setTruthValue(*itr,KNOWN_TRUE);
  }
  for (VecInt::const_iterator itr = delFacts.begin(); itr != delFacts.end(); ++itr) {
    setTruthValue(*itr,KNOWN_FALSE);
  }
  for (FluentTable::const_iterator itr = fluentUpdates.begin(); itr != fluentUpdates.end(); ++itr) {
    assert (fluentVals.find(itr->first) != fluentVals.end()); // This is not technically a problem; it's ok to assign a value to an undefined numeric constant; I'm just putting this here because I'm currently not expecting any of these
    fluentVals[itr->first] = itr->second;
  }
  clearUpdates();
}

void WorldState::clearUpdates()
{
  addFacts.clear();
  delFacts.clear();
  fluentUpdates.clear();
  obsListByPlyr.clear();  // remove everything
  obsListByPlyr.resize(nRoles);	
}

int WorldState::getNRoles() const
{
  return nRoles;
}

int WorldState::getWhoseTurn() const
{
  int playerId = -1;
  unsigned activePlayersFound = 0; // increase by one for every player for which (whoseturn player) is true
  for (unsigned i = 0; i < roleIds.size(); i++) {
    TruthState ts = getTruthValue(roleIds[i]);
    assert (ts != UNKNOWN); // A player should always know if it's his turn
    if (ts == KNOWN_TRUE) {
      playerId = i;
	//cout << "active Player: " << i << std::endl;
      activePlayersFound++;
    }
  }
  //If we allow it to be no one's turn when the game is over, this assertion is too strong
  //assert (activePlayersFound == 1); // There should always be exactly one player whose turn it is
  return playerId;
}

int WorldState::getIndex(const VecInt& args, int headId)
{
  int result = fastOffsets[headId];
  const VecInt& mults = fastPredMults[headId];
  for (unsigned i = 0; i < mults.size(); i++) {
    result += mults[i]*args[i];
  }
  assert (result >= 0);
  return result; 
}

// Return the index of the fact of the predName propType, where the ith argument is args[inidcies[i]]
// for variables and -1-indices[i] for constants
int WorldState::fastNumLookupByArgIndex(unsigned predId, const VecInt& indices, const VecInt& args)
{
  static VecInt intArgs;
  assert(predId < predHeadTbl.size());
  //unsigned nArgs = predParamTypeTbl[predId].size(); 
  if (intArgs.size() < indices.size()) intArgs.resize(indices.size());
  for (unsigned i = 0; i < indices.size(); i++) {
    if (indices[i] >= 0) {
      assert (indices[i] < (int)args.size());
      intArgs[i] = args[indices[i]];
    } else {
      intArgs[i] = -1 - indices[i];
    }
    assert (intArgs[i] >= 0);
    //assert (intArgs[i] < (int)typeTbl[predHeadTbl[predId][i]].size());
  } 
  lastChecked = getIndex(intArgs, predId);
  return lastChecked;
  //return getIndex(intArgs,predId);
}

TruthState WorldState::getTruthValue(unsigned predId, const VecInt& indices, const VecInt& args)
{
  int factNum = fastNumLookupByArgIndex(predId, indices, args);
  return getTruthValue(factNum);
}

// Return the index of the fact of the predName propType, where the ith argument is args[inidcies[i]]
// for variables and -1-indices[i] for constants
NumScalar WorldState::fastFuncLookupByArgIndex(VecInt& intArgs, unsigned funcId, const VecInt& indices, const VecInt& args)
{
  //static VecInt intArgs;
  assert(funcId < funcHeadTbl.size());
  //unsigned nArgs = predParamTypeTbl[predId].size(); 
  //if (intArgs.size() < indices.size()) 
  intArgs.resize(indices.size()+1);
  for (unsigned i = 0; i < indices.size(); i++) {
    if (indices[i] >= 0) {
      assert (indices[i] < (int)args.size());
      intArgs[i+1] = args[indices[i]];
    } else {
      intArgs[i+1] = -1 - indices[i];
    }
    assert (intArgs[i+1] >= 0);
    //assert (intArgs[i] < (int)typeTbl[predHeadTbl[predId][i]].size());
  } 
  intArgs[0] = funcId;
  NumScalar funcVal = getValue(intArgs);
  return funcVal;
}

};
