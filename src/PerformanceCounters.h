#ifndef __PERFORMANCE_COUNTERS_H__
#define __PERFORMANCE_COUNTERS_H__

#include <string>
namespace VAL {
class PerformanceCounters
{
public:
  static unsigned procLegalOperators;
  static unsigned graphLegalOperators;
  static unsigned graphNonConflicting;
  static unsigned worldStateApplyUpdates;
  static unsigned graphApplyEffects;
  static unsigned graphCompareValues;
  static unsigned generateNaive;
  static unsigned solutionCount;
  static unsigned total;
  static float efficiency;
  static void resetCounters();
  static float getEfficiency();
  static std::string getPerformanceReportAsString();

};
}
#endif
