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
	static VecPayoff playGame(Processor* p, const VecPlayerType& players, int seed = 0);
	static VecPayoff playGame(Processor* p, const VecPlayerType& players, int seed, GameLogger& gameLogger);
	static VecPayoff playManyGames(Processor* proc, const VecPlayerType& players, int n, int rank, int size, int& nGamesPlayed, GameLogger& logger);
	static int chooseMove(PlayerType pt, Processor* p, VecInt& canDo, unsigned pid, VecVecVecKey& obs, WorldState& initState,
			int nSamples, int oppSamples);
	static int chooseMCTSMove(Processor* p, unsigned pid, VecVecVecKey& obs, SetVecInt& infoSet, WorldState& initState);
	static int chooseInferenceMove(Processor* p, unsigned pid, VecVecVecKey& obs, SetVecInt& infoset,
			const VecInt& legalOptions, WorldState& initState, int nSamples, int oppSamples);
	static int chooseHumanMove(Processor* p, const VecInt& legalActions, WorldState& initState);
	static void analyzeInfosetFailure(const VecInt& gameHistory, WorldState initState, WorldState current, Processor* p,
			unsigned k);

	static VecFloat mctsExplore(const VecInt& seq, const WorldState& initState, Processor* p, int nSamples = -1); // use Node::nSamples by default
	static VecVecFloat getDecisionMatrix(const SetVecInt& infoset, const WorldState& initState, Processor* p, int nSamples);
	static int getDecisionFromMatrix(Processor* proc, const VecInt& legalOptions, const VecVecFloat& vvf, VecFloat& sums);
	static int chooseMoveMCTS(Processor* p, const VecVecVecKey& obs, const SetVecInt& infoset, const VecInt& legalOptions,
			const WorldState& initState, int nSamples);
	static int chooseMoveHeuristic(Processor* p, const VecVecVecKey& obs, const SetVecInt& infoset, const VecInt& legalOptions,
			const WorldState& initState, int nSamples);
};

}

#endif
