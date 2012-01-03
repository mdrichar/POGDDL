#include "GameLogGenerator.h"
#include "GameModerator.h"
#include "InfoSetGenerator.h"
#include "WorldStateFormatter.h"
#include "processor.h"
#include "typecheck.h"
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <sstream>
#include "PerformanceCounters.h"

#define PARSE_DEBUG 0
extern int yyparse();
extern int yydebug;
using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;
namespace VAL {
  
  parse_category* top_thing = NULL;
  
  analysis* current_analysis;
  
  yyFlexLexer* yfl;
  bool Silent;
  int errorCount;
  bool Verbose;
  bool ContinueAnyway;
  bool ErrorReport;
  bool InvariantWarnings;
  bool LaTeX;
  
  ostream * report = &cout;
  
  bool makespanDefault;

// Caller is responsible to delete the analysis that is produced by this function
analysis* GameParser::parseGame(const string& domainString, const string& problemString)
{
  	Silent = false;
  	errorCount = 0;
  	Verbose = PARSE_DEBUG ;
  	ContinueAnyway = false;
  	ErrorReport = false;
  VAL::current_analysis = new analysis;
  //yyFlexLexer* yfl;
  ifstream domainFile(domainString.c_str());
  if(!domainFile) 
  {
  	cerr << "Bad domain file!\n";
  	if(LaTeX) *report << "\\section{Error!} Bad domain file! \n \\end{document}\n";
  	exit(1);
  };
  
  VAL::yfl= new yyFlexLexer(&domainFile,&cout);
  
  yydebug = PARSE_DEBUG;
  int parseErrorCode = yyparse();
  assert(parseErrorCode == 0);
  delete VAL::yfl;
  
  if(!current_analysis->the_domain)
  {
  	cerr << "Problem in domain definition!\n";
  	if(LaTeX) *report << "\\section{Error!} Problem in domain definition! \n \\end{document}\n";
  	exit(1);
  } else {
  
  }   
  if (Verbose) {
      if (current_analysis->the_domain)
      {
          cout << "Domain parsed successfully\n";
      }
      
  }
  TypeChecker tc(VAL::current_analysis);
  
      if(LaTeX) Verbose = false;
  bool typesOK = tc.typecheckDomain();
  //return 0;
  
  if(LaTeX) Verbose = true;
  
  if(!typesOK)
  {
  	cerr << "Type problem in domain description!\n";
  	
  	if(LaTeX)
  	{
  		*report << "\\section{Error!} Type problem in domain description! \n \\begin{verbatim}";
  		tc.typecheckDomain();
  		*report << "\\end{verbatim} \\end{document}\n";
  	};
  	
  
  	exit(1);
  };
  
  ifstream problemFile(problemString.c_str());
  if(!problemFile)
  {
  	cerr << "Bad problem file!\n";
  	if(LaTeX) *report << "\\section{Error!} Bad problem file! \n \\end{document}\n";
  	exit(1);
  };
  
  yfl = new yyFlexLexer(&problemFile,&cout);
  parseErrorCode = yyparse();
  assert(parseErrorCode == 0);
  delete yfl;
  
  if(!tc.typecheckProblem())
  {
  	cerr << "Type problem in problem specification!\n";
  	if(LaTeX) *report << "\\section{Error!} Type problem in problem specification!\n \\end{document}\n";
  	exit(1);
  } else if (Verbose) {
  	cout << "Problem type-checking completed.\n";
  };
  return VAL::current_analysis;
}









GameLogGenerator::GameLogGenerator(VAL::Processor* p, const VecPlayerType& players)
{

}

void GameLogGenerator::generateGames(const string& logString, const string& domainString, const string& problemString, unsigned nGamesToPlay)
{
        // Parse the game description
  	this->logString = logString;
        analysis* an_analysis = GameParser::parseGame(domainString,problemString);
        this->p = new VAL::Processor(an_analysis);
        //return;
	
        // Set the players to use Monte Carlo Tree Search
        VecPlayerType vpt(p->initialWorld.getNRoles());
        //vpt[0] = P_HUMAN;
        vpt[0] = P_RANDOM;  // Chance player 
        vpt[1] = P_RANDOM;
        if (vpt.size() == 3) vpt[2] = P_RANDOM;
	int nGamesPlayed = 0;
        GameModerator gm(p,p->initialWorld,vpt);
	FileGameLogger fileGameLogger(logString);
        VecPayoff payoffs = gm.playManyGames((int)nGamesToPlay,0,1,nGamesPlayed, fileGameLogger);
        cout << "Totals: " << p->asString(payoffs) << "\n";
        delete an_analysis;
	delete this->p;
}

GameLogGenerator::GameLogGenerator()
  : p(0), logString("defaultGameLog")
{

}

GameLogGenerator::~GameLogGenerator()
{

}

void generateGame()
{

}

};




