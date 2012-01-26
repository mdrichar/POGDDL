#ifndef __STATIC_EVALUATION_FUNCTION_H
#define __STATIC_EVALUATION_FUNCTION_H

#include "ptree.h" // Should be just for typdefs and other #includes, which could/should be refactored into their own file
#include <string>
#include "WorldState.h"
#include "processor.h"

namespace VAL {



class StaticEvaluationFunction {
public:
  Processor* proc;
  StaticEvaluationFunction();
  virtual ~StaticEvaluationFunction();
  virtual void setProcessor(Processor* procIn);
  virtual NumScalar getEstimatedValue(const VecInt& gameHistory, WorldState& ws, const VecVecVecKey& kb) = 0;
};

class DefaultStaticEvaluationFunction : public StaticEvaluationFunction {
public:
  DefaultStaticEvaluationFunction();
  virtual ~DefaultStaticEvaluationFunction();
  virtual NumScalar getEstimatedValue(const VecInt& gameHistory, WorldState& ws, const VecVecVecKey& kb);
};

class RackoStaticEvaluationFunction : public StaticEvaluationFunction {
public:

  RackoStaticEvaluationFunction();
  virtual ~RackoStaticEvaluationFunction();
  virtual NumScalar getEstimatedValue(const VecInt& gameHistory, WorldState& ws, const VecVecVecKey& kb);

};

class BattleshipStaticEvaluationFunction : public StaticEvaluationFunction {
public:

  BattleshipStaticEvaluationFunction();
  virtual ~BattleshipStaticEvaluationFunction();
  virtual NumScalar getEstimatedValue(const VecInt& gameHistory, WorldState& ws, const VecVecVecKey& kb);

};

class DifferenceStaticEvaluationFunction : public StaticEvaluationFunction {
public:

  DifferenceStaticEvaluationFunction();
  virtual ~DifferenceStaticEvaluationFunction();
  virtual NumScalar getEstimatedValue(const VecInt& gameHistory, WorldState& ws, const VecVecVecKey& kb);

};
class EndGameStaticEvaluationFunction : public StaticEvaluationFunction {
public:

  EndGameStaticEvaluationFunction();
  virtual ~EndGameStaticEvaluationFunction();
  virtual NumScalar getEstimatedValue(const VecInt& gameHistory, WorldState& ws, const VecVecVecKey& kb);

};

}

#endif
