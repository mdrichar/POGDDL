#ifndef __WORLD_STATE_FORMATTER_H
#define __WORLD_STATE_FORMATTER_H

#include "ptree.h" // Should be just for typdefs and other #includes, which could/should be refactored into their own file
#include <string>
#include "WorldState.h"
#include "processor.h"

namespace VAL {



class WorldStateFormatter {
public:
  Processor* proc;
  WorldStateFormatter();
  virtual ~WorldStateFormatter();
  virtual void setProcessor(Processor* procIn);
  virtual std::string asString(const VecInt& gameHistory, WorldState& ws, const VecVecVecKey& kb) = 0;
  virtual std::string actionString(int actionId);
};

class DefaultWorldStateFormatter : public WorldStateFormatter {
public:
  DefaultWorldStateFormatter();
  virtual ~DefaultWorldStateFormatter();
  virtual std::string asString(const VecInt& gameHistory, WorldState& ws, const VecVecVecKey& kb);
};

class RackoWorldStateFormatter : public WorldStateFormatter {
public:

  RackoWorldStateFormatter();
  virtual ~RackoWorldStateFormatter();
  virtual std::string asString(const VecInt& gameHistory, WorldState& ws, const VecVecVecKey& kb);
  virtual std::string actionString(int actionId);

};

class BattleshipWorldStateFormatter : public WorldStateFormatter {
public:

  BattleshipWorldStateFormatter();
  virtual ~BattleshipWorldStateFormatter();
  virtual std::string asString(const VecInt& gameHistory, WorldState& ws, const VecVecVecKey& kb);
  virtual std::string actionString(int actionId);
};

class DifferenceWorldStateFormatter : public WorldStateFormatter {
public:

  DifferenceWorldStateFormatter();
  virtual ~DifferenceWorldStateFormatter();
  virtual std::string asString(const VecInt& gameHistory, WorldState& ws, const VecVecVecKey& kb);

};

class EndGameWorldStateFormatter : public WorldStateFormatter {
public:

  EndGameWorldStateFormatter();
  virtual ~EndGameWorldStateFormatter();
  virtual std::string asString(const VecInt& gameHistory, WorldState& ws, const VecVecVecKey& kb);

};

class GopsWorldStateFormatter : public WorldStateFormatter {
public:

  GopsWorldStateFormatter();
  virtual ~GopsWorldStateFormatter();
  virtual std::string asString(const VecInt& gameHistory, WorldState& ws, const VecVecVecKey& kb);
  virtual std::string actionString(int actionId);

};

}

#endif
