#include "WorldStateFormatter.h"
#include <iostream>
#include <sstream>
#include <iomanip>

using std::cout;
using std::endl;
using std::ostringstream;

namespace VAL {

WorldStateFormatter::WorldStateFormatter() {

}

WorldStateFormatter::~WorldStateFormatter() {

}

void WorldStateFormatter::setProcessor(Processor* procIn) {
	this->proc = procIn;
	proc->setFormatter(this);
}

DefaultWorldStateFormatter::DefaultWorldStateFormatter() {

}

DefaultWorldStateFormatter::~DefaultWorldStateFormatter() {

}

std::string DefaultWorldStateFormatter::asString(const VecInt& gameHistory, WorldState& ws, const VecVecVecKey& kb) {
	return string("--Default World State--");
}

RackoWorldStateFormatter::RackoWorldStateFormatter() {
}

RackoWorldStateFormatter::~RackoWorldStateFormatter() {

}

std::string RackoWorldStateFormatter::asString(const VecInt& gameHistory, WorldState& ws, const VecVecVecKey& kb) {
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
	return os.str();
}

BattleshipWorldStateFormatter::~BattleshipWorldStateFormatter() {
}

BattleshipWorldStateFormatter::BattleshipWorldStateFormatter() {
}

std::string BattleshipWorldStateFormatter::asString(const VecInt& gameHistory, WorldState & ws,
		const VecVecVecKey& kb) {
	ostringstream os;
	//os << "Battleship World with " << shipCount << " ships and " << gridPointsPerSide << " cards";
	//os << " head id for 'at': " << proc->predHeadTbl["at"];
	int occupiedIndex = proc->predHeadTbl["occupied"];
	int guessedIndex = proc->predHeadTbl["guessed"];
	unsigned gridPointsPerSide = proc->typeCardinalities[proc->typeNameIds["row"]];

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

	return os.str();
}
}

VAL::EndGameWorldStateFormatter::EndGameWorldStateFormatter() {
}

VAL::EndGameWorldStateFormatter::~EndGameWorldStateFormatter() {
}

std::string VAL::EndGameWorldStateFormatter::asString(const VecInt& gameHistory, WorldState & ws,
		const VecVecVecKey& kb) {
	ostringstream os;
	int scoreIndex = proc->getFunctionId("score");

	int cardCount = proc->typeCardinalities[proc->typeNameIds["card"]];
//	int slotCount = proc->typeCardinalities[proc->typeNameIds["slot"]];

	VecInt cardRow(cardCount, -1);
	NumScalar p1Score = ws.getFluentValue(scoreIndex, 1);
	NumScalar p2Score = ws.getFluentValue(scoreIndex, 2);
	os << "Score 1: " << p1Score << " Score 2: " << p2Score << "\n";
	//assert((unsigned)cardCount <= gameHistory.size());
	if ((unsigned) cardCount < gameHistory.size()) {
		for (unsigned i = 1; i <= (int) cardRow.size(); i++) {
			int index = gameHistory[i];
			VecInt args = proc->operatorIndexToVector(index);
//		os << proc->getOperatorName(args[0]) << " " << proc->asString(args) << "\n";
			cardRow[i - 1] = args[3] + 1; // c0=1 c1=2, etc.
		}
	}
	os << proc->asString(cardRow, LIST) << "\n";

	return os.str();

}

VAL::DifferenceWorldStateFormatter::DifferenceWorldStateFormatter() {
}

VAL::DifferenceWorldStateFormatter::~DifferenceWorldStateFormatter() {
}

