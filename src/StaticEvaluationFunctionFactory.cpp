/*
 * StaticEvaluationFunctionFactory.cpp
 *
 *  Created on: Jan 4, 2012
 *      Author: mdrichar
 */

#include "StaticEvaluationFunctionFactory.h"

namespace VAL {

StaticEvaluationFunctionFactory::StaticEvaluationFunctionFactory() {
	// TODO Auto-generated constructor stub

}

StaticEvaluationFunctionFactory::~StaticEvaluationFunctionFactory() {
	// TODO Auto-generated destructor stub
}

    StaticEvaluationFunction *StaticEvaluationFunctionFactory::createEvaluator(Processor *proc)
    {
    	std::string domainName = proc->getDomainName();
    	StaticEvaluationFunction* result = NULL;
    	if (domainName == "racko") {
    		result = new RackoStaticEvaluationFunction;
    	} else if (domainName == "battleship") {
    		result = new BattleshipStaticEvaluationFunction;
    	} else if (domainName == "endgame") {
    		result = new EndGameStaticEvaluationFunction;
    	} else if (domainName == "difference") {
    		result = new DifferenceStaticEvaluationFunction;
    	} else {
    		result = new DefaultStaticEvaluationFunction;
    	}
    	return result;

    }

} /* namespace VAL */
