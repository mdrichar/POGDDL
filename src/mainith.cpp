#ifdef MYMPI
#include "mpi.h"
#endif

#include <string.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <exception>
#include <numeric> // accumulate
#include "ptree.h"
#include "FlexLexer.h"
#include "main.h"
#include "typecheck.h"
#include "processor.h"
#include "Node.h"
#include "GameModerator.h"
#include "GameLogGenerator.h"
#include "WorldStateFormatter.h"

using std::ifstream;
using std::ofstream;
using std::cerr;
using std::cout;
using std::for_each;
using std::copy;
using std::exception;

extern int yyparse();
extern int yydebug;

void usage()
{
	cout << "pog: Stochastic and Imperfect Information Game Language\n"
             << "Usage: validate domainFile problemFile messageFile\n";

};

using namespace VAL;
char * current_filename;

void assignGame(int gameid, bool& alwaysCheckPayoffs, string& domainString, string& problemString)
{
  switch (gameid) {
	case 1: 
		domainString = "../domains/Racko.pog";
		//problemString = "games/r3-9.pog";
		problemString = "../problems/Racko/racko-5.20.pog";
		//problemString = "games/Racko/racko10-60.pog";
		//problemString = "games/Racko/racko5-40.pog";
		alwaysCheckPayoffs = true;
		break;
	case 2: 
		domainString = "games/GoFish.pog";
		problemString = "games/gf1.pog";
		break;
	case 3: 
		domainString = "games/Clue.pog";
		problemString = "games/c1.pog";
		break;
	case 4: 
		domainString = "games/Stratego.pog";
		problemString = "games/s1.pog";
		break;
	case 5: 
		domainString = "games/Scrabble.pog";
		problemString = "games/a1.pog";
		break;
	case 6: 
		domainString = "games/Simplest.pog";
		problemString = "games/simp1.pog";
		break;
	case 7: 
		domainString = "games/Game2.pog";
		problemString = "games/Game2p.pog";
		break;
	case 8: 
		domainString = "games/Game3.pog";
		problemString = "games/Game3p.pog";
		break;
	case 9: 
		domainString = "games/Game4.pog";
		problemString = "games/Game4p.pog";
		break;
	case 10: 
		domainString = "games/Game5.pog";
		problemString = "games/Game5p.pog";
		break;
	case 11: 
		domainString = "games/Game6.pog";
		problemString = "games/Game6p.pog";
		break;
	case 12: 
		domainString = "games/Game7.pog";
		problemString = "games/Game7p.pog";
		break;
	case 13: 
		domainString = "games/Digram.pog";
		problemString = "games/dr.pog";
		break;
	case 14: 
		domainString = "games/Matching.pog";
		problemString = "games/m1.pog";
		alwaysCheckPayoffs = false;
		break;
	case 15: 
		domainString = "games/Yahtzee.pog";
		problemString = "games/y1.pog";
		break;
	case 16: 
		domainString = "games/Gops.pog";
		problemString = "games/g1.pog";
		break;
	case 17: 
		domainString = "games/Kriegspiel.pog";
		problemString = "games/k1.pog";
		break;
	case 18: 
		domainString = "games/Territory.pog";
		problemString = "games/terr3.pog";
		break;
	case 19: 
		domainString = "games/Mastermind.pog";
		problemString = "games/mast1.pog";
		break;
	case 20: 
		domainString = "games/Battleship.pog";
		problemString = "games/b1.pog";
		alwaysCheckPayoffs = true;
		break;
	case 21: 
		domainString = "games/LongBattleship.pog";
		problemString = "games/Battleship/battleship2-6.pog";
		//problemString = "games/Battleship/battleship5-10.pog";
		//problemString = "games/Battleship/battleship1-3.pog";
		alwaysCheckPayoffs = true;
		break;
	case 22: 
		domainString = "games/ShortLatentTicTacToe.pog";
		problemString = "games/t1.pog";
		//problemString = "games/Battleship/battleship5-10.pog";
		//problemString = "games/Battleship/battleship1-3.pog";
		alwaysCheckPayoffs = true;
		break;
	case 23: 
		domainString = "games/Unimind.pog";
		problemString = "games/unimind3.pog";
		//problemString = "games/Battleship/battleship5-10.pog";
		//problemString = "games/Battleship/battleship1-3.pog";
		alwaysCheckPayoffs = false;
		break;
	case 24: 
		domainString = "../domains/EndGame.pog";
		problemString = "../problems/EndGame/endgame-1.pog";
		//problemString = "games/Battleship/battleship5-10.pog";
		//problemString = "games/Battleship/battleship1-3.pog";
		alwaysCheckPayoffs = false;
		break;
	default:
		assert(false);
  }
}

