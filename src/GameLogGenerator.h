#ifndef GAME_LOG_GENERATOR
#define GAME_LOG_GENERATOR

#include "ptree.h"
#include "FlexLexer.h"
#include "processor.h"


namespace VAL {

enum Game {RACKO,BATTLESHIP,GOPS,MASTERMIND,MATCHING} ;
class WorldStateFormatter;
class GameParser
{
public:
  static analysis only_analysis;
  static analysis* parseGame(const string& domainString, const string& problemString);  
};

class GameLogGenerator
{
public:
  VAL::Processor* p;
  string logString;
  GameLogGenerator(Processor* p, const VecPlayerType& players);
  GameLogGenerator();
  ~GameLogGenerator();
  void generateGames(const string& logString, const string& domainString, const string& problemString, unsigned nGamesToPlay);
  void generateGame();
};

class GameLogger
{
public:
  GameLogger();
  virtual ~GameLogger();
  virtual void reset();
  virtual void append(unsigned currentMove) = 0;
  virtual void close() = 0;

protected:
  string gameLogFilename;
  VecInt gameLogVec;
};

class DefaultGameLogger : public GameLogger
{
public:
  DefaultGameLogger();
  virtual ~DefaultGameLogger();
  virtual void append(unsigned currentMove);
  virtual void close();

};

class FileGameLogger : public GameLogger
{
public:
  FileGameLogger(const string& filename);
  virtual ~FileGameLogger();
  virtual void append(unsigned currentMove);
  virtual void close();

};

class GameLogReader
{
public:
  string logString;
  VAL::Processor* p;
  analysis* an_analysis;
  GameLogReader(const string& domainString, const string& problemString, const string& logString, WorldStateFormatter* formatter);
  ~GameLogReader();
  void sumPayoffsOverManyGames();
};
}
#endif
