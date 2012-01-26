#include "StaticEvaluationFunction.h"
#include <iostream>
#include <sstream>
#include <iomanip>

using std::cout;
using std::endl;
using std::ostringstream;

namespace VAL {

StaticEvaluationFunction::StaticEvaluationFunction() {

}

StaticEvaluationFunction::~StaticEvaluationFunction() {

}

void StaticEvaluationFunction::setProcessor(Processor* procIn) {
	this->proc = procIn;
	proc->setStaticEvaluationFunction(this);
}

DefaultStaticEvaluationFunction::DefaultStaticEvaluationFunction() {

}

DefaultStaticEvaluationFunction::~DefaultStaticEvaluationFunction() {

}

NumScalar DefaultStaticEvaluationFunction::getEstimatedValue(const VecInt& gameHistory, WorldState& ws,
		const VecVecVecKey& kb) {
	return 1;
}

RackoStaticEvaluationFunction::RackoStaticEvaluationFunction() {
}

RackoStaticEvaluationFunction::~RackoStaticEvaluationFunction() {

}

NumScalar RackoStaticEvaluationFunction::getEstimatedValue(const VecInt& gameHistory, WorldState& ws,
		const VecVecVecKey& kb) {
	ostringstream os;
	// Number of slot objects is (# slots per player)*2 + 1.  (Extra 1 is "dealer").
	// Integer division by two gives # slots per player.
	unsigned slotCount = (unsigned) (proc->typeCardinalities[proc->typeNameIds["slot"]] / 2);
	unsigned cardCount = (unsigned) (proc->typeCardinalities[proc->typeNameIds["card"]]);
	os << "Racko World with " << slotCount << " slots and " << cardCount << " cards";
	os << " head id for 'at': " << proc->predHeadTbl["at"];
	int atIndex = proc->predHeadTbl["at"];
	int topIndex = proc->predHeadTbl["top"];
	VecInt args(2, 0);
	int whoseTurn = ws.getWhoseTurn();
	switch (whoseTurn) {
	case 0: // chance
		os << "Chance: ";
		args[1] = slotCount; // This will be the slot 'dealer'
		for (int c = 0; c < (int) (((cardCount))); c++) {
			args[0] = c;
			int index = ws.getIndex(args, atIndex);
			if (ws.getTruthValue(index) == KNOWN_TRUE) {
				os << " " << std::setw(2) << (c + 1); //" fact: "  << proc->getFact(index);
			}
		}

		os << "\n";
		break;
	case 1:
	case 2:
		os << "Player " << whoseTurn << "\n";
		int minSlot = (whoseTurn == 2) ? 0 : slotCount + 1;
		int oppMinSlot = (whoseTurn == 1) ? 0 : slotCount + 1;
		for (int s = minSlot; s < (int) ((((minSlot + slotCount)))); s++) {
			os << "   " << (char) (((('A' + s - minSlot)))) << ": "; //" fact: "  << proc->getFact(index);
			args[1] = s;
			for (int c = 0; c < (int) (((cardCount))); c++) {
				args[0] = c;
				int index = ws.getIndex(args, 0);
				if (ws.getTruthValue(index) == KNOWN_TRUE) {
					os << std::setw(2) << (c + 1); //" fact: "  << proc->getFact(index);
				}
			}

			os << "\n";
		}

		if (false)
			for (int s = oppMinSlot; s < (int) ((((oppMinSlot + slotCount)))); s++) {
				os << "   " << (char) (((('A' + s - oppMinSlot)))) << ": "; //" fact: "  << proc->getFact(index);
				args[1] = s;
				for (int c = 0; c < (int) (((cardCount))); c++) {
					args[0] = c;
					int index = ws.getIndex(args, 0);
					if (ws.getTruthValue(index) == KNOWN_TRUE) {
						os << std::setw(2) << (c + 1); //" fact: "  << proc->getFact(index);
					}
				}

				os << "\n";
			}

		os << "\n";
		break;
	}
	for (unsigned c = 0; c < cardCount; c++) {
		if (ws.getTruthValue(topIndex, c) == KNOWN_TRUE) {
			os << "Top: " << (c + 1) << "\n";
		}
	}
	int drawnIndex = proc->predHeadTbl["drawn"];
	for (unsigned c = 0; c < cardCount; c++) {
		for (unsigned ithDraw = 0; ithDraw < cardCount; ithDraw++) {
			if (ws.getTruthValue(drawnIndex, c, ithDraw) == KNOWN_TRUE) {
				os << "Drawn: " << (c + 1) << "\n";
			}
		}
	}
	return 1;
}

BattleshipStaticEvaluationFunction::~BattleshipStaticEvaluationFunction() {
}

BattleshipStaticEvaluationFunction::BattleshipStaticEvaluationFunction() {
}

NumScalar BattleshipStaticEvaluationFunction::getEstimatedValue(const VecInt& gameHistory, WorldState & ws,
		const VecVecVecKey& kb) {
	ostringstream os;
	//os << "Battleship World with " << shipCount << " ships and " << gridPointsPerSide << " cards";
	//os << " head id for 'at': " << proc->predHeadTbl["at"];
	int occupiedIndex = proc->predHeadTbl["occupied"];
	int guessedIndex = proc->predHeadTbl["guessed"];
	int totalhitsIndex = proc->getFunctionId("totalhits");
	unsigned gridPointsPerSide = proc->typeCardinalities[proc->typeNameIds["row"]];
	NumScalar hits1 = ws.getFluentValue(totalhitsIndex,1);
	NumScalar hits2 = ws.getFluentValue(totalhitsIndex,2);

	//int atIndex = proc->predHeadTbl["at"];
	VecInt args(2, 0);
	int whoseTurn = ws.getWhoseTurn();
	int guessedFactIndex;
	int occupiedFactIndex;
	TruthState guessedTruthValue;
	TruthState occupiedTruthValue;
	switch (whoseTurn) {
	case 0: // chance
		os << "Chance: ";
		break;
	case 1:
	case 2:
		VecInt guessedPredicateArgs(3, 0);
		VecInt occupiedPredicateArgs(3, 0);
		os << "Player " << whoseTurn << "\n";
		os << "Mine:\n";
		occupiedPredicateArgs[0] = whoseTurn;
		guessedPredicateArgs[0] = 3 - whoseTurn; // Opponent's id
		for (unsigned r = 0; r < gridPointsPerSide; r++) {
			occupiedPredicateArgs[2] = (int) ((r));
			guessedPredicateArgs[2] = (int) ((r));
			for (unsigned c = 0; c < gridPointsPerSide; c++) {
				occupiedPredicateArgs[1] = (int) ((c));
				occupiedFactIndex = proc->getIndex(occupiedPredicateArgs, occupiedIndex);
				occupiedTruthValue = ws.getTruthValue(occupiedFactIndex);
				guessedPredicateArgs[1] = (int) ((c));
				guessedFactIndex = proc->getIndex(guessedPredicateArgs, guessedIndex);
				guessedTruthValue = ws.getTruthValue(guessedFactIndex);
				guessedPredicateArgs[1] = (int) ((c));
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
			occupiedPredicateArgs[2] = (int) ((r));
			guessedPredicateArgs[2] = (int) ((r));
			for (unsigned c = 0; c < gridPointsPerSide; c++) {
				occupiedPredicateArgs[1] = (int) ((c));
				occupiedFactIndex = proc->getIndex(occupiedPredicateArgs, occupiedIndex);
				occupiedTruthValue = ws.getTruthValue(occupiedFactIndex);
				guessedPredicateArgs[1] = (int) ((c));
				guessedFactIndex = proc->getIndex(guessedPredicateArgs, guessedIndex);
				guessedTruthValue = ws.getTruthValue(guessedFactIndex);
				guessedPredicateArgs[1] = (int) ((c));
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

	return 1;
}
}

VAL::EndGameStaticEvaluationFunction::EndGameStaticEvaluationFunction() {
}

VAL::EndGameStaticEvaluationFunction::~EndGameStaticEvaluationFunction() {
}

NumScalar VAL::EndGameStaticEvaluationFunction::getEstimatedValue(const VecInt& gameHistory, WorldState & ws,
		const VecVecVecKey& kb) {
	ostringstream os;
	int scoreIndex = proc->getFunctionId("score");

	int cardCount = proc->typeCardinalities[proc->typeNameIds["card"]];
//	int slotCount = proc->typeCardinalities[proc->typeNameIds["slot"]];

	VecInt cardRow(cardCount);
	NumScalar p1Score = ws.getFluentValue(scoreIndex, 1);
	NumScalar p2Score = ws.getFluentValue(scoreIndex, 2);
	assert((unsigned)cardCount <= gameHistory.size());
	for (unsigned i = 1; i <= (int) cardRow.size(); i++) {
		int index = gameHistory[i];
		VecInt args = proc->operatorIndexToVector(index);
//		os << proc->getOperatorName(args[0]) << " " << proc->asString(args) << "\n";
		cardRow[i - 1] = args[3] + 1; // c0=1 c1=2, etc.
	}assert(!gameHistory.empty());
	int lastMoveIndex = gameHistory[gameHistory.size() - 1];
	VecInt lastMoveArgs = proc->operatorIndexToVector(lastMoveIndex);
	int lastMoveHeadId = lastMoveArgs[0];
	int activePlayerId = lastMoveArgs[1];
	int bonus = 0;
	int chosenCard = -1;
	if (lastMoveHeadId == 1) { // choose-left
		chosenCard = lastMoveArgs[3]-1;
	} else if (lastMoveHeadId == 3) { // choose-right
		chosenCard = lastMoveArgs[4]-1;
	}
	if (chosenCard != -1) {
		assert(chosenCard < (int)cardRow.size());
		bonus = cardRow[chosenCard];
	}
	if (activePlayerId == 1) {
		p1Score += bonus;
	} else if (activePlayerId == 2) {
		p2Score += bonus;
	}
	os << proc->asString(cardRow,LIST) << " in estval with last action " << lastMoveHeadId << proc->operatorIndexToString(lastMoveIndex);
	cout << "EVAL: " << os.str() << "\n";
	return p1Score - p2Score;
}

VAL::DifferenceStaticEvaluationFunction::DifferenceStaticEvaluationFunction()
{
}



VAL::DifferenceStaticEvaluationFunction::~DifferenceStaticEvaluationFunction()
{
}



NumScalar VAL::DifferenceStaticEvaluationFunction::getEstimatedValue(const VecInt & gameHistory, WorldState & ws, const VecVecVecKey & kb)
{
	int scoreIndex = proc->getFunctionId("score");
	NumScalar p1Score = ws.getFluentValue(scoreIndex, 1);
	NumScalar p2Score = ws.getFluentValue(scoreIndex, 2);
	return -p1Score + p2Score;
}





// VAL
