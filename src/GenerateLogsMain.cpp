
#include "GameLogGenerator.h"
#include <iostream>
#include <fstream>
#include <sstream>
using std::ostringstream;

char* current_filename;   // file global ( I don't know why this is necessary)

int main(int argc,char * argv[])
{
  //Processor::checkPayoffs = true; gameLogGenerator.generateGames(string("racko1060x10"), string("games/Racko.pog"), string("games/Racko/racko10-60.pog"), 10);
  Game gameid = GOPS;

  switch (gameid) {
    case RACKO:
    Processor::checkPayoffs = true;
    for (unsigned slotCount = 5; slotCount <= 10; slotCount++) {
      for (unsigned cardCount = 20; cardCount <= 60; cardCount += 10) {
        string domainFile = "games/Racko.pog";
        ostringstream problemStream;
        problemStream << "games/Racko/racko" << slotCount << "-" << cardCount << ".pog";
        ostringstream logStream;
        logStream << "Logs/gameLogRacko" << slotCount << "-" << cardCount;
        //string problemFile = "games/r5-20.pog";
        //string logFile = "Logs/gameLogRacko5-20";
        GameLogGenerator gameLogGenerator;
        gameLogGenerator.generateGames(logStream.str(), domainFile, problemStream.str(), 10);
      }
    }
    break;
  
    case BATTLESHIP:
    Processor::checkPayoffs = true;
    for (unsigned shipCount = 1; shipCount <= 5; shipCount++) {
      for (unsigned cellsPerSideCount = 3; cellsPerSideCount <= 10; cellsPerSideCount ++) {
        string domainFile = "games/LongBattleship.pog";
        ostringstream problemStream;
        problemStream << "games/Battleship/battleship" << shipCount << "-" << cellsPerSideCount << ".pog";
        ostringstream logStream;
        logStream << "Logs/Battleship/gameLogBattleship" << shipCount << "-" << cellsPerSideCount;
        //string problemFile = "games/r5-20.pog";
        //string logFile = "Logs/gameLogBattleship5-20";
        GameLogGenerator gameLogGenerator;
        gameLogGenerator.generateGames(logStream.str(), domainFile, problemStream.str(), 10);
      }
    }
    break;

    case GOPS:
    Processor::checkPayoffs = false;
    for (unsigned slotCount = 1; slotCount <= 10; slotCount++) {
      for (unsigned extraCardCount = 0; extraCardCount <= 45; extraCardCount += 5) {
        string domainFile = "../domains/Gops.pog";
        ostringstream problemStream;
        problemStream << "../problems/Gops/gops-" << slotCount << "." << extraCardCount << ".pog";
        ostringstream logStream;
        logStream << "../logs/Gops/gameLogGops-" << slotCount << "." << extraCardCount;
        //string problemFile = "games/r5-20.pog";
        //string logFile = "Logs/gameLogBattleship5-20";
        GameLogGenerator gameLogGenerator;
        gameLogGenerator.generateGames(logStream.str(), domainFile, problemStream.str(), 10);
      }
    }
    break;
    default:
	std::cerr << "Game not yet handled\n";
	
    break;
  }
  return 0;
}
