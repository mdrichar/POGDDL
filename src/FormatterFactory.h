/*
 * FormatterFactory.h
 *
 *  Created on: Jan 3, 2012
 *      Author: mdrichar
 */

#ifndef FORMATTERFACTORY_H_
#define FORMATTERFACTORY_H_

#include "WorldStateFormatter.h"

namespace VAL {

class FormatterFactory {
public:
	FormatterFactory();
	virtual ~FormatterFactory();
	static WorldStateFormatter* createFormatter(Processor* proc);
};

} /* namespace VAL */
#endif /* FORMATTERFACTORY_H_ */
