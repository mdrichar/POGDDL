#include "InfoSetGenerator.h"
#include "NewConditionController.h"
#include "PerformanceCounters.h"
#include "Node.h"
#include <stack>
#include <iterator>
#include <sstream>

using std::cout;
using std::endl;

int printer = 3;
namespace VAL {

Processor* InfoSetGenerator::gproc;

InfoSetGenerator::InfoSetGenerator(Processor* proc_, const VecVecVecKey& obs_, const WorldState& initialWorld_) :
		proc(proc_), obs(obs_), initialWorld(initialWorld_) {
	assert(proc);
	InfoSetGenerator::gproc = proc;
}

SetVecInt InfoSetGenerator::randomPruneDownTo(const SetVecInt& sequences, unsigned maxSize) {
	//cout << "Randomly pruning\n"  ;
	//assert (sequences.size() > maxSize); // Otherwise, it should not have been called
	if (sequences.size() <= maxSize) {
		return sequences;
	}
	VecInt indices(sequences.size());
	for (unsigned i = 0; i < indices.size(); i++)
		indices[i] = i;
	random_shuffle(indices.begin(), indices.end());
	SetVecInt result;
	for (unsigned ptr = 0; ptr < maxSize; ptr++) {
		SetVecInt::const_iterator itr = sequences.begin();
		for (int setpt = 0; setpt < indices[ptr]; ++itr, ++setpt) {

		}assert(itr != sequences.end());
		result.insert(*itr);
	}assert(result.size() == maxSize);
	return result;
}

SetVecInt InfoSetGenerator::generateN(unsigned k, unsigned maxSize) {
	static int timesCalled = 0;
	cout << "GenerateN times called: " << ++timesCalled << endl;
	cout << proc->printKnowledgeState(k) << endl;
	SetVecInt result;
	this->k = k;
	VecInt path(obs.size() + 1, 0);

	static const double timeLimit = 10.0;
	double startTime = get_clock();
	generateNaive(path, initialWorld, 1, result, startTime, timeLimit, maxSize);
	return result;
}

SetVecInt InfoSetGenerator::generate(unsigned k, unsigned maxSize) {
	//static int timesCalled = 0;
	//cout << "Generate times called: " << ++timesCalled << endl;
	//cout << proc->printKnowledgeState(k) << endl;
	static const double timeLimit = 30.0;
	static const unsigned sequentialThreshold = 2000;
	static const bool verbose = false;
	SetVecInt result;
	this->k = k;
	actionStack.push(ActionGraph(proc, obs, proc->opsByRev, initialWorld, k));
	ActionGraph& ag = actionStack.top();
	ag.initializeGraph();
	string initialAgStringVerb = ag.asString();
	string initialAgStringBrief = ag.asString(true);
	bool overflow = true;
	unsigned totalCombinations = ag.totalPossibilities(overflow, sequentialThreshold);
	//assert (totalCombinations > 0);
	if (verbose)
		cout << "Total combinations: " << totalCombinations << "\n";
	if (verbose)
		cout << "ACTION GRAPH\n";
	if (verbose)
		cout << initialAgStringBrief << "\n";
	if (totalCombinations == 1 && !overflow) {
		if (verbose)
			cout << "ONE OPTION\n";
		processSolution(ag, result);
	} else if (totalCombinations < sequentialThreshold && !overflow) { // If the total number of possible combinations is low, do an exhaustive search
		if (verbose)
			cout << "RECURSIVELY GENERATE" << totalCombinations << std::endl;
		//cout << "XXX Before Generate Rec\n";
		generateRec(result);
		//cout << "XXX After Generate Rec\n";
		//getchar();
	} else {
		double searchStartTime = get_clock();
		unsigned attempt = 0;
		printer = 3;
		//generateRec(); // need to recursively generate the list
		while (result.size() < maxSize) {
			double elapsedTime = get_clock() - searchStartTime;
			if (elapsedTime > timeLimit) {
				cout << "Timed out after " << elapsedTime << " with " << result.size() << " nodes " << endl;
				break;
			}
			generateRandom(result);
			//cout << "XXX END generate Random " << attempt << "\n";
			attempt++;
		}
		if (result.empty()) {
			std::cerr << "Failed to generate valid sequence in " << attempt << " tries\n";
			cout << "OPPVERBOSELY\n";
			cout << initialAgStringVerb << "\n";
			cout << "OPPBRIEFLY\n";
			cout << initialAgStringBrief << "\n";
			return result;
			//exit(-1);
			//for (unsigned m = 0; m < 100*maxSize; m++) {
			//  generateRandom(result);
			//    cout << "XXX END bonus generate Random " << m << "\n";
			//}
		}
		//assert (!result.empty());
	}
	if (result.size() > maxSize) {
		//assert (result.size() < 100);
		result = randomPruneDownTo(result, maxSize); // Randomly select a subset of maxSize members from result
	}
	return result;
}

void InfoSetGenerator::displayPath(const VecInt& path) {
	cout << "ISnode: ";
	//copy(path.begin(),path.end(),std::ostream_iterator<int>(cout, " "));
	for (unsigned i = 1; i < path.size(); i++) {
		cout << proc->operatorIndexToString(path[i]) << "; ";
	}
	cout << "\n";

}

void InfoSetGenerator::processSolution(const ActionGraph& ag, SetVecInt& result) {
	static const bool isgverbose = true;
	VecInt path = ag.extractPath();
	if (verify(path, this->obs, this->k, this->initialWorld)) {
		++PerformanceCounters::solutionCount;
		result.insert(path);
		if (isgverbose) {
			cout << "CBS";
			displayPath(path);
		}
	} else {
		//cout << "Not verified" << std::endl;
	}
}

// To be a legitimate member of the information set, the sequence of actions must be executable
// (action i can be executed from the state that results from applying action i-1) and the observations
// must match
bool InfoSetGenerator::verify(const VecInt& path, VecVecVecKey& obs, unsigned k, const WorldState& initialWorld) {
	WorldState currentWorld = initialWorld;
	for (unsigned t = 1; t < path.size(); t++) {
		assert(t-1 < obs.size());
		NewConditionController ncc(proc, currentWorld);
		int index = path[t];
		operator_* op = proc->getOpFromIndex(index);
		ncc.args = VecInt(op->getArgSizeNeeded(), -1);
		string dummy;
		proc->decodeOperator(index, dummy, ncc.args);
		op->precondition->visit(&ncc);
		if (ncc.truthValue != KNOWN_TRUE) {
			//cout << "Not verified because action " << t << "(" << proc->operatorIndexToString(index) << ") is not known to be executable in " << std::endl;
			//cout << proc->printState(currentWorld) << "\n";
			return false;
		}
		currentWorld.clearUpdates();
		proc->apply(index, currentWorld);
		if (currentWorld.obsListByPlyr[k] != obs[t - 1][k]) {
			cout << "Trying to match observation:\n";
			for (unsigned o = 0; o < obs[t - 1][k].size(); o++) {
				cout << asIntString(obs[t - 1][k][o]) << "; ";
			}
			cout << "\n";
			cout << "with:\n";
			for (unsigned o = 0; o < currentWorld.obsListByPlyr[k].size(); o++) {
				cout << asIntString(currentWorld.obsListByPlyr[k][o]) << "; ";
			}
			cout << "\n";
			return false;
		}
		currentWorld.applyUpdates();
		if (proc->isTerminal(currentWorld)) {
			//cout << "Not verified because isTerminal(currentWorld)" << std::endl;
			//if (printer > 0) {
			//  cout << proc->printState(currentWorld) << "\n";
			//  printer--;
			//}
			return false; // This can happen if, say, a node that would otherwise have been in the infoset would actually have resulted
			// in the end of the game.  For example, if we hypothesize a configuration of the opponent's rack in Racko such that the opponent
			// would have won the game, then that state is clearly not in the information set.  If the game had ended, the moderator would not
			// have allowed us to get to this point.
		}
	}
	//cout << "Verified\n";
	return true;
}

void InfoSetGenerator::generateNaive(VecInt& sequence, WorldState& currentWorld, unsigned t, SetVecInt& results,
		double startTime, double timeLimit, unsigned maxSize) {
	if (results.size() >= maxSize)
		return;
	if (t >= sequence.size()) {
		//displayPath(sequence);
		results.insert(sequence);
		++PerformanceCounters::solutionCount;
	} else {
		++PerformanceCounters::generateNaive;
		VecInt legalActions = proc->legalOperators(currentWorld);
		for (unsigned i = 0; i < legalActions.size(); i++) {
			WorldState nextWorld = currentWorld;
			VecVecKey currObs = proc->fullApply(legalActions[i], nextWorld);
			if (currObs[k] == obs[t - 1][this->k]) {
				sequence[t] = legalActions[i];
				double elapsedTime = get_clock() - startTime;
				if (elapsedTime < timeLimit) {
					generateNaive(sequence, nextWorld, t + 1, results, startTime, timeLimit, maxSize);
				}
			}
		}
	}
}

void InfoSetGenerator::generateRec(SetVecInt& result) {
	ActionGraph& ag = actionStack.top();
	//cout << "Called generateRec at depth " << actionStack.size() << "\n" << ag.asString(true) << "\n";
	assert(!ag.allActionSetsAreSingletons());
	// Otherwise, this method should not have been called
	unsigned var = ag.mostConstrained();
	SetInt& actions = ag.getActions(var);
	assert(actions.size() > 1);
	int i = 0;
	for (SetInt::const_iterator itr = actions.begin(); itr != actions.end(); ++i, ++itr) {
		//cout << proc->operatorIndexToString(*itr) << " is option " << i << "/" << actions.size() << " at level " << actionStack.size() << std::endl;
		//cout << "What we know: \n" << ag.abbreviatedStagesAsStrings() << endl;
		actionStack.push(actionStack.top()); // Make a new copy of the ag on the top of the stack
		ActionGraph& newtop = actionStack.top();
		newtop.stages[var].possActions.clear();
		newtop.stages[var].possActions.insert(*itr);
		if (newtop.reevaluatelegalActions()) {
			if (newtop.allActionSetsAreSingletons()) {
				processSolution(newtop, result);
			} else {
				generateRec(result); // Recurse to fill in more actions
			}
		}
		actionStack.pop();
	}
}

void InfoSetGenerator::generateRandom(SetVecInt& result) {
	ActionGraph& ag = actionStack.top();
	assert(!ag.allActionSetsAreSingletons());
	// Otherwise, this method should not have been called
	unsigned var = ag.mostConstrained();
	SetInt& actions = ag.getActions(var);
	int rv = rand();
	int randomChoice = rv % actions.size();
#ifdef PRANDOM
	cout << "RANDOM: " << rv << " " << " \% by " << actions.size() << " = " << randomChoice << " ISG\n";
#endif
	SetInt::const_iterator itr = actions.begin();
	for (int i = 0; i < randomChoice; ++i, ++itr) {

	}assert(itr != actions.end());
	int chosenAction = *itr;
	assert(actions.size() > 1);
	//cout << "XXX GenerateRandom at depth " << actionStack.size() << " trying move " << var << " = " << proc->operatorIndexToString(chosenAction) << "\n";
	actionStack.push(actionStack.top()); // Make a new copy of the ag on the top of the stack
	ActionGraph& newtop = actionStack.top();
	newtop.stages[var].possActions.clear();
	newtop.stages[var].possActions.insert(chosenAction);
	if (newtop.reevaluatelegalActions()) {
		if (newtop.allActionSetsAreSingletons()) {
			//cout << "XXX Checking solution\n ";
			processSolution(newtop, result);
		} else {
			generateRandom(result); // Recurse to fill in more actions
		}
	}
	actionStack.pop();
}

string InfoSetGenerator::moveSequenceString(const VecInt& sequence) {
	std::ostringstream os;
	for (unsigned i = 1; i < sequence.size(); i++) {
		os << "(" << i << ") " << gproc->operatorIndexToString(sequence[i]) << "; ";
	}
	return os.str();
}

}
