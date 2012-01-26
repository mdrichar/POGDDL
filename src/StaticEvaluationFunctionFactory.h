/*
 * StaticEvaluationFunctionFactory.h
 *
 *  Created on: Jan 4, 2012
 *      Author: mdrichar
 */

#ifndef STATICEVALUATIONFUNCTIONFACTORY_H_
#define STATICEVALUATIONFUNCTIONFACTORY_H_

#include "StaticEvaluationFunction.h"

namespace VAL {

class StaticEvaluationFunctionFactory {
public:
	StaticEvaluationFunctionFactory();
	virtual ~StaticEvaluationFunctionFactory();
	static StaticEvaluationFunction* createEvaluator(Processor* proc);
};

} /* namespace VAL */
#endif /* STATICEVALUATIONFUNCTIONFACTORY_H_ */
