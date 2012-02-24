#include "GameModerator.h"
#include "InfoSetGenerator.h"
#include "NewConditionController.h"
#include "Node.h"

#include <cstdio>
//#define INTERACTIVE

const bool gmverbose = true;
const bool medverbose = false;
const bool showApplying = true;
unsigned GameModerator::maxSize = 30;
unsigned augmentedRequest = 200;
namespace VAL {

int GameModerator::manySamples;
int GameModerator::fewSamples;

StringToPlayerType GameModerator::getPlayerTypes() {
	StringToPlayerType result;
	result["HUMAN"] = P_HUMAN;
	result["MCTS"] = P_MCTS;
	result["RANDOM"] = P_RANDOM;
	result["LIMITED"] = P_MCTS_LIMITED;
	result["INFER"] = P_INFER;
	result["HEURISTIC"] = P_HEURISTIC;
	return result;
}

GameModerator::GameModerator(Processor* p, WorldState& initState_, VecPlayerType& playerIds) :
		proc(p), initState(initState_), players(playerIds) {
	assert((unsigned)initState.getNRoles() == playerIds.size());
}

string GameModerator::typeString(PlayerType pt) {
	switch (pt) {
	case P_HUMAN:
		return "HUMAN";
	case P_MCTS:
		return "MCTS";
	case P_RANDOM:
		return "RANDOM";
	case P_MCTS_LIMITED:
		return "LIMITED";
	case P_INFER:
		return "INFER";
	case P_HEURISTIC:
		return "HEURISTIC";
	default:
		return "UNKNOWN";
	}
	return "IMPOSSIBLE";
}

VecPayoff GameModerator::playGame(Processor* p, const VecPlayerType& players, int seed) {
	DefaultGameLogger defaultLogger;
	return playGame(p, players, seed, defaultLogger);
}

VecPayoff GameModerator::playGame(Processor* proc, const VecPlayerType& players, int seed, GameLogger& gameLogger) {
	gameLogger.reset();
	VecInt gameHistory(1, -1); // 0th action gets -1 as a dummy; first true action is gameHistory[1]
	ActionGraph::fluentHistory.resize(1);
	if (medverbose)
		cout << "Seed: " << seed << "\n";
	if (seed) {
		srand(seed);
	}
	static const bool pverbose = true;
	WorldState current = proc->initialWorld;
	if (pverbose)
		cout << "Initial State*********************************************************************************\n"
				<< proc->printState(current) << "\n";

	proc->kb.clear();
	int moveNumber = 1;
	while (true) {
		VecInt canDo = proc->legalOperators(current);
		if (canDo.empty() || proc->alwaysCheckPayoffs()) {
			proc->computePayoffs(current);
			int sumPayoffs = proc->sumPayoffs();
			if (sumPayoffs > 0 || canDo.empty()) {
				// Game over; Display final info
				if (pverbose)
					cout << proc->printState(current) << "\n";
				if (pverbose)
					cout << "Payoffs: " << proc->asString(proc->payoffs) << "\n";
				if (pverbose)
					cout
							<< "FinalState*********************************************************************************\n"
							<< proc->printState(current) << "\n";
				gameLogger.close();
				return proc->payoffs;
			}
		}
		// If we get to this point, it is clearly NOT the end of the game.  Players can assume that it is not the end of the game
		unsigned whoseTurn = current.getWhoseTurn();
		assert(whoseTurn < players.size());
		int chosenAction;
		static const bool showOptions = true;
		if (showOptions) {
			for (unsigned i = 0; i < canDo.size(); i++) {
				//cout << "Option: " << i << " " << proc->operatorIndexToString(canDo[i]) << "\n";
			}
		}
		if (players[whoseTurn] == P_RANDOM || canDo.size() == 1) { // Don't mess with any sampling if it's a forced move (i.e., canDo.size() == 1); just do it
			int rv = rand();
			chosenAction = canDo[rv % canDo.size()];
#ifdef PRANDOM
			cout << "RANDOM: " << rv << " " << " \% by " << canDo.size() << " = " << chosenAction << " GM\n";
#endif
#ifdef INTERACTIVE
			if (canDo.size() == 1) {
				int dummy;
				cout << proc->printPartialState(current);
				cout << "One legal move: " << proc->operatorIndexToString(chosenAction) << "\n";
				std::cin >> dummy;
			}
#endif
		} else if (players[whoseTurn] == P_HUMAN) {
			cout << proc->getFormattedState(gameHistory, current, proc->kb) << std::endl;
			VecInt legalActions = proc->legalOperators(current);
			for (unsigned i = 0; i < legalActions.size(); i++) {
				cout << i << " " << proc->operatorIndexToString(legalActions[i]) << "\n";
			}
			chosenAction = chooseHumanMove(proc, canDo, proc->initialWorld);
		} else {
			if (gmverbose) {
				cout << "WT: " << whoseTurn << "\n";
			}
			chosenAction = chooseMove(players[whoseTurn], proc, canDo, whoseTurn, proc->kb, proc->initialWorld,
					GameModerator::manySamples, GameModerator::fewSamples);
		}
		if (chosenAction == -1) { // Means that information set generation failed
			analyzeInfosetFailure(gameHistory, proc->initialWorld, current, proc, whoseTurn);
		}assert(find(canDo.begin(),canDo.end(),chosenAction) != canDo.end());
		if (pverbose) {
			for (unsigned i = 0; i < canDo.size(); i++) {
				cout << "Choice " << i << " " << proc->operatorIndexToString(canDo[i]) << "; ";
			}
			cout << "\n";
		}
		if (medverbose || showApplying)
			cout << "(" << moveNumber << ") Applying: " << chosenAction << " "
					<< proc->operatorIndexToString(chosenAction) << " chosen by " << whoseTurn << " who is "
					<< typeString(players[whoseTurn]) << " to state: \n";
		moveNumber++;
		//getchar();
		//if (pverbose) cout << proc->printPartialState(current) << "\n";
		proc->apply(chosenAction, current);
		proc->finalizeApply(current);
		gameHistory.push_back(chosenAction);
		gameLogger.append(chosenAction);
		ActionGraph::fluentHistory.push_back(current.getFluents());
		if (pverbose)
			cout << proc->printState(current) << "\n";
		//if (pverbose) cout << "After executing action " << i << "\n";
	}
	return proc->payoffs;
}
VecPayoff GameModerator::playManyGames(Processor* proc, const VecPlayerType& players, int n, int rank, int size, int & nGamesPlayed, GameLogger & logger) {
	static const int fixedSeed = 1234987123;
	//static const int fixedSeed = 20110510;
	VecPayoff result(proc->initialWorld.getNRoles(), 0);
	for (int i = rank; i < n; i += size) {
		int notRandom = fixedSeed + i * i + i * i * i + i;
		VecPayoff oneGameOutcomes = playGame(proc, players, notRandom, logger);
		cout << "Game " << i << " Rank " << rank << ": " << proc->asString(oneGameOutcomes) << "\n";
		for (unsigned j = 0; j < result.size(); j++) {
			result[j] += oneGameOutcomes[j];
		}
		nGamesPlayed++;
	}

	return result;
}

VecInt getLegals(const SetVecInt & infoset, WorldState current, Processor *p) {
	assert(!infoset.empty());
	if (infoset.empty()) {
		std::cerr << "Info set empty\n";
		exit(-1);
	}
	p->apply(*infoset.begin(), current);
	return p->legalOperators(current);
}

int GameModerator::chooseMove(PlayerType pt, Processor *p, VecInt & legalOptions, unsigned pid, VecVecVecKey & obs,
		WorldState & initState, int nSamples, int oppSamples) {
	InfoSetGenerator isg(p, obs, initState);
	if (gmverbose)
		cout << "CHOOSE MOVE BEGIN INFOSET GENERATION\n";

	unsigned requestSize = (pt == P_INFER) ? augmentedRequest : maxSize;
	SetVecInt infoset = isg.generate(pid, requestSize);
	if (gmverbose)
		cout << "CHOOSE MOVE END INFOSET GENERATION\n";

	if (infoset.empty()) {
		cout << "Empty infoset in choosemove\n";
		return -1;
	}
	//VecInt legalOptions = getLegals(infoset, initState, p);
	//cout << proc->printPartialState(current) << "\n";
	int result;
	switch (pt) {
	//case P_HUMAN:
	case P_INFER:
		result = chooseInferenceMove(p, pid, obs, infoset, legalOptions, initState, nSamples, oppSamples);
		if (result == -2) {
			// None of the infoset nodes were consistent
			cout << "Zero prob est for all infoset nodes -- using MCTS instead\n";
			for (SetVecInt::const_iterator it = infoset.begin(); it != infoset.end(); ++it) {
				cout << InfoSetGenerator::moveSequenceString(*it) << "\n\n";
			}
			infoset = InfoSetGenerator::randomPruneDownTo(infoset, maxSize);
			result = chooseMoveMCTS(p, obs, infoset, legalOptions, initState, nSamples);
		}

		break;
	case P_MCTS_LIMITED:
		result = chooseMoveMCTS(p, obs, infoset, legalOptions, initState, oppSamples);
		break;
	case P_MCTS:
		result = chooseMoveMCTS(p, obs, infoset, legalOptions, initState, nSamples);
		break;
	case P_HEURISTIC:
		result = chooseMoveHeuristic(p, obs, infoset, legalOptions, initState, nSamples);
		break;
	default:
		//return chooseMCTSMove(p, pid, obs, infoset, initState);
		result = chooseMoveMCTS(p, obs, infoset, legalOptions, initState, nSamples);
		//int GameModerator::chooseMoveMCTS(Processor* p, const VecVecVecKey& obs, const SetVecInt& infoset, const VecInt& legalOptions, const WorldState& initState)
		break;
	}

	// Don't make it this far
	//cout << "Move selected: " << p->operatorIndexToString(result) << " by " << typeString(pt) << "\n";
	return result;
}

int argMax(VecFloat & vf) {
	if (gmverbose)
		cout << "ArgMax: " << vf.size() << "\n";

	assert(!vf.empty());
	int result = 0;
	float vmax = vf[0];
	for (unsigned i = 1; i < vf.size(); i++) {
		if (vf[i] > vmax) {
			result = i;
			vmax = vf[i];
		}
	}

	return result;
}

int GameModerator::chooseHumanMove(Processor *p, const VecInt & legalActions, WorldState & initState) {
	//assert (!infoset.empty());
	//WorldState current = initState;
	//proc->apply(*infoset.begin(),current);
	unsigned dummy;
	std::cin >> dummy;
	assert(dummy >= 0);
	assert(dummy < legalActions.size());
	return legalActions[dummy];
}

VecFloat GameModerator::mctsExplore(const VecInt & seq, const WorldState & initState, Processor *p, int nSamples) {
	if (gmverbose)
		cout << "MCTS EXPLORE: " << nSamples << "\n";

	assert(nSamples > 0);
	if (nSamples < 0)
		nSamples = (int) (Node::nSamples);

	Node & root = Node::nodeVec()[0];
	// For each node (specified by a sequence of moves from root)
	WorldState current = initState;
	p->apply(seq, current);
	VecInt legalActions = p->legalOperators(current);
	unsigned nOptions = legalActions.size();
	VecFloat result(nOptions);
	if (gmverbose)
		cout << "Running MCTS on active player: " << current.getWhoseTurn() << "\n";

	root.initializeRoot(current);
	if (root.terminal) { // \TODO: Maybe just assert !root.terminal
		if (gmverbose)
			cout << "INITIAL STATE\n";
		if (gmverbose)
			cout << p->printPartialState(initState) << "\n";
		if (gmverbose)
			cout << "Root zeroed on initialization\n";
		if (gmverbose)
			cout << p->printPartialState(Node::nodeVec()[0].ws) << "\n";
		assert(false);
		// Should have returned before here;
		// By definition of info sets, it cannot be ambiguous whether the game is over or not.
		// It is therefore not possible that some nodes in an information set are end of game
		// and some are not.
	}
	for (int s = 0; s < nSamples; s++) {
		if (gmverbose || true)
			cout << "Sample #" << s << "\n";
		// Should be able to delete this. If it wasn't terminal before, it shouldn't be now
		if (root.terminal) {
			//cout << "Root zeroed on " << s << "\n";
			assert(false);
			// Should have returned before here.
		}
		Node::nodeVec()[0].select();
	}
	//int choice = Node::nodeVec()[0].mctsChoice();
	assert(root.children.size() == nOptions);
	for (unsigned a = 0; a < nOptions; a++) {
		result[a] = root.children[a].valueEst;
	}
	return result;
}

VecVecFloat GameModerator::getDecisionMatrix(const SetVecInt & infoset, const WorldState & initState, Processor *p,
		int nSamples) {
	VecVecFloat result;
	for (SetVecInt::const_iterator itr = infoset.begin(); itr != infoset.end(); ++itr) {
		result.push_back(mctsExplore(*itr, initState, p, nSamples));
	}
	return result;
}

int GameModerator::getDecisionFromMatrix(Processor* proc, const VecInt & legalOptions, const VecVecFloat & vvf, VecFloat & sums) {
	static const bool decverbose = true || gmverbose;
	//VecFloat sums(legalOptions.size(),0.0f);
	for (unsigned a = 0; a < legalOptions.size(); a++) {
		if (decverbose) {
			cout << proc->operatorIndexToString(legalOptions[a]) << "; ";
		}
	}
	if (decverbose) {
		cout << "\n";
	}
	for (unsigned i = 0; i < vvf.size(); i++) {
		for (unsigned a = 0; a < legalOptions.size(); a++) {
			if (decverbose) {
				printf("%1.3f ", vvf[i][a]);
			}
			sums[a] += vvf[i][a];
		}
		if (decverbose) {
			printf("\n");
		}
	}
	if (decverbose) {
		printf("\n");
		for (unsigned a = 0; a < legalOptions.size(); a++) {
			printf("K%1.3f ", sums[a]);
		}
		printf("\n");
	}

	unsigned bestActionIndex = argMax(sums);
	return legalOptions[bestActionIndex];
}

int GameModerator::chooseMoveMCTS(Processor *p, const VecVecVecKey & obs, const SetVecInt & infoset,
		const VecInt & legalOptions, const WorldState & initState, int nSamples) {
	static const bool decverbose = false || gmverbose;
	VecVecFloat decisionMat = getDecisionMatrix(infoset, initState, p, nSamples);
	VecFloat sums(legalOptions.size(), 0.0f);
	int bestAction = getDecisionFromMatrix(p, legalOptions, decisionMat, sums);
	if (decverbose)
		cout << "Optimal decision: " << bestAction << " (" << p->operatorIndexToString(bestAction) << ")\n";

	//exit(0);
	//assert(false);
	return bestAction;
}
int lastOpponentMove(const VecInt & sequence, unsigned pid, Processor *p, WorldState current) {
	static const bool verbose = false;
	unsigned result = -1;
	NewConditionController ncc(p, current);
	if (verbose)
		cout << "PID: " << pid << "\n";

	for (unsigned t = 1; t < sequence.size(); t++) {
		unsigned wt = current.getWhoseTurn();
		if (verbose)
			cout << "Action at time " << t << " is made by player " << wt << " ("
					<< p->operatorIndexToString(sequence[t]) << ")\n";

		if (wt != pid && wt != 0) {
			result = t;
		}
		p->shortApply(sequence[t], current, ncc);
	}

	if (verbose)
		cout << "Last opponent move: " << result << "\n";

	return result;
}
int GameModerator::chooseInferenceMove(Processor *p, unsigned pid, VecVecVecKey & obs, SetVecInt & infoset,
		const VecInt & legalOptions, WorldState & initState, int nSamples, int oppSamples) {
	static bool infverbose = false;
	VecFloat probs(infoset.size(), 0.0); // Initially, the unnormalized prob for each node in the infoset.  probs[i] = 0 if we estimate that the previous player would not have allowed play to reach that point
	VecVecFloat decArray;
	// store a backup of the obs from p
	int i = 0;
	int nConsistents = 0;
	for (SetVecInt::const_iterator itr = infoset.begin(); itr != infoset.end(); ++i, ++itr) { // For each node in the info set
		VecInt oppPrefix = *itr;
		assert(!oppPrefix.empty());
		int lastOppMove = lastOpponentMove(oppPrefix, pid, p, initState);
		if (oppPrefix.size() == 1 || lastOppMove < 0) { // this is the root node, so we can't run our inference algorithm
			return chooseMoveMCTS(p, obs, infoset, legalOptions, initState, nSamples);
		}
		//int lastDecision = oppPrefix[oppPrefix.size()-1];
		//oppPrefix.resize(oppPrefix.size()-1); // Kludge for now -- assumes that opponent was the last player to go, needs to be updated for when there are chance moves between
		int lastDecision = oppPrefix[lastOppMove];
		oppPrefix.resize(lastOppMove);
		WorldState current = initState;
		VecVecVecKey oppObs = p->getObs(oppPrefix, current);
		//cout << "CURRENT:\n" << p->printState(current) << "\n";
		unsigned ppid = current.getWhoseTurn();
		assert(ppid != 0);
		if (infverbose)
			cout << "GENERATING OPPONENT'S INFOSET\n";
		InfoSetGenerator isg(p, oppObs, initState);
		SetVecInt oppInfoset = isg.generate(ppid, maxSize);
		if (oppInfoset.empty()) {
			std::cerr << "Failed to generate opponent's infoset" << std::endl;
			return -1;
		}
		//VecVecFloat decisionMat = getDecisionMatrix(oppInfoset, initState, p);
		VecInt oppLegalActions = getLegals(oppInfoset, initState, p);
		//int bestOppAction = chooseMoveMCTS(p, oppObs, oppInfoset, oppLegalActions, initState, oppSamples);
// Figure out what the other guy's decision would have been
		VecVecFloat decisionMat = getDecisionMatrix(oppInfoset, initState, p, oppSamples);
		VecFloat sums(oppLegalActions.size(), 0.0f);
		int bestOppAction = getDecisionFromMatrix(p, oppLegalActions, decisionMat, sums);
		if (infverbose) {
			for (unsigned s = 0; s < sums.size(); s++) {
				printf("S%1.3f ", sums[s]);
			}
			printf("\n");
		}
		//if (infverbose) cout << "Optimal decision: " << bestOppAction << " (" << proc->operatorIndexToString(bestOppAction) << ")\n";
// End
		if (infverbose)
			cout << "From node " << i << " bestMCTS option is " << bestOppAction << " "
					<< p->operatorIndexToString(bestOppAction) << "\n";
		if (bestOppAction == lastDecision) {
			if (infverbose)
				cout << "This decision is consistent with this information set node\n";
			probs[i] = 1.0;
			//VecFloat GameModerator::mctsExplore(const VecInt& seq, const WorldState& initState, Processor* p, int nSamples)
			decArray.push_back(mctsExplore(*itr, initState, p, nSamples));
			nConsistents++;
		} else {
			// See if one of the options is "close enough"
			int bestIndex = -1;
			bool foundBest = false;
			int matchingIndex = -1;
			bool foundMatching = false;
			for (unsigned k = 0; k < oppLegalActions.size(); k++) {
				if (oppLegalActions[k] == lastDecision) {
					foundMatching = true;
					matchingIndex = k;
				}
				if (oppLegalActions[k] == bestOppAction) {
					foundBest = true;
					bestIndex = k;
				}
			}
			if (!foundBest) {
				cout << "Failure to find something that was supposedly there.\n";
				exit(1);
			}
			if (foundMatching && (sums[bestIndex] < .03 || (sums[matchingIndex] / sums[bestIndex]) > .9)) {
				if (gmverbose)
					cout << "This decision is close enough to the one observed to consider this information set node\n";
				if (gmverbose)
					cout << "sbi: " << sums[bestIndex] << " " << sums[matchingIndex] << "/" << sums[bestIndex] << " = "
							<< sums[matchingIndex] / sums[bestIndex] << "\n";
				if (gmverbose)
					cout << "best index " << bestIndex << " matching index: " << matchingIndex << "\n";
				probs[i] = 1.0;
				//VecFloat GameModerator::mctsExplore(const VecInt& seq, const WorldState& initState, Processor* p, int nSamples)
				decArray.push_back(mctsExplore(*itr, initState, p, nSamples));
				nConsistents++;
			} else {
				if (gmverbose)
					cout << "not consistent\n";
				probs[i] = 0.0;
				decArray.push_back(VecFloat(legalOptions.size(), 0.0f));
			}
		}
		if (nConsistents >= (int) maxSize)
			break;
		// If it's the first one, find the last move that is not a move made by pid or chance
		// Let the sequence seq' have length n, and the let the player who played last be ppid
		// Get the list of observations for player ppid when seq' is executed
		// Generate the information set for ppid from that point
		// Call chooseMCTSMove on that info set
		// probs[i] = computeExtensionProbability(results from chooseMCTSMove, seq', *itr)
		// Call MCTS from *itr and return the VecFloats that gives the cumulative rewards for each of the legal moves
	}
	if (gmverbose)
		cout << "INFOSET nodes that are CONSISTENT: " << nConsistents << "/" << infoset.size() << "\n";

	if (nConsistents == 0)
		return -2;

	VecFloat weightedEsts(legalOptions.size(), 0.0f);
	for (unsigned j = 0; j < decArray.size(); j++) {
		assert(decArray[j].size() == legalOptions.size());
		if (medverbose)
			printf("%2.3f: ", probs[j]);
		for (unsigned i = 0; i < legalOptions.size(); i++) {
			if (probs[j] > 0.0f) {
				weightedEsts[i] += probs[j] * decArray[j][i];
				//cout << "Weighting: " << i << " " << j << " (" << p->operatorIndexToString(legalOptions[i]) << ") decArray: " << decArray[j][i] << " prob: " << probs[j] << "\n";
			}
			if (medverbose)
				printf("%2.3f ", decArray[j][i]);
		}
		if (medverbose)
			printf("\n");
	}
	if (medverbose) {
		printf("sums: ");
		for (unsigned i = 0; i < legalOptions.size(); i++) {
			printf("%2.3f ", weightedEsts[i]);
		}
	}

	//for (unsigned i = 0; i < legalOptions.size(); i++) {
	//  cout << "i " << i << "\n";
	//  for (unsigned j = 0; j < decArray.size(); j++) {
	//    assert (decArray[j].size() == legalOptions.size());
	//    if (probs[j] > 0.0f) {
	//      weightedEsts[i] += probs[j]*decArray[j][i];
	//      cout << "Weighting: " << i << " " << j << " (" << p->operatorIndexToString(legalOptions[i]) << ") decArray: " << decArray[j][i] << " prob: " << probs[j] << "\n";
	//    }
	//  }
	//}
	unsigned bestIndex = argMax(weightedEsts);
	return legalOptions[bestIndex];
	// for each move m
	//   val[m] = 0
	//   for each infoset node n
	//     val[m] += prob[n]*estVals[n][m]
	// return argmax(val)
	//return 0;
}

//int GameModerator::chooseMCTSMove(Processor *p, unsigned pid, VecVecVecKey & obs, SetVecInt & infoSet,
//		WorldState & initState) {
//	assert(!infoSet.empty());
//	unsigned nOptions = -1;
//	bool firstOne = true;
//	VecVecFloat actionValueEsts(infoSet.size());
//	unsigned i = 0;
//	VecInt legalActions;
//	Node & root = Node::nodeVec()[0];
//	// For each node (specified by a sequence of moves from root)
//	for (SetVecInt::const_iterator itr = infoSet.begin(); itr != infoSet.end(); ++i, ++itr) {
//		const VecInt& sequence = *itr;
//		WorldState current = initState;
//		proc->apply(sequence, current);
//		assert(current.getWhoseTurn() == (int)pid);
//		legalActions = proc->legalOperators(current);
//
//		if (firstOne) {
//			firstOne = false;
//			nOptions = legalActions.size();
//		} else {
//			assert(legalActions.size() == nOptions);
//		}
//		actionValueEsts[i].resize(nOptions);
//		if (gmverbose)
//			cout << "Running MCTS on active player: " << current.getWhoseTurn() << "\n";
//		//proc->computePayoffs(current);
//		//cout << "BEFORE: " << proc->asString(proc->payoffs,PAYOFFS) << "\n";
//		root.initializeRoot(current);
//		if (root.terminal) {
//			if (gmverbose)
//				cout << "INITIAL STATE\n";
//			if (gmverbose)
//				cout << proc->printPartialState(initState) << "\n";
//			if (gmverbose)
//				cout << "Root zeroed on initialization\n";
//			if (gmverbose)
//				cout << proc->printPartialState(Node::nodeVec()[0].ws) << "\n";
//			assert(false);
//			// Should have returned before here;
//			// By definition of info sets, it cannot be ambiguous whether the game is over or not.
//			// It is therefore not possible that some nodes in an information set are end of game
//			// and some are not.
//		}
//		for (unsigned s = 0; s < Node::nSamples; s++) {
//			if (gmverbose || false)
//				cout << "Sample #" << s << "\n";
//			// Should be able to delete this. If it wasn't terminal before, it shouldn't be now
//			if (root.terminal) {
//				//cout << "Root zeroed on " << s << "\n";
//				assert(false);
//				// Should have returned before here.
//			}
//			Node::nodeVec()[0].select();
//		}
//		//int choice = Node::nodeVec()[0].mctsChoice();
//		assert(root.children.size() == nOptions);
//		for (unsigned a = 0; a < nOptions; a++) {
//			actionValueEsts[i][a] = root.children[a].valueEst;
//		}
//	}
//	// Estimated values;
//	static const bool decverbose = false || gmverbose;
//	VecFloat sums(nOptions, 0.0f);
//	for (unsigned a = 0; a < nOptions; a++) {
//		if (decverbose)
//			cout << proc->operatorIndexToString(root.children[a].actionId) << "; ";
//
//	}
//	if (decverbose)
//		cout << "\n";
//
//	for (unsigned i = 0; i < actionValueEsts.size(); i++) {
//		for (unsigned a = 0; a < nOptions; a++) {
//			if (decverbose)
//				printf("%1.3f ", actionValueEsts[i][a]);
//
//			sums[a] += actionValueEsts[i][a];
//		}
//		if (decverbose)
//			printf("\n");
//
//	}
//	unsigned bestActionIndex = argMax(sums);
//	int bestAction = legalActions[bestActionIndex];
//	if (decverbose)
//		cout << "Optimal decision: " << bestAction << " (" << proc->operatorIndexToString(bestAction) << ")\n";
//
//	//exit(0);
//	//assert(false);
//	return bestAction;
//}

void GameModerator::analyzeInfosetFailure(const VecInt & gameHistory, WorldState initState, WorldState current,
		Processor *p, unsigned k) {
	cout << "ANALYZE INFOSET FAILURE\n";
	cout << p->printKnowledgeState(k) << std::endl;
	ActionGraph ag(p, p->kb, p->opsByRev, initState, k);
	ag.initializeGraph();
	cout << "Current state: \n" << p->printPartialState(current);
	WorldState newCurrent = initState;
	p->apply(gameHistory, newCurrent);
	cout << "Start state + gameHistory: \n" << p->printPartialState(newCurrent);
	for (unsigned t = 1; t < gameHistory.size(); t++) {
		cout << "Move " << t << " " << p->operatorIndexToString(gameHistory[t]) << "\n";
	}
	cout << "VERBOSELY\n";
	cout << ag.asString() << "\n";
	cout << "BRIEFLY\n";
	cout << ag.asString(true) << "\n";
	exit(0);
}

int GameModerator::chooseMoveHeuristic(Processor *p, const VecVecVecKey & obs, const SetVecInt & infoset,
		const VecInt & legalOptions, const WorldState & initState, int nSamples) {
	VecVecFloat decisionMatrix(infoset.size());
	int i = 0;
	for (SetVecInt::const_iterator itr = infoset.begin(); itr != infoset.end(); ++itr, ++i) {
		decisionMatrix[i].resize(legalOptions.size());
		VecInt seq = *itr;
		int nextMoveIndex = seq.size();
		seq.push_back(-1);
		for (unsigned m = 0; m < legalOptions.size(); m++) {
			seq[nextMoveIndex] = legalOptions[m];
			WorldState current = initState;
			p->apply(seq, current);
			decisionMatrix[i][m] = p->getEstimatedValue(seq, current, obs);
		}
	}
	VecFloat sums(legalOptions.size(), 0.0f);
	int bestAction = getDecisionFromMatrix(p, legalOptions, decisionMatrix, sums);
	return bestAction;
}

}
