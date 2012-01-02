#ifndef INFO_SET_GENERATOR
#define INFO_SET_GENERATOR

#include "ptree.h"
#include "ActionGraph.h"
#include "processor.h"
#include <stack>


namespace VAL{
class InfoSetGenerator
{
public:
  static Processor* gproc;
  Processor* proc;
  VecVecVecKey obs;
  WorldState initialWorld; // game tree root 
  unsigned k; // Player k's perspective
  
  InfoSetGenerator(Processor* proc_, const VecVecVecKey& obs_, const WorldState& initialWorld_);
  SetVecInt generate(unsigned k, unsigned maxSize);
  SetVecInt generateN(unsigned k, unsigned maxSize);

  // Begin from the root of the tree.  Perform a depth-first search, pruning whenever simulated observation 
  // differs from actual observation
  void generateNaive(VecInt& sequence, WorldState& currentWorld, unsigned t, SetVecInt& results, double startTime, double timeLimit, unsigned maxSize);
  void generateRec(SetVecInt& result);
  void generateRandom(SetVecInt& result);
  bool verify(const VecInt& path, VecVecVecKey& obs, unsigned k, const WorldState& initialWorld);
  std::stack<ActionGraph> actionStack;
  void displayPath(const VecInt& path);
  void processSolution(const ActionGraph& ag, SetVecInt& result);
  static SetVecInt randomPruneDownTo(const SetVecInt& sequences, unsigned maxSize);
  static string moveSequenceString(const VecInt& sequence);
};
}

#endif
