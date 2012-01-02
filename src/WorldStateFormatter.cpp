#include "WorldStateFormatter.h"
#include <iostream>
#include <sstream>
#include <iomanip>

using std::cout;
using std::endl;
using std::ostringstream;

namespace VAL {

WorldStateFormatter::WorldStateFormatter()
{

}

WorldStateFormatter::~WorldStateFormatter()
{

}

void WorldStateFormatter::setProcessor(Processor* procIn)
{
  this->proc = procIn;
  proc->setFormatter(this);
}

DefaultWorldStateFormatter::DefaultWorldStateFormatter()
{

}

DefaultWorldStateFormatter::~DefaultWorldStateFormatter()
{

}

std::string DefaultWorldStateFormatter::asString(WorldState& ws)
{
  return string("--Default World State--");
}

RackoWorldStateFormatter::RackoWorldStateFormatter(unsigned slotCountIn, unsigned cardCountIn)
  : slotCount(slotCountIn), cardCount(cardCountIn)
{

}

RackoWorldStateFormatter::~RackoWorldStateFormatter()
{

}

std::string RackoWorldStateFormatter::asString(WorldState& ws)
{
  ostringstream os;
  os << "Racko World with " << slotCount << " slots and " << cardCount << " cards";
  os << " head id for 'at': " << proc->predHeadTbl["at"];
  VecInt args(2,0);
  int whoseTurn = ws.getWhoseTurn();
  switch (whoseTurn) {
    case 0: // chance
	os << "Chance: ";
        args[1] = slotCount; // This will be the slot 'dealer'
	for (int c = 0; c < (int)cardCount; c++) {
	  args[0] = c;
	  int index = ws.getIndex(args,0);
	  if (ws.getTruthValue(index) == KNOWN_TRUE) {
            os << " " << std::setw(2) << (c+1); //" fact: "  << proc->getFact(index);
 	  }
	}
        os << "\n";
	break;
    case 1:
    case 2:
	os << "Player " << whoseTurn << "\n";
        int minSlot = (whoseTurn == 2) ? 0 : slotCount + 1;
        int oppMinSlot = (whoseTurn == 1) ? 0 : slotCount + 1;
	for (int s = minSlot; s < (int)(minSlot + slotCount); s++) {
          os << "   " << (char)('A' + s - minSlot) << ": " ; //" fact: "  << proc->getFact(index);
	  args[1] = s;
	  for (int c = 0; c < (int)cardCount; c++) {
	    args[0] = c;
	    int index = ws.getIndex(args,0);
	    if (ws.getTruthValue(index) == KNOWN_TRUE) {
              os << std::setw(2) << (c+1); //" fact: "  << proc->getFact(index);
 	    }
	  }
          os << "\n";
	}
	if (false)
	for (int s = oppMinSlot; s < (int)(oppMinSlot + slotCount); s++) {
          os << "   " << (char)('A' + s - oppMinSlot) << ": " ; //" fact: "  << proc->getFact(index);
	  args[1] = s;
	  for (int c = 0; c < (int)cardCount; c++) {
	    args[0] = c;
	    int index = ws.getIndex(args,0);
	    if (ws.getTruthValue(index) == KNOWN_TRUE) {
              os << std::setw(2) << (c+1); //" fact: "  << proc->getFact(index);
 	    }
	  }
          os << "\n";
	}
        os << "\n";
 	break;
  } 
  return os.str();
}




















BattleshipWorldStateFormatter::BattleshipWorldStateFormatter(unsigned shipCountIn, unsigned gridPointsPerSideIn)
  : shipCount(shipCountIn), gridPointsPerSide(gridPointsPerSideIn)
{

}

BattleshipWorldStateFormatter::~BattleshipWorldStateFormatter()
{

}

std::string BattleshipWorldStateFormatter::asString(WorldState& ws)
{
  ostringstream os;
  //os << "Battleship World with " << shipCount << " ships and " << gridPointsPerSide << " cards";
  //os << " head id for 'at': " << proc->predHeadTbl["at"];
  int occupiedIndex = proc->predHeadTbl["occupied"];
  int guessedIndex = proc->predHeadTbl["guessed"];
  //int atIndex = proc->predHeadTbl["at"];
  VecInt args(2,0);
  int whoseTurn = ws.getWhoseTurn();
  int guessedFactIndex;
  int occupiedFactIndex;
  TruthState guessedTruthValue;
  TruthState occupiedTruthValue;
  switch (whoseTurn) {
    case 0: // chance
	os << "Chance: ";
    case 1:
    case 2:
	VecInt guessedPredicateArgs(3,0);
	VecInt occupiedPredicateArgs(3,0);
	os << "Player " << whoseTurn << "\n";
	os << "Mine:\n";
	occupiedPredicateArgs[0] = whoseTurn;
	guessedPredicateArgs[0] = 3 - whoseTurn; // Opponent's id
	for (unsigned r = 0; r < gridPointsPerSide; r++) {
	  occupiedPredicateArgs[2] = (int)r;
	  guessedPredicateArgs[2] = (int)r;
	  for (unsigned c = 0; c < gridPointsPerSide; c++) {
	    occupiedPredicateArgs[1] = (int)c;
	    occupiedFactIndex = proc->getIndex(occupiedPredicateArgs,occupiedIndex);
	    occupiedTruthValue = ws.getTruthValue(occupiedFactIndex);
	    guessedPredicateArgs[1] = (int)c;
	    guessedFactIndex = proc->getIndex(guessedPredicateArgs,guessedIndex);
	    guessedTruthValue = ws.getTruthValue(guessedFactIndex);
	    guessedPredicateArgs[1] = (int)c;
            if (guessedTruthValue == KNOWN_TRUE) {
              if (occupiedTruthValue == KNOWN_TRUE) {
                os << "H";
              } else {
                os << "M";
              }
            } else {
              if (occupiedTruthValue == KNOWN_TRUE) {
                os << "1";
              } else {
                os << "0";
              }
            }
		//cout << "factIndex: " << factIndex << " <=> " << proc->getFact(factIndex) << std::endl;
          }
          os << "\n";
        }
 	os << "\n";
	os << "Opponent:\n";
	occupiedPredicateArgs[0] = 3 - whoseTurn;
	guessedPredicateArgs[0] = whoseTurn; // Opponent's id
	for (unsigned r = 0; r < gridPointsPerSide; r++) {
	  occupiedPredicateArgs[2] = (int)r;
	  guessedPredicateArgs[2] = (int)r;
	  for (unsigned c = 0; c < gridPointsPerSide; c++) {
	    occupiedPredicateArgs[1] = (int)c;
	    occupiedFactIndex = proc->getIndex(occupiedPredicateArgs,occupiedIndex);
	    occupiedTruthValue = ws.getTruthValue(occupiedFactIndex);
	    guessedPredicateArgs[1] = (int)c;
	    guessedFactIndex = proc->getIndex(guessedPredicateArgs,guessedIndex);
	    guessedTruthValue = ws.getTruthValue(guessedFactIndex);
	    guessedPredicateArgs[1] = (int)c;
            if (guessedTruthValue == KNOWN_TRUE) {
              if (occupiedTruthValue == KNOWN_TRUE) {
                os << "H";
              } else {
                os << "M";
              }
            } else {
              //if (occupiedTruthValue == KNOWN_TRUE) {
              //  os << "1";
              //} else {
                os << "0";
              //}
            }
		//cout << "factIndex: " << factIndex << " <=> " << proc->getFact(factIndex) << std::endl;
          }
          os << "\n";
        }
 	break;
  } 
  return os.str();
}
} // VAL
