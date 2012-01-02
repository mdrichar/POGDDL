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
  ~WorldStateFormatter();
  virtual void setProcessor(Processor* procIn);
  virtual std::string asString(WorldState& ws) = 0;
};

class DefaultWorldStateFormatter : public WorldStateFormatter {
public:
  DefaultWorldStateFormatter();
  ~DefaultWorldStateFormatter(); 
  virtual std::string asString(WorldState& ws);
};

class RackoWorldStateFormatter : public WorldStateFormatter {
public:
  unsigned slotCount;
  unsigned cardCount;

  RackoWorldStateFormatter(unsigned slotCountIn, unsigned cardCountIn);
  ~RackoWorldStateFormatter(); 
  virtual std::string asString(WorldState& ws);
};

class BattleshipWorldStateFormatter : public WorldStateFormatter {
public:
  unsigned shipCount;
  unsigned gridPointsPerSide;

  BattleshipWorldStateFormatter(unsigned shipCountIn, unsigned gridPointsPerSideIn);
  ~BattleshipWorldStateFormatter(); 
  virtual std::string asString(WorldState& ws);
};

}

#endif
