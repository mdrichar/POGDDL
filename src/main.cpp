
#include "GameLogGenerator.h"
#include <iostream>
#include <sstream>
#include <string>
#include <gsl/gsl_rng.h>
#include "WorldStateFormatter.h"

using std::ostringstream;

char* current_filename;   // file global ( I don't know why this is necessary)

int main(int argc,char * argv[])
{
  //const gsl_rng_type * T;

  //gsl_rng_env_setup();
  //gsl_rng* r;

  //T = gsl_rng_default;
  //r = gsl_rng_alloc (T);
  //
  //printf("generator type: %s\n", gsl_rng_name (r));
  //printf("seed = %u\n", gsl_rng_default_seed);
  //printf("first value = %u\n", gsl_rng_get (r));
  //return 0;
  //string logFile = "defaultGameLog";
  //BattleshipWorldStateFormatter formatter(1,3);
  //BattleshipWorldStateFormatter formatter(5,10);
  //BattleshipWorldStateFormatter formatter(2,5);
  //GameLogReader glr("games/LongBattleship.siigl", "games/Battleship/battleship1-3.siigl", "cannedBattle", &formatter);
  //GameLogReader glr("games/LongBattleship.siigl", "games/Battleship/battleship1-3.siigl", "testBattle13", &formatter);
  //Processor::checkPayoffs = true; GameLogReader glr("games/LongBattleship.siigl", "games/Battleship/battleship2-5.siigl", "testbattle25", &formatter);
  //Processor::checkPayoffs = true; GameLogReader glr("games/LongBattleship.siigl", "games/Battleship/battleship5-10.siigl", "nbattles510magic", &formatter);
  
  //GameLogReader glr("games/Gops.siigl", "games/Gops/gops1-3.siigl", "gops13x1", &formatter);
  //Processor::checkPayoffs = false;GameLogReader glr("games/Gops.siigl", "games/Gops/gops10-60.siigl", "abigGops", &formatter);
  //GameLogReader glr("games/Racko.siigl", "games/Racko/racko10-60.siigl", "racko1060", &formatter);
  //RackoWorldStateFormatter formatter(5,40); 
  //GameLogReader glr("games/Racko.siigl", "games/Racko/racko5-40.siigl", "Logs/gameLogRacko5-40", &formatter);
  //GameLogReader glr("games/Racko.siigl", "games/Racko/racko5-40.siigl", "humangameRacko540", &formatter);
  //GameLogReader glr("games/Racko.siigl", "games/Racko/racko5-40.siigl", "newhard540", &formatter);
  //GameLogReader glr("games/Racko.siigl", "games/Racko/racko5-20.siigl", "oneGame", &formatter);

  //DefaultWorldStateFormatter formatter;
  //Processor::checkPayoffs = false;
  //GameLogReader glr("games/Gops.siigl", "games/Gops/gops10-45.siigl", "Logs/Gops/gameLogGops10-45", &formatter);
  //glr.sumPayoffsOverManyGames();
  //return 0;






  Game gameid = MASTERMIND;

  switch (gameid) {
    case RACKO:
    Processor::checkPayoffs = true;
    for (unsigned slotCount = 5; slotCount <= 10; slotCount++) {
      for (unsigned cardCount = 20; cardCount <= 60; cardCount += 10) {
        RackoWorldStateFormatter formatter(slotCount,cardCount); 
        string domainFile = "games/Racko.siigl";
        ostringstream problemStream;
        problemStream << "games/Racko/racko" << slotCount << "-" << cardCount << ".siigl";
        ostringstream logStream;
        logStream << "Logs/gameLogRacko" << slotCount << "-" << cardCount;
        cout << "Processing game logs for: " << domainFile << " " << problemStream.str() << " " << logStream.str() << std::endl;
        GameLogReader gameLogReader(domainFile, problemStream.str(), logStream.str(),&formatter);
        gameLogReader.sumPayoffsOverManyGames();
        //GameLogGenerator gameLogGenerator;
        //gameLogGenerator.generateGames(logStream.str(), domainFile, problemStream.str(), 5);
      }
    }
    break;
  
    case BATTLESHIP:
    Processor::checkPayoffs = true;
    for (unsigned shipCount = 1; shipCount <= 5; shipCount++) {
      for (unsigned cellsPerSideCount = 4; cellsPerSideCount <= 10; cellsPerSideCount += 2) {
        BattleshipWorldStateFormatter formatter(shipCount,cellsPerSideCount); 
        string domainFile = "games/LongBattleship.siigl";
        ostringstream problemStream;
        problemStream << "games/Battleship/battleship" << shipCount << "-" << cellsPerSideCount << ".siigl";
        ostringstream logStream;
        logStream << "Logs/Battleship/gameLogBattleship" << shipCount << "-" << cellsPerSideCount;
        cout << "Processing game logs for: " << domainFile << " " << problemStream.str() << " " << logStream.str() << std::endl;
        GameLogReader gameLogReader(domainFile, problemStream.str(), logStream.str(),&formatter);
        gameLogReader.sumPayoffsOverManyGames();
      }
    }
    break;

    case GOPS:
    Processor::checkPayoffs = false;
    for (unsigned slotCount = 2; slotCount <= 10; slotCount += 2) {
      for (unsigned extraCardCount = 0; extraCardCount <= 40; extraCardCount += 10) {
        DefaultWorldStateFormatter formatter; 
        string domainFile = "games/Gops.siigl";
        ostringstream problemStream;
        problemStream << "games/Gops/gops" << slotCount << "-" << extraCardCount << ".siigl";
        ostringstream logStream;
        logStream << "Logs/Gops/gameLogGops" << slotCount << "-" << extraCardCount;
        cout << "Processing game logs for: " << domainFile << " " << problemStream.str() << " " << logStream.str() << std::endl;
        GameLogReader gameLogReader(domainFile, problemStream.str(), logStream.str(),&formatter);
        gameLogReader.sumPayoffsOverManyGames();
      }
    }
    break;

    case MASTERMIND:
    {
    	Processor::checkPayoffs = true;
    	DefaultWorldStateFormatter formatter; 
    	string domainFile = "games/Mastermind4.siigl";
    	ostringstream problemStream;
    	problemStream << "games/mast7.siigl"; // << slotCount << "-" << extraCardCount << ".siigl";
    	ostringstream logStream;
    	logStream << "Logs/mastermind";// << slotCount << "-" << extraCardCount;
    	cout << "Processing game logs for: " << domainFile << " " << problemStream.str() << " " << logStream.str() << std::endl;
    	GameLogReader gameLogReader(domainFile, problemStream.str(), logStream.str(),&formatter);
    	gameLogReader.sumPayoffsOverManyGames();
    }
    break;

    default:
	std::cerr << "Game not yet handled\n";
	
    break;
  }













  return 0;
}
