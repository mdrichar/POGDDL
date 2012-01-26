#ifndef GAME_MODERATOR_H
#define GAME_MODERATOR_H

#include "processor.h"
#include "WorldState.h"
#include "GameLogGenerator.h"
#include <vector>
#include <map>

namespace VAL {
class GameModerator {
public:
	static int manySamples;
	static int fewSamples;
	static unsigned maxSize;
	static StringToPlayerType getPlayerTypes();
	static string typeString(PlayerType pt);
	Processor* proc;
	WorldState initState;
	VecPlayerType players;

	GameModerator(Processor* p, WorldState& initState_, VecPlayerType& playerIds);
	VecPayoff playGame(int seed = 0);
	VecPayoff playGame(int seed, GameLogger& gameLogger);
	VecPayoff playManyGames(int n, int rank, int size, int& nGamesPlayed, GameLogger& logger);
	int chooseMove(PlayerType pt, Processor* p, VecInt& canDo, unsigned pid, VecVecVecKey& obs, WorldState& initState,
			int nSamples, int oppSamples);
	int chooseMCTSMove(Processor* p, unsigned pid, VecVecVecKey& obs, SetVecInt& infoSet, WorldState& initState);
	int chooseInferenceMove(Processor* p, unsigned pid, VecVecVecKey& obs, SetVecInt& infoset,
			const VecInt& legalOptions, WorldState& initState, int nSamples, int oppSamples);
	int chooseHumanMove(Processor* p, const VecInt& legalActions, WorldState& initState);
	void analyzeInfosetFailure(const VecInt& gameHistory, WorldState initState, WorldState current, Processor* p,
			unsigned k);

	VecFloat mctsExplore(const VecInt& seq, const WorldState& initState, Processor* p, int nSamples = -1); // use Node::nSamples by default
	VecVecFloat getDecisionMatrix(const SetVecInt& infoset, const WorldState& initState, Processor* p, int nSamples);
	int getDecisionFromMatrix(const VecInt& legalOptions, const VecVecFloat& vvf, VecFloat& sums);
	int chooseMoveMCTS(Processor* p, const VecVecVecKey& obs, const SetVecInt& infoset, const VecInt& legalOptions,
			const WorldState& initState, int nSamples);
	int chooseMoveHeuristic(Processor* p, const VecVecVecKey& obs, const SetVecInt& infoset, const VecInt& legalOptions,
			const WorldState& initState, int nSamples);
};

}

#endif