GameLogger::GameLogger()
{

}

GameLogger::~GameLogger()
{

}

void GameLogger::reset()
{
  gameLogVec = VecInt(1,-1);
}






DefaultGameLogger::DefaultGameLogger()
{
  this->gameLogFilename = string("defaultGameLog");
}

DefaultGameLogger::~DefaultGameLogger()
{

}

void DefaultGameLogger::append(unsigned currentMove)
{
  gameLogVec.push_back(currentMove);
}

void DefaultGameLogger::close()
{
  std::ofstream gameLogFile(gameLogFilename.c_str(), std::ios::out|std::ios::app);
  gameLogFile << gameLogVec.size() << " ";
  copy(gameLogVec.begin(), gameLogVec.end(), std::ostream_iterator<unsigned>(gameLogFile," "));
  gameLogFile << "\n";
  gameLogFile.close();
}





FileGameLogger::FileGameLogger(const string& filename)
{
  this->gameLogFilename = filename;
}

FileGameLogger::~FileGameLogger()
{

}

void FileGameLogger::append(unsigned currentMove)
{
  gameLogVec.push_back(currentMove);
}

void FileGameLogger::close()
{
  std::ofstream gameLogFile(gameLogFilename.c_str(), std::ios::out|std::ios::app);
  gameLogFile << gameLogVec.size() << " ";
  copy(gameLogVec.begin(), gameLogVec.end(), std::ostream_iterator<unsigned>(gameLogFile," "));
  gameLogFile << "\n";
  gameLogFile.close();
}





GameLogReader::GameLogReader(const string& domainString, const string& problemString, const string& logString, WorldStateFormatter* formatter)
{
  	this->logString = logString;
        this->an_analysis = GameParser::parseGame(domainString,problemString);
        this->p = new VAL::Processor(an_analysis);
	formatter->setProcessor(this->p);
}

GameLogReader::~GameLogReader()
{
  delete this->p;
  this->p = 0;
  delete this->an_analysis;
  this->an_analysis = 0;
}

