/*
 * FormatterFactory.cpp
 *
 *  Created on: Jan 3, 2012
 *      Author: mdrichar
 */

#include "FormatterFactory.h"
#include "WorldStateFormatter.h"
#include <cstddef>

namespace VAL {

FormatterFactory::FormatterFactory() {
	// TODO Auto-generated constructor stub

}

FormatterFactory::~FormatterFactory() {
	// TODO Auto-generated destructor stub
}

WorldStateFormatter *FormatterFactory::createFormatter(Processor *proc) {
	std::string domainName = proc->getDomainName();
	WorldStateFormatter* result = NULL;
	if (domainName == "racko") {
		result = new RackoWorldStateFormatter;
	} else if (domainName == "battleship") {
		result = new BattleshipWorldStateFormatter;
	} else if (domainName == "difference") {
		result = new DifferenceWorldStateFormatter;
	} else if (domainName == "endgame") {
		result = new EndGameWorldStateFormatter;
	} else if (domainName == "gops") {
		result = new GopsWorldStateFormatter;
	} else {
		result = new DefaultWorldStateFormatter;
	}
	return result;
}

} /* namespace VAL */
