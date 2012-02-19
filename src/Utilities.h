/*
 * Utilities.h
 *
 *  Created on: Feb 18, 2012
 *      Author: mdrichar
 */

#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

namespace VAL {

class Utilities {
public:
	static std::vector< std::string > file_get_contents(const std::string& filename);
	static std::string file_as_string(const std::string& filename);
	static std::string join(const std::vector< std::string >& lines);
};

} /* namespace VAL */
#endif /* UTILITIES_H_ */
