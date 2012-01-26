
#include "GameLogGenerator.h"
#include <iostream>
#include <fstream>
#include <sstream>
using std::ostringstream;

char* current_filename;   // file global ( I don't know why this is necessary)

int main(int argc,char * argv[])
{
  GameLogGenerator gameLogGenerator;
  //Processor::checkPayoffs = true; gameLogGenerator.generateGames(string("Logs/gameLogRacko5-40"), string("games/Racko.pog"), string("games/Racko/racko5-40.pog"), 10);
//  Processor::checkPayoffs = false; gameLogGenerator.generateGames(string("Logs/mastermind"), string("games/Mastermind4.pog"), string("games/mast7.pog"), 10);
  Processor::checkPayoffs = false; gameLogGenerator.generateGames(string("../logs/EndGame/endgame"), string("../domains/EndGame.pog"), string("../problems/EndGame/endgame-1.pog"), 10);
  //Processor::checkPayoffs = true; gameLogGenerator.generateGames(string("racko1060x10"), string("games/Racko.pog"), string("games/Racko/racko10-60.pog"), 10);
 // Processor::checkPayoffs = false; gameLogGenerator.generateGames(string("gops13x1"), string("games/Gops.pog"), string("games/Gops/gops1-3.pog"), 1);
  //Processor::checkPayoffs = true; gameLogGenerator.generateGames(string("nbattles510magic"), string("games/LongBattleship.pog"), string("games/Battleship/battleship5-10.pog"), 10);
  return 0;
}