void GameLogReader::sumPayoffsOverManyGames()
{
  std::ifstream gameLogInFile(logString.c_str(),std::ios::in);
  unsigned currentLineNumber = 0;
  unsigned gameCount = 0; // number of games analyzed
  unsigned overallOperationCountDFS = 0; // number of expansions and legality checks over all instances
  unsigned overallSolutionCountDFS = 0; // Number of information set nodes found over all instances
  unsigned overallOperationCountCBS = 0; // number of expansions and legality checks over all instances
  unsigned overallSolutionCountCBS = 0; // Number of information set nodes found over all instances
  unsigned totalMoves = 0;
  while (!gameLogInFile.eof()) {
    string oneGameString;
    getline(gameLogInFile, oneGameString);
    std::istringstream oneGameStream(oneGameString);
    unsigned moveCount = 0;
    oneGameStream >> moveCount;
    if (!oneGameStream.eof()) {
      PerformanceCounters::resetCounters();
      ++currentLineNumber;
      WorldState currentGameState = p->initialWorld;
      p->kb.clear();
      ActionGraph::fluentHistory.resize(1); // Reset fluent history
      //cout << p->printState(currentGameState) << endl;
      VecInt choiceVector(moveCount);
      cout << "Moves: " << moveCount << endl;
      unsigned targetMove = moveCount * 0.5;
      //unsigned targetMove = 26; 
      bool foundAcceptable = false;
      for (unsigned i = 0; i < moveCount; i++) {
	unsigned chosenAction;
        oneGameStream >> chosenAction; 
        choiceVector[i] = chosenAction;
        if (i != 0) { // Throw away 0th item
          cout << i << ": " << chosenAction << " " << p->operatorIndexToString(chosenAction) << endl;
          if (i > targetMove && currentGameState.getWhoseTurn() != 0) {
            //cout << "Generatable" << endl;
            foundAcceptable = true;
	    break;
          }
          p->apply(chosenAction,currentGameState);
          p->finalizeApply(currentGameState);
          ActionGraph::fluentHistory.push_back(currentGameState.getFluents());
        }
      }
      if (!foundAcceptable) {
	cout << currentLineNumber << " Infoset: No non-chance moves after " << targetMove << endl;
      } else {
  	//if (gmverbose) cout << "CHOOSE MOVE BEGIN INFOSET GENERATION\n";
  	const unsigned int maxSize = 30;
  	const unsigned int augmentedRequest =200;
	const unsigned int pid = currentGameState.getWhoseTurn();
	const PlayerType pt = P_MCTS;
  	unsigned requestSize = (pt == P_INFER) ? augmentedRequest : maxSize;
        cout << p->getFormattedState(currentGameState) << endl;

	// Do it once using depth first search and once using constraint-satisfaction search
        PerformanceCounters::resetCounters();
  	InfoSetGenerator isg(p,p->kb,p->initialWorld);
  	SetVecInt infoset = isg.generateN(pid,requestSize); // Parker05 DFS
	cout << currentLineNumber << " Infoset size using dfs: " << infoset.size() << "  First action: " << choiceVector[1] << endl;
	// Accumulate
        cout << "DFS: " << PerformanceCounters::getPerformanceReportAsString() << endl;
	overallSolutionCountDFS += PerformanceCounters::solutionCount;
	overallOperationCountDFS += PerformanceCounters::total;


        PerformanceCounters::resetCounters();
  	InfoSetGenerator isgCBS(p,p->kb,p->initialWorld);
  	SetVecInt infosetCBS = isgCBS.generate(pid,requestSize); // CSIG
	cout << currentLineNumber << " Infoset size using cbs: " << infosetCBS.size() << "  First action: " << choiceVector[1] << endl;
	// Accumulate
        cout << "CBS: " << PerformanceCounters::getPerformanceReportAsString() << endl;
	overallSolutionCountCBS += PerformanceCounters::solutionCount;
	overallOperationCountCBS += PerformanceCounters::total;

	gameCount++;
        totalMoves += moveCount;
      }
      //p->computePayoffs(currentGameState);
      //VecPayoff payoffs = p->payoffs;
      //cout << "Payoffs: " << VAL::Processor::asString(payoffs) << endl;
    }
  }
  float overallEfficiencyDFS = overallOperationCountDFS / (float) overallSolutionCountDFS;
  float overallEfficiencyCBS = overallOperationCountCBS / (float) overallSolutionCountCBS;
  float avgMoveCount = totalMoves / (float) gameCount;
  cout << "#games: " << gameCount << " avg.dpth: " << avgMoveCount 
	<< " DFS found: " << overallSolutionCountDFS << " ops: " << overallOperationCountDFS << " efficiency: " << overallEfficiencyDFS
	<< " CBS found: " << overallSolutionCountCBS << " ops: " << overallOperationCountCBS << " efficiency: " << overallEfficiencyCBS
	 << " " << logString << endl;
}