std::string VAL::DifferenceWorldStateFormatter::asString(const VecInt & gameHistory, WorldState & ws,
		const VecVecVecKey & kb) {
	ostringstream os;
	int scoreIndex = proc->getFunctionId("score");
	//int ownsIndex = proc->getPredId("owns");

//	int cardCount = proc->typeCardinalities[proc->typeNameIds["card"]];
//	int slotCount = proc->typeCardinalities[proc->typeNameIds["slot"]];

//	VecInt cardRow(cardCount);
	NumScalar p1Score = ws.getFluentValue(scoreIndex, 1);
	NumScalar p2Score = ws.getFluentValue(scoreIndex, 2);
	os << "Score 1: " << p1Score << " Score 2: " << p2Score << "\n";
//	assert((unsigned)cardCount <= gameHistory.size());
//	for(unsigned i = 1; i <= (int)cardRow.size(); i++) {
//		int index = gameHistory[i];
//		VecInt args = proc->operatorIndexToVector(index);
////		os << proc->getOperatorName(args[0]) << " " << proc->asString(args) << "\n";
//		cardRow[i-1] = args[3]+1; // c0=1 c1=2, etc.
//	}
//	os << proc->asString(cardRow,LIST) << "\n";

	return os.str();
}

VAL::GopsWorldStateFormatter::GopsWorldStateFormatter() {
}

VAL::GopsWorldStateFormatter::~GopsWorldStateFormatter() {
}

std::string VAL::GopsWorldStateFormatter::asString(const VecInt & gameHistory, WorldState & ws,
		const VecVecVecKey & kb) {
	ostringstream os;
	int scoreIndex = proc->getFunctionId("score");
	string oName;
	VecInt oArgs;
	int slotCount = proc->typeCardinalities[proc->typeNameIds["slot"]];
	int biddingRoundCount = (slotCount - 1) / 3; // Subtract 1 for dealer, divide by 3 because for every bidding round, there
	// is the card to bid on plus one card per player to bid with
	VecVecInt startingHands(3);
	VecVecInt results(3);
	for (unsigned i = 0; i < 3; i++) {
		startingHands[i] = VecInt(biddingRoundCount, 0);
		results[i] = VecInt(biddingRoundCount, 0);
	}
	int biddingRoundId = 0;
	int currentBiddable = -1;
	for (unsigned i = 1; i < gameHistory.size(); i++) {
		proc->decodeOperator(gameHistory[i], oName, oArgs, true);
		int cardId = oArgs[0] + 1;
		if (oName == "deal") {
			//os << i << " Deal " << (oArgs[0] + 1) << "\n";
			if (i <= biddingRoundCount) {
				startingHands[0][i - 1] = cardId;
			} else {
				int pid = 2 - ((i - biddingRoundCount - 1) % 2);
				startingHands[pid][(i - biddingRoundCount - 1) / 2] = cardId;
			}
		} else if (oName == "announce") {
			currentBiddable = cardId;
		} else if (oName == "bid1") {
			results[1][biddingRoundId] = cardId;
		} else if (oName == "bid2") {
			results[2][biddingRoundId] = cardId;
		} else if (oName == "determine") {
			int p1Bid = oArgs[0] + 1;
			int p2Bid = oArgs[1] + 1;
			results[0][biddingRoundId] = (p2Bid > p1Bid) + 1; // 1 if p1 won it; 2 if p2 won it
			biddingRoundId++;
		}
	}

	std::string labels[] = { "bids", "p1", "p2" };
	for (unsigned i = 0; i < 3; i++) {
		if (i > 0 && i != ws.getWhoseTurn()) continue; // Don't print the other guys cards
		os << labels[i] << " ";
		for (int j = 0; j < biddingRoundCount; j++) {
			os << startingHands[i][j] << " ";
		}
		os << "\n";
	}

	os << "\n";
	labels[0] = "W ";
	for (unsigned i = 0; i < 3; i++) {
		os << labels[i] << " ";
		for (int j = 0; j < biddingRoundId; j++) {
			os << results[i][j] << " ";
		}
		os << "\n";
	}
	os << "Currently bidding on: " << currentBiddable << "\n";
//	int ownsIndex = proc->getPredId("owns");

//	int cardCount = proc->typeCardinalities[proc->typeNameIds["card"]];
//	int slotCount = proc->typeCardinalities[proc->typeNameIds["slot"]];

//	VecInt cardRow(cardCount);
	NumScalar p1Score = ws.getFluentValue(scoreIndex, 1);
	NumScalar p2Score = ws.getFluentValue(scoreIndex, 2);
	os << "Score 1: " << p1Score << " Score 2: " << p2Score << "\n";

	return os.str();
}

// VAL
