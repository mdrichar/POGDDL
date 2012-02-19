/*
 * Utilities.cpp
 *
 *  Created on: Feb 18, 2012
 *      Author: mdrichar
 */

#include "Utilities.h"

using namespace std;

namespace VAL {

}

std::vector<std::string> VAL::Utilities::file_get_contents(const std::string & filename)
{
    int count = 0;
    vector<string> logs;
    string line;

    cout << "Testing loading of file." << endl;
    ifstream myfile (filename.c_str());
    if ( myfile.is_open() )
    {
         while ( ! myfile.eof() )
         {
               getline (myfile, line);
               logs.push_back(line);
               count++;
         }
         myfile.close();
    }else{
          cout << "Unable to open file." << endl;
    }
//    cout << "the log count is: " << count << endl
    return logs;
}

std::string VAL::Utilities::file_as_string(const std::string & filename)
{
	std::vector< std::string > lines = file_get_contents(filename);
	cout << "Read: " << lines.size() << "lines" << endl;
	return join(lines);
}

std::string VAL::Utilities::join(const std::vector<std::string> & lines)
{
	ostringstream os;
	for (unsigned i = 0; i < lines.size(); i++) {
		os << lines[i] << "\n";
	}
	return os.str();
}



 /* namespace VAL */