void processArgsFile(string filename, VecPlayerType& players, bool& alwaysCheckPayoffs, string& domainString, string& problemString, int& nGames)
{
  const bool Verbose = true;
  VecStr args;
  ifstream infile(filename.c_str(), std::ios::in);
  if (!infile) {
    cerr << "Error reading file: " << filename << std::endl;
    exit(1);
  }
  string oneline;
  while (!infile.eof()) {
    getline(infile,oneline);
    if (oneline == "") break;
    std::istringstream linestring(oneline);
    string nextArg;
    linestring >> nextArg; // dummy text at the front of a line
    linestring >> nextArg; // actual arg 
    args.push_back(nextArg);
  }
  StringToPlayerType playerTypes = GameModerator::getPlayerTypes();
  for (unsigned i = 0; i < args.size(); i++) {
    if (Verbose) cout << i << " " << args[i] << "\n";
    switch (i) {
	case 0: 
          players[0] = playerTypes[args[0]];
	  break;
	case 1: 
          players[1] = playerTypes[args[1]];
	  break;
	case 2: 
          players[2] = playerTypes[args[2]];
	  break;
	case 3:
	  assignGame(atoi(args[3].c_str()),alwaysCheckPayoffs,domainString,problemString);
	  break;
	case 4:
	  nGames = atoi(args[4].c_str());
          break;
	case 5:
	  GameModerator::manySamples = atoi(args[5].c_str());
          break;
	case 6:
	  GameModerator::fewSamples = atoi(args[6].c_str());
          break;
	case 7:
	  GameModerator::maxSize = atoi(args[7].c_str());
          break;

    }
  }
  if (Verbose) {
    cout << "PT0: " << GameModerator::typeString(players[0]) << std::endl;
    cout << "PT1: " << GameModerator::typeString(players[1]) << std::endl;
    cout << "PT2: " << GameModerator::typeString(players[2]) << std::endl;
    cout << "Game: " << domainString << " " << problemString << std::endl;
    cout << "nGames: " << nGames << std::endl;
    cout << "maxSamples: " << GameModerator::manySamples << std::endl;
    cout << "minSamples: " << GameModerator::fewSamples << std::endl;
    cout << "infoSet size: " << GameModerator::maxSize << std::endl;
  }
}

int main(int argc,char * argv[])
{
        VecNode actualNodes;
        Node::nodes = &actualNodes;
        Node::nodeVec().resize(10000);
        Processor::clockTotal = 0.0; 
  	//Silent = false;
  	//errorCount = 0;
  	//Verbose = 0; //PARSE_DEBUG ;
  	//ContinueAnyway = false;
  	//ErrorReport = false;

  	int rank = 0;
  	int size = 1;
#ifdef MYMPI
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&size);
  cout << "Myrank: " << rank << std::endl;
#endif
        string domainString = "games/Clue.pog";
        string problemString = "games/c1.pog";
        int game = 0;
        int nMoves = 6;
        int nGames = 1;
        VecPlayerType playerFileTypes;
        playerFileTypes.resize(3);
        playerFileTypes[0] = playerFileTypes[1] = playerFileTypes[2] = P_RANDOM;
        bool alwaysCheckPayoffs = false;
        if (argc == 2) {
          processArgsFile(argv[1],playerFileTypes, alwaysCheckPayoffs, domainString, problemString, nGames);
        } else {
          if (argc > 1) {
            game = atoi(argv[1]);
            assignGame(game, alwaysCheckPayoffs, domainString, problemString);
          }
          if (argc >= 3) {
            nMoves = atoi(argv[2]);
            nGames = atoi(argv[2]);
          }
          if (argc >= 4) {
            //Node::nSamples = atoi(argv[3]);
            Node::nSamples = -1;
            GameModerator::manySamples = atoi(argv[3]); 
            if (argc >= 5) {
              GameModerator::fewSamples = atoi(argv[4]); 
            } else {
              GameModerator::fewSamples = GameModerator::manySamples; 
            }
          }
        }

        //current_analysis= &an_analysis;
	string s;
	
        int argcount = 1;


	if(argcount>argc) 
	{
		printf("argcount %d argc %d\n",argcount,argc);
		usage();
		return 0;
	};
        analysis* an_analysis = GameParser::parseGame(domainString,problemString);
        VAL::Processor p(an_analysis);
        bool pverbose = false;
	if (pverbose) cout << "Printing state\n";
	if (pverbose) p.printState();
        srand(7);
	// Allocate space for nodes up front
        Node::gproc = &p;

	Processor::checkPayoffs = alwaysCheckPayoffs;
        VecPlayerType vpt(p.initialWorld.getNRoles());
        //vpt[0] = P_HUMAN;
        vpt[0] = playerFileTypes[0];
        vpt[1] = playerFileTypes[1];
	if (vpt.size() == 3) { 
	  vpt[2] = playerFileTypes[2];
        }

        int nGamesPlayed = 0;  // Tracks number of games generated
	//GameLogGenerator gameLogGenerator(&p, vpt);
        //gameLogGenerator.generateGames(domainString, problemString, nGames);
        //return 0; 
        GameModerator gm(&p,p.initialWorld,vpt);
 	DefaultGameLogger defaultGameLogger;
	DefaultWorldStateFormatter formatter;
	//RackoWorldStateFormatter formatter(5,20);
	//RackoWorldStateFormatter formatter(10,60);
	//BattleshipWorldStateFormatter formatter(2,6);
	//BattleshipWorldStateFormatter formatter(1,3);
	formatter.setProcessor(&p);
        VecPayoff payoffs = gm.playManyGames(nGames,rank,size,nGamesPlayed, defaultGameLogger);
        cout << "Totals: " << p.asString(payoffs) << "\n";
#ifdef MYMPI
    int allGamesPlayed;
    MPI_Reduce(&nGamesPlayed,&allGamesPlayed,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
    VecPayoff allPayoffs(p.initialWorld.getNRoles());
    MPI_Reduce(&payoffs[0],&allPayoffs[0],allPayoffs.size(),MPI_FLOAT,MPI_SUM,0,MPI_COMM_WORLD);
    MPI_Finalize();  
    if (rank == 0) {
      cout << "NGames: " << allGamesPlayed;
      for (unsigned i = 1; i < allPayoffs.size(); i++) {
        cout << " " << allPayoffs[i];
      }
      cout << std::endl;
    }
#endif
    return 0;
};
