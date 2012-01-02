#include "PerformanceCounters.h"
#include <string>
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <sstream>
using std::string;

namespace VAL {
  
  unsigned PerformanceCounters::procLegalOperators;
  unsigned PerformanceCounters::graphLegalOperators;
  unsigned PerformanceCounters::graphNonConflicting;
  unsigned PerformanceCounters::worldStateApplyUpdates;
  unsigned PerformanceCounters::graphApplyEffects;
  unsigned PerformanceCounters::graphCompareValues;
  unsigned PerformanceCounters::generateNaive;
  unsigned PerformanceCounters::solutionCount;
  unsigned PerformanceCounters::total;
  float PerformanceCounters::efficiency;
  void PerformanceCounters::resetCounters() {
    procLegalOperators = 0;
    graphLegalOperators = 0;
    graphNonConflicting = 0;
    worldStateApplyUpdates = 0;
    graphApplyEffects = 0;
    graphCompareValues = 0;
    generateNaive = 0;
    solutionCount = 0;
  }
  
  string PerformanceCounters::getPerformanceReportAsString() {
    std::ostringstream os;
    os << "procLegalOperators: " << procLegalOperators << " ";
    os << "graphLegalOperators: " << graphLegalOperators << " ";
    os << "graphNonConflicting: " << graphNonConflicting << " ";
    os << "worldStateApplyUpdates: " << worldStateApplyUpdates << " ";
    os << "graphApplyEffects: " << graphApplyEffects << " ";
    os << "graphCompareValues: " << graphCompareValues << " ";
    os << "generateNaive: " << generateNaive << " ";
    os << "solutionCount: " << solutionCount << " ";
    os << "efficiency: " << getEfficiency() << " ";
    return os.str();
  }
 
  float PerformanceCounters::getEfficiency() {
    total = procLegalOperators + graphLegalOperators + graphNonConflicting + worldStateApplyUpdates + graphApplyEffects + graphCompareValues + generateNaive + solutionCount;
    efficiency = (float) total / solutionCount;
    return efficiency; 
  }
}

