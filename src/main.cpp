#include "GameLogGenerator.h"
#include <iostream>
#include <sstream>
#include <string>
#include <gsl/gsl_rng.h>
#include "WorldStateFormatter.h"

using std::ostringstream;

char* current_filename; // file global ( I don't know why this is necessary)

int main(int argc, char * argv[]) {
	ostringstream problemStream;
	ostringstream logStream;
	string domainFile;
	DefaultWorldStateFormatter formatter;

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
	//GameLogReader glr("games/LongBattleship.pog", "games/Battleship/battleship1-3.pog", "cannedBattle", &formatter);
	//GameLogReader glr("games/LongBattleship.pog", "games/Battleship/battleship1-3.pog", "testBattle13", &formatter);
	Processor::checkPayoffs = true; GameLogReader glr("../domains/LongBattleship.pog", "../problems/Battleship/battleship-2.4.pog", "../debuglogs/battle-2.4.log", &formatter);
	//Processor::checkPayoffs = true; GameLogReader glr("games/LongBattleship.pog", "games/Battleship/battleship5-10.pog", "nbattles510magic", &formatter);

	//GameLogReader glr("games/Gops.pog", "games/Gops/gops1-3.pog", "gops13x1", &formatter);
	//Processor::checkPayoffs = false;GameLogReader glr("games/Gops.pog", "games/Gops/gops10-60.pog", "abigGops", &formatter);
	//GameLogReader glr("games/Racko.pog", "games/Racko/racko10-60.pog", "racko1060", &formatter);
	//RackoWorldStateFormatter formatter(5,40);
	//GameLogReader glr("games/Racko.pog", "games/Racko/racko5-40.pog", "Logs/gameLogRacko5-40", &formatter);
	//GameLogReader glr("games/Racko.pog", "games/Racko/racko5-40.pog", "humangameRacko540", &formatter);
	//GameLogReader glr("games/Racko.pog", "games/Racko/racko5-40.pog", "newhard540", &formatter);
	//GameLogReader glr("games/Racko.pog", "games/Racko/racko5-20.pog", "oneGame", &formatter);

	//DefaultWorldStateFormatter formatter;
	//Processor::checkPayoffs = false;
	//GameLogReader glr("games/Gops.pog", "games/Gops/gops10-45.pog", "Logs/Gops/gameLogGops10-45", &formatter);
	glr.sumPayoffsOverManyGames();
	return 0;

//	Game gameid = ENDGAME;

//	switch (gameid) {
//	case ENDGAME:
		Processor::checkPayoffs = false;
		domainFile = "../domains/EndGame.pog";
		problemStream << "../problems/EndGame/endgame-1.pog";
		logStream << "../logs/EndGame/endgame";
		cout << "Processing game logs for: " << domainFile << " " << problemStream.str() << " " << logStream.str()
				<< std::endl;
		GameLogReader gameLogReader(domainFile, problemStream.str(), logStream.str(), &formatter);
		gameLogReader.sumPayoffsOverManyGames();
//		break;
//
//	case RACKO:
//		Processor::checkPayoffs = true;
//		for (unsigned slotCount = 5; slotCount <= 10; slotCount++) {
//			for (unsigned cardCount = 20; cardCount <= 60; cardCount += 10) {
//				domainFile = "games/Racko.pog";
//				problemStream << "games/Racko/racko" << slotCount << "-" << cardCount << ".pog";
//				logStream << "Logs/gameLogRacko" << slotCount << "-" << cardCount;
//				cout << "Processing game logs for: " << domainFile << " " << problemStream.str() << " "
//						<< logStream.str() << std::endl;
//				GameLogReader gameLogReader(domainFile, problemStream.str(), logStream.str(), &formatter);
//				gameLogReader.sumPayoffsOverManyGames();
//				//GameLogGenerator gameLogGenerator;
//				//gameLogGenerator.generateGames(logStream.str(), domainFile, problemStream.str(), 5);
//			}
//		}
//		break;
//
//	case BATTLESHIP:
//		Processor::checkPayoffs = true;
//		for (unsigned shipCount = 1; shipCount <= 5; shipCount++) {
//			for (unsigned cellsPerSideCount = 4; cellsPerSideCount <= 10; cellsPerSideCount += 2) {
//				domainFile = "games/LongBattleship.pog";
//				problemStream << "games/Battleship/battleship" << shipCount << "-" << cellsPerSideCount << ".pog";
//				logStream << "Logs/Battleship/gameLogBattleship" << shipCount << "-" << cellsPerSideCount;
//				cout << "Processing game logs for: " << domainFile << " " << problemStream.str() << " "
//						<< logStream.str() << std::endl;
//				GameLogReader gameLogReader(domainFile, problemStream.str(), logStream.str(), &formatter);
//				gameLogReader.sumPayoffsOverManyGames();
//			}
//		}
//		break;
//
//	case GOPS:
//		Processor::checkPayoffs = false;
//		for (unsigned slotCount = 2; slotCount <= 10; slotCount += 2) {
//			for (unsigned extraCardCount = 0; extraCardCount <= 40; extraCardCount += 10) {
//				domainFile = "games/Gops.pog";
//				problemStream << "games/Gops/gops" << slotCount << "-" << extraCardCount << ".pog";
//				logStream << "Logs/Gops/gameLogGops" << slotCount << "-" << extraCardCount;
//				cout << "Processing game logs for: " << domainFile << " " << problemStream.str() << " "
//						<< logStream.str() << std::endl;
//				GameLogReader gameLogReader(domainFile, problemStream.str(), logStream.str(), &formatter);
//				gameLogReader.sumPayoffsOverManyGames();
//			}
//		}
//		break;
//
//	case MASTERMIND:
//		Processor::checkPayoffs = true;
//		domainFile = "games/Mastermind4.pog";
//		problemStream << "games/mast7.pog"; // << slotCount << "-" << extraCardCount << ".pog";
//		logStream << "Logs/mastermind"; // << slotCount << "-" << extraCardCount;
//		cout << "Processing game logs for: " << domainFile << " " << problemStream.str() << " " << logStream.str()
//				<< std::endl;
//		GameLogReader mastermindLogReader(domainFile, problemStream.str(), logStream.str(), &formatter);
//		mastermindLogReader.sumPayoffsOverManyGames();
//
//		break;
//
//	default:
//		std::cerr << "Game not yet handled\n";
//
//		break;
//	}

	return 0;
}
