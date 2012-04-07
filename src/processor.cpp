#include <sstream>
#include <iterator>
#include <climits>
#include <algorithm>
#include <numeric>

#include "processor.h"
#include "InitialStateVisitController.h"
#include "InfoSetGenerator.h"
#include "ConditionController.h"
#include "OperatorController.h"
#include "NewConditionController.h"
#include "ActionGraph.h"
#include "WorldStateFormatter.h"
#include "StaticEvaluationFunction.h"
#include "PerformanceCounters.h"
using namespace VAL;

using std::cout;
using std::endl;
using std::ostringstream;

double Processor::clockTotal = 0.0;
bool Processor::superVerbose = false;
bool Processor::checkPayoffs = true;
namespace VAL {

// Return the index of the fact of the predName propType, where the ith argument is args[inidcies[i]]
// for variables and -1-indices[i] for constants
int Processor::factNumLookupByArgIndex(const string& predName, const VecInt& indices, const VecInt& args) {
	static VecInt intArgs;
	assert(hasPred(predName));
	unsigned nArgs = predTbl[predName].size();
	if (intArgs.size() < nArgs)
		intArgs.resize(nArgs);
	for (unsigned i = 0; i < nArgs; i++) {
		if (indices[i] >= 0) {
			assert(indices[i] < (int)args.size());
			intArgs[i] = args[indices[i]];
		} else {
			intArgs[i] = -1 - indices[i];
		}assert(intArgs[i] >= 0);
		// ASSERTION Prep
		//cout << "Predicate: " << predName << " i: " << i << "Arg name: " << proc->predTbl[predName][i]
		//     << " Arg size: " << (int)proc->typeTbl[proc->predTbl[predName][i]].size() << "\n";
		// ASSERTtion prep
		assert(intArgs[i] < (int)typeTbl[predTbl[predName][i]].size());
	}
	int factNum = getIndex(predName, intArgs);
	return factNum;
}

// Return the index of the fact of the predName propType, where the ith argument is args[inidcies[i]]
// for variables and -1-indices[i] for constants
int Processor::fastNumLookupByArgIndex(unsigned predId, const VecInt& indices, const VecInt& args) {
	static VecInt intArgs;
	assert(predId < predHeadTbl.size());
	//unsigned nArgs = predParamTypeTbl[predId].size();
	if (intArgs.size() < indices.size())
		intArgs.resize(indices.size());
	for (unsigned i = 0; i < indices.size(); i++) {
		if (indices[i] >= 0) {
			assert(indices[i] < (int)args.size());
			intArgs[i] = args[indices[i]];
		} else {
			intArgs[i] = -1 - indices[i];
		}
		assert(intArgs[i] >= 0);
		//assert (intArgs[i] < (int)typeTbl[predHeadTbl[predId][i]].size());
	}
	int factNum = getIndex(intArgs, predId);
	return factNum;
}

// Return the index of the fact of the predName propType, where the ith argument is args[inidcies[i]]
// for variables and -1-indices[i] for constants
int Processor::fastFuncLookupByArgIndex(VecInt& intArgs, unsigned funcId, const VecInt& indices, const VecInt& args) {
	//static VecInt intArgs;
	assert(funcId < funcHeadTbl.size());
	//unsigned nArgs = predParamTypeTbl[predId].size();
	//if (intArgs.size() < indices.size())
	intArgs.resize(indices.size() + 1);
	for (unsigned i = 0; i < indices.size(); i++) {
		if (indices[i] >= 0) {
			assert(indices[i] < (int)args.size());
			intArgs[i + 1] = args[indices[i]];
		} else {
			intArgs[i + 1] = -1 - indices[i];
		}
		assert(intArgs[i+1] >= 0);
		//assert (intArgs[i] < (int)typeTbl[predHeadTbl[predId][i]].size());
	}
	intArgs[0] = funcId;
	int factNum = getValue(intArgs);
	return factNum;
}

void Processor::add(VecSetInt& vss, int key, int value) {
	ensureCapacity(vss, key);
	vss[key].insert(value);
}

void Processor::ensureCapacity(VecSetInt& vss, int neededIndex) {
	assert(neededIndex >= 0);
	if ((int) ((((((((((((((vss.size())))))))))))))) <= neededIndex)
		vss.resize(neededIndex + 1);

}
string Processor::asString(const StringToInt & si) {
	ostringstream os;
	for (StringToInt::const_iterator itr = si.begin(); itr != si.end(); ++itr) {
		os << itr->first << " " << itr->second << "\n";
	}
	return os.str();
}

string Processor::asString(const VecInt & vi, ListType lt) {
	ostringstream os;
	string name;
	unsigned revId;
	VecInt revArgTypes;
	unsigned typeId;
	string typeName;
	switch (lt) {
	case OPERATOR:
		name = vecOps[vi[0]]->name->getName();
		os << name;
		break;
	case REVELATION:
		os << observationIntToString[vi[0]];
		break;
	case NUMERIC:
	case PREDICATE:
		//default:
		os << vi[0] << ": ";
		break;
	case LIST:
		os << vi[0]; // no colon
		break;
	}
	for (unsigned i = 1; i < vi.size(); i++) {
		if (vi[i] == UVAL) {
			os << " u";
		} else {
			switch (lt) {
			case OPERATOR:
				os << " " << invObjectTbl[actionArgListTypes[name][i - 1]][vi[i]];
				break;
			case REVELATION:
				revId = vi[0];
				assert(revId < obsParamTypeTbl.size());
				revArgTypes = obsParamTypeTbl[revId];
				assert(i-1 < revArgTypes.size());
				typeId = revArgTypes[i - 1];
				assert(typeId < typeIdNames.size());
				typeName = typeIdNames[typeId];
				assert(invObjectTbl.find(typeName) != invObjectTbl.end());
				assert(vi[i] < (int)invObjectTbl[typeName].size());
				//os << " " << invObjectTbl[ typeIdNames[ revParamTypeTbl[vi[0]][i-1] ]][ vi[i] ];
				os << " " << invObjectTbl[typeName][vi[i]];
				break;
			case NUMERIC:
			case PREDICATE:
			case LIST:
				os << " " << vi[i];
				break;
			}
		}
	}
	return os.str();
}

string Processor::factSetAsString(const SetInt & facts) {
	ostringstream os;
	for (SetInt::const_iterator itr = facts.begin(); itr != facts.end(); ++itr) {
		os << getFact(*itr) << "; ";
	}
	return os.str();
}

string Processor::asString(const VecPayoff & vsi) {
	ostringstream os;
	for (unsigned i = 1; i < vsi.size(); i++) {
		os << vsi[i] << " ";
	}
	return os.str();
}

string Processor::asString(const VecSetInt & vsi) {
	ostringstream os;
	for (unsigned i = 0; i < vsi.size(); i++) {
		os << i << ": " << asString(vsi[i]) << endl;
	}
	return os.str();
}

string Processor::asString(const VecSetVecInt & vsvi) {
	ostringstream os;
	for (unsigned i = 0; i < vsvi.size(); i++) {
		os << i << ": ";
		for (SetVecInt::const_iterator itr = vsvi[i].begin(); itr != vsvi[i].end(); ++itr) {
			os << "(";
			for (unsigned j = 0; j < (*itr).size(); j++) {
				if (j)
					os << ",";

				os << (*itr)[j];
			}
			os << ") ";
		}

		os << "\n";
	}

	return os.str();
}

string Processor::asString(const SetInt & vsi) {
	ostringstream os;
	for (SetInt::const_iterator itr = vsi.begin(); itr != vsi.end(); ++itr) {
		os << *itr << " ";
	}
	return os.str();
}

void Processor::showLookupTables() {
	cout << "PREDICATES" << endl;
	cout << asString(predHeadTbl) << endl;
	cout << "FUNCTIONS" << endl;
	cout << asString(funcHeadTbl) << endl;
	cout << "OPERATORS" << endl;
	cout << asString(opHeadTbl) << endl;
	cout << "REVELATIONS" << endl;
	cout << asString(obsHeadTbl) << endl;
	cout << "OPSBYREV" << endl;
	cout << asString(opsByRev) << endl;
	cout << "MUTABLES" << endl;
	for (StringToInt::const_iterator itr = predHeadTbl.begin(); itr != predHeadTbl.end(); ++itr) {
		if (mutablePreds[itr->second]) {
			cout << itr->first << " ";
		}
	}

	for (StringToInt::const_iterator itr = funcHeadTbl.begin(); itr != funcHeadTbl.end(); ++itr) {
		if (mutableFuncs[itr->second]) {
			cout << itr->first << " ";
		}
	}

	cout << "\n";
	cout << "IMMUTABLES" << endl;
	for (StringToInt::const_iterator itr = predHeadTbl.begin(); itr != predHeadTbl.end(); ++itr) {
		if (!mutablePreds[itr->second]) {
			cout << itr->first << " ";
		}
	}

	for (StringToInt::const_iterator itr = funcHeadTbl.begin(); itr != funcHeadTbl.end(); ++itr) {
		if (!mutableFuncs[itr->second]) {
			cout << itr->first << " ";
		}
	}

	cout << "\n";
}

Processor::Processor(const gain_list & gains_, const operator_list & operators_) :
		maxPred(0), maxOperator(0), worldState(), gains(gains_), operators(operators_) {
}

Processor::Processor(VAL::analysis *parsedStructures) :
		maxPred(0), maxOperator(0), worldState(), gains(*parsedStructures->the_domain->gains), operators(
				*parsedStructures->the_domain->ops) {
	this->domainName = parsedStructures->the_domain->name;
	processTypes(parsedStructures->pddl_type_tab);
	processConstants(parsedStructures->const_tab);
	processPredicates(*parsedStructures->the_domain->predicates);
	if (parsedStructures->the_domain->functions)
		processFunctions(*parsedStructures->the_domain->functions);

	populateObjectLookupTable();
	generateOffsets();
	enumerateFacts();
	processOperators();
	processGains();
	enumerateInitFacts(parsedStructures->the_problem->initial_state);
	computeDerivedPredClosures(*parsedStructures->the_domain->drvs);
	initializeStaticWorldState();
}
void Processor::processConstants(const const_symbol_table & constants) {
	for (const_symbol_table::const_iterator itr = constants.begin(); itr != constants.end(); ++itr) {
		//cout << "constant: " << (*itr).first;  // A constant string
		if (superVerbose && (*itr).second->type)
			cout << ":" << (*itr).second->type->getName(); // constant is typed
		//cout <<  endl;
		assert((*itr).second->type);
		typeTbl[(*itr).second->type->getName()].insert(itr->first); // add constant to list of items of its type
	}
	nRoles = typeTbl["role"].size();
	payoffs.resize(nRoles);
	assert(nRoles > 0);
	// Now
	typeCardinalities.resize(typeNameIds.size());
	for (StringToInt::const_iterator itr = typeNameIds.begin(); itr != typeNameIds.end(); ++itr) {
		typeCardinalities[itr->second] = typeTbl[itr->first].size();
		if (superVerbose)
			cout << "There are " << typeTbl[itr->first].size() << " objects of type " << itr->first << " (id="
					<< itr->second << ")\n";

	}
}

// payoffs
bool Processor::alwaysCheckPayoffs() {
	return Processor::checkPayoffs;
}

bool Processor::isTerminal(WorldState & ws) {
	VecInt canDo = legalOperators(ws);
	if (canDo.empty()) {
		cout << "No legal moves\n";
		return true; // End of game because there are no legal moves (the usual criterion)
	}
	if (alwaysCheckPayoffs()) {
		computePayoffs(ws);
		NumScalar sum = sumPayoffs();
		if (sum > 0) {
			// Game over
			//cout << "Non zero payoffs: " << asString(this->payoffs);
			return true; // End of game because someone has a non-zero payoff.  This is a kludge to deal with the fact that
			// Racko can end when the gain condition comes true (someone has their Rack sorted).  I can eliminate this kludge
			// by reformulating the gain condition as derived predicates, or something like that.
		}
	}

	return false;
}

void Processor::computePayoffs(WorldState & ws) {
	double startt = -get_clock();
	for (unsigned i = 0; i < payoffs.size(); i++)
		payoffs[i] = 0;

	int j = 0;
	for (gain_list::const_iterator itr = gains.begin(); itr != gains.end(); ++j, ++itr) {
		NewConditionController ncc(this, ws);
		ncc.args.resize((*itr)->getArgSizeNeeded()); // the player var in the gain statement
		for (unsigned role = 1; role < nRoles; ++role) {
			//cout << "Evaluating gain " << j << " for player " << role << "\n";
			ncc.args[0] = role;
			(*itr)->visit(&ncc);
			if (ncc.truthValue) {
				//cout << "Gain evaluates to true for player " << role << " for gain decl " << j << "\n";
				payoffs[role] += (*itr)->getPayoff();
			}
		}

	}

	startt += get_clock();
	Processor::clockTotal += startt;
	//cout << "PAYOFFS: " << asString(payoffs,PAYOFFS) << "\n";
}

NumScalar Processor::sumPayoffs() {
	int sum = std::accumulate(payoffs.begin(), payoffs.end(), 0);
	return sum;
}

VecInt Processor::legalOperators(WorldState & partialWorld) {
	++PerformanceCounters::procLegalOperators;
	bool verbose = false;
	VecInt result;
	NewConditionController ncc(this, partialWorld);
	for (operator_list::const_iterator itr = operators.begin(); itr != operators.end(); ++itr) {
		string opName = (*itr)->name->getName();
		if (verbose)
			cout << "CHECKING ARG LEGALITY: " << opName << "\n";

		VecInt & argMaxes = (*itr)->argMaxes;
		ncc.args = VecInt((*itr)->getArgSizeNeeded(), -1);
		int nParams = (int) (((((((((((((((*itr)->parameters->size()))))))))))))));
		if (verbose)
			cout << "OP: " << opName << " argssize: " << ncc.args.size() << "\n";

		if (!ncc.passesScreeningChecks(*itr, -1)) {
			if (verbose)
				cout << "Failed parameter-independent screening checks\n";

			continue;
		}
		int ptr = 0;
		while (ptr >= 0) {
			if (ptr >= (int) ((((((((((((((nParams))))))))))))))) {
				//cout << "OP: " << opName << " ";
				ncc.truthValue = KNOWN_FALSE;
				//copy(ncc.args.begin(),ncc.args.end(),std::ostream_iterator<int>(cout," ")); cout << "\n";
				(*itr)->precondition->visit(&ncc);
				if (ncc.truthValue != KNOWN_FALSE) {
					//if (verbose) cout << "Found Legal!: " <<  getOperatorString(opName,ncc.args) << "\n";
					int index = getOperatorIndex(opName, ncc.args);
					result.push_back(index);
					// BEGIN TEMP
					//string oName;
					//VecInt oArgs;
					//decodeOperator(index,oName,oArgs);
					//string fullCircle = getOperatorString(oName,oArgs);
					//cout << "Lookup tests: " << index << " " << fullCircle << "\n";
					//// Now see what the effects WOULD be
					//ncc.clearEffects();
					//(*itr)->effects->visit(&ncc);
					//cout << "Add effects: ";
					//for(SetInt::const_iterator sitr = ncc.addFacts.begin(); sitr != ncc.addFacts.end(); ++sitr) {
					//  cout << getFact(*sitr) << "; ";
					//}
					//cout << "\n";
					//cout << "Del effects: ";
					//for(SetInt::const_iterator sitr = ncc.delFacts.begin(); sitr != ncc.delFacts.end(); ++sitr) {
					//  cout << getFact(*sitr) << "; ";
					//}
					//cout << "\n";
					// END TEMP
				} else {
					//if (Processor::superVerbose) cout << "Not Legal!: " <<  getOperatorString(opName,ncc.args) << "\n";
				}
				ptr--;
			} else if (ncc.args[ptr] + 1 >= argMaxes[ptr]) {
				ncc.args[ptr] = -1;
				ptr--;
			} else {
				ncc.args[ptr]++;
				// If the minimum conditions are met, then...
				if (ncc.passesScreeningChecks(*itr, ptr)) {
					ptr++;
				} //else {
				  //cout << "Screening failed on " << ptr << " = " << ncc.args[ptr] << "\n";
				  //}
			}

		}

	}

	return result;
}

void Processor::processOperators() {
	int currentOffset = 0;
	int differential = 0;
	vecOps.resize(operators.size());
	unsigned opId = 0;
	for (operator_list::const_iterator itr = operators.begin(); itr != operators.end(); ++opId, ++itr) {
		vecOps[opId] = *itr;
		(*itr)->setHeadId(opId);
		VecStr argTypes; // store the types of the args for this predicate
		string opName = (*itr)->name->getName();
		opHeadTbl[opName] = opId;
		operatorIntToString.push_back(opName);
		opsFromParser[opName] = (operator_*) (((((((((((((((*itr))))))))))))))); // Be able to lookup the operator structure by its name
		//cout << "OP: " << opName << " ";
		//var_symbol_list::const_iterator sitr = (*itr)->parameters->begin();
		for (var_symbol_list::const_iterator sitr = (*itr)->parameters->begin(); sitr != (*itr)->parameters->end();
				++sitr) {
			if (superVerbose)
				cout << (*sitr)->type->getName() << " ";

			argTypes.push_back((*sitr)->type->getName());
		}
		if (superVerbose)
			cout << "\n";

		OperatorController opCon(this);
		(*itr)->visit(&opCon);
		opsTbl.insert(PredicateTableEntry(opName, VecStr(argTypes)));
		VecInt mults(argTypes.size());
		//assert (!mults.empty());
		if (mults.empty()) {
			differential = 1;
		} else {
			mults[mults.size() - 1] = 1; // Identity multiplier for last argument
			for (int m = (int) ((((((((((((((mults.size())))))))))))))) - 2; m >= 0; m--) {
				mults[m] = mults[m + 1] * typeTbl[argTypes[m + 1]].size();
				//  count *= typeTbl[argTypes[m]].size();
			}
			differential = mults[0] * typeTbl[argTypes[0]].size();
		}

		opsOffsets.insert(StringToBoundsEntry(opName, Bounds(0, differential)));
		currentOffset += differential;
		opsMults.insert(PredicateMultTableEntry((*itr)->name->getName(), VecInt(mults)));
		actionArgListTypes[opName] = argTypes;
	}

	// Encode offsets for operators
	currentOffset = 0;
	for (StringToBounds::iterator itr = opsOffsets.begin(); itr != opsOffsets.end(); ++itr) {
		differential = itr->second.upper;
		itr->second.lower += currentOffset;
		itr->second.upper += currentOffset - 1;
		if (superVerbose)
			cout << "OpName: " << itr->first << " min: " << itr->second.lower << " max: " << itr->second.upper << endl;

		maxOperator = itr->second.upper;
		currentOffset += differential;
	}
	for (StringToInt::const_iterator itr = obsHeadTbl.begin(); itr != obsHeadTbl.end(); ++itr) {
		if (superVerbose)
			cout << itr->first << ":" << itr->second << "\n";
		assert(itr->second < (int)obsParamTypeTbl.size());
		assert(itr->second < (int)obsParamTypeCardTbl.size());
		for (unsigned j = 0; j < obsParamTypeTbl[itr->second].size(); j++) {
			if (superVerbose)
				cout << "Param " << j << " has id " << obsParamTypeTbl[itr->second][j] << " and card "
						<< obsParamTypeCardTbl[itr->second][j] << "\n";
		}
	}
}

void Processor::processGains() {
	for (gain_list::const_iterator itr = gains.begin(); itr != gains.end(); ++itr) {
		OperatorController opCon(this);
		(*itr)->visit(&opCon);
	}
}

void Processor::processPredicates(const pred_decl_list & predicates) {
	int currentOffset = 0;
	int differential = 0;
	unsigned i = 0;
	for (pred_decl_list::const_iterator itr = predicates.begin(); itr != predicates.end(); ++i, ++itr) {
		VecStr argTypes; // store the types of the args for this predicate
		const string & predName = (*itr)->head->getName();
		predHeadTbl[predName] = i;
		//inversePreds.push_back(predName);
		if (superVerbose)
			cout << "PredHead " << predName << " = " << i << "\n";

		// iterate by alphabetical order
		//for (var_symbol_table::const_iterator sitr = (*itr)->var_tab->begin(); sitr != (*itr)->var_tab->end(); ++sitr) {
		//    //cout << sitr->second->type->getName() << " ";
		//    argTypes.push_back(sitr->second->type->getName());
		//}
		for (var_symbol_list::const_iterator sitr = (*itr)->args->begin(); sitr != (*itr)->args->end(); ++sitr) {
			//cout << sitr->second->type->getName() << " ";
			argTypes.push_back((*sitr)->type->getName());
		}
		//cout << "\n";
		predTbl.insert(PredicateTableEntry(predName, VecStr(argTypes)));
		predParamTypeTbl.push_back(VecInt(argTypes.size()));
		predParamTypeCardTbl.push_back(VecInt(argTypes.size()));
		for (unsigned j = 0; j < argTypes.size(); j++) {
			predParamTypeTbl[i][j] = typeNameIds[argTypes[j]];
			predParamTypeCardTbl[i][j] = typeCardinalities[typeNameIds[argTypes[j]]];
			if (superVerbose)
				cout << "Parameter " << j << " for predicate " << predName << "(id " << i << ") has type "
						<< argTypes[j] << " (id " << typeNameIds[argTypes[j]] << ") which has cardinality "
						<< typeCardinalities[typeNameIds[argTypes[j]]] << "\n";

		}
		offsets[predName] = 0;
		VecInt mults(argTypes.size());
		//cout << "QQ: " << predName << " " << argTypes.size() << "\n";
		//assert (!mults.empty());
		if (!mults.empty()) {
			mults[mults.size() - 1] = 1; // Identity multiplier for last argument
			int count = typeTbl[argTypes[mults.size() - 1]].size();
			for (int m = (int) ((((((((((((((mults.size())))))))))))))) - 2; m >= 0; m--) {
				mults[m] = mults[m + 1] * typeTbl[argTypes[m + 1]].size();
				count *= typeTbl[argTypes[m]].size();
			}
			counts[predName] = count;
			if (superVerbose)
				cout << "predicate " << predName << ": "; // Name of the predicate

			int index = 0;
			for (var_symbol_list::const_iterator sitr = (*itr)->args->begin(); sitr != (*itr)->args->end(); ++sitr) {
				if (superVerbose)
					cout << (*sitr)->type->getName() << "(" << mults[index] << ") ";

				++index;
			}
			if (superVerbose)
				cout << " total: " << counts[predName] << "\n";

			predMults.insert(PredicateMultTableEntry((*itr)->head->getName(), VecInt(mults)));
			differential = mults[0] * typeTbl[argTypes[0]].size();
		} else {
			counts[predName] = 1;
			differential = 1; // For named propositional constants
			predMults.insert(PredicateMultTableEntry((*itr)->head->getName(), VecInt())); // TODO: Can set up fast Pred mults right here instead of copying later
		}
		factsOffsets.insert(StringToBoundsEntry(predName, Bounds(0, differential)));
	}

	fastPredMults.resize(predMults.size());
	for (PredicateMultTable::const_iterator itr = predMults.begin(); itr != predMults.end(); ++itr) {
		fastPredMults[predHeadTbl[itr->first]] = itr->second;
	}
	// Encode offsets for facts
	currentOffset = 0;
	fastOffsets.resize(predHeadTbl.size());
	for (StringToBounds::iterator itr = factsOffsets.begin(); itr != factsOffsets.end(); ++itr) {
		differential = itr->second.upper;
		itr->second.lower += currentOffset;
		itr->second.upper += currentOffset - 1;
		if (superVerbose)
			cout << "PredName: " << itr->first << " min: " << itr->second.lower << " max: " << itr->second.upper
					<< endl;

		fastOffsets[predHeadTbl[itr->first]] = itr->second.lower;
		maxPred = itr->second.upper;
		currentOffset += differential;
	}
	// Initialize the values for (whoseturn i)
	int whoseturnIndex = predHeadTbl["whoseturn"];
	VecInt args(1);
	WorldState::roleIds.resize(nRoles);
	for (unsigned i = 0; i < nRoles; i++) {
		args[0] = i;
		int index = getIndex(args, whoseturnIndex);
		WorldState::roleIds[i] = index;
	}
	if (WorldState::intLength(maxPred) > WorldState::intsize) {
		std::cerr << "Ints needed: " << WorldState::intLength(maxPred) << " Allocated: " << WorldState::intsize << "\n";
		exit(1);
	}
	worldState = WorldState(maxPred, nRoles);
	//
	this->mutablePreds = VecInt(predicates.size(), 0);
}

void Processor::processFunctions(const func_decl_list & functions) {
	mutableFuncs = VecInt(functions.size(), 0);
}

void Processor::processTypes(const pddl_type_symbol_table & types) {
	int i = 0;
	for (pddl_type_symbol_table::const_iterator itr = types.begin(); itr != types.end(); ++i, ++itr) {
		if (superVerbose)
			cout << (*itr).first << " " << i << "\n";

		typeNameIds[(*itr).first] = i;
		typeIdNames.push_back((*itr).first);
	}
}

void Processor::populateObjectLookupTable() {
	unsigned typeId = 0;
	this->objectTblByTypeId.resize(typeTbl.size());
	for (TypeTable::const_iterator itr = typeTbl.begin(); itr != typeTbl.end(); ++itr, ++typeId) {
		const SetStr& s = itr->second;
		unsigned i = 0;
		this->objectTblByTypeId[typeId].resize(s.size());
		for (SetStr::const_iterator sitr = s.begin(); sitr != s.end(); ++sitr) {
			objectTbl.insert(pair<string, int>(*sitr, i));
			this->objectTblByTypeId[typeId][i] = *sitr;
			objectTypes.insert(pair<string, string>(*sitr, itr->first));
			VecStr& invLookupTbl = invObjectTbl[itr->first];
			assert(invLookupTbl.size() == i);
			invLookupTbl.push_back(*sitr);
			++i;
		}
	}
}

void Processor::write2(ostream & o) {
	for (TypeTable::const_iterator itr = typeTbl.begin(); itr != typeTbl.end(); ++itr) {
		o << itr->first << ":\n";
		const SetStr & s = itr->second;
		unsigned i = 0;
		for (SetStr::const_iterator sitr = s.begin(); sitr != s.end(); ++i, ++sitr) {
			o << "    " << *sitr << " " << i << endl;
		}
	}

	for (PredicateTable::const_iterator itr = predTbl.begin(); itr != predTbl.end(); ++itr) {
		o << itr->first << ": ";
		const VecStr & s = itr->second;
		for (VecStr::const_iterator sitr = s.begin(); sitr != s.end(); ++sitr) {
			o << " " << typeTbl[*sitr].size();
		}
		o << endl;
	}

}

void Processor::generateFacts(const string & pred, const VecStr & argTypes, VecInt & args, unsigned level) {
	static const bool verbose = false;
	if (level == argTypes.size()) {
		if (verbose)
			cout << pred << " ";

		if (verbose)
			cout << ": " << pred << "(";

		for (unsigned i = 0; i < args.size(); i++) {
			if (verbose)
				if (i)
					cout << ", ";

			if (verbose)
				cout << invObjectTbl[argTypes[i]][args[i]];

		}
		if (verbose)
			cout << "): " << getIndex(pred, args) << "\n";

	} else {
		unsigned thisMax = typeTbl[argTypes[level]].size();
		for (unsigned i = 0; i < thisMax; i++) {
			args[level] = i;
			generateFacts(pred, argTypes, args, level + 1);
		}
	}

}

void Processor::enumerateFacts() {
	string predName;
	for (PredicateTable::const_iterator itr = predTbl.begin(); itr != predTbl.end(); ++itr) {
		predName = itr->first;
		const VecStr & argTypes = itr->second;
		VecInt args(argTypes.size()); // Will work for up to
		generateFacts(predName, argTypes, args, 0);
	}
}

int Processor::intLength(unsigned maxBit) {
	int result = (maxBit / (sizeof(int) * 8));
	if (result * (sizeof(int) * 8) != maxBit) {
		++result; // computation of result rounded down, so we need one more
	}
	return result;
}

bool Processor::hasPred(const string & name) {
	PredicateTable::const_iterator itr = predTbl.find(name);
	return (itr != predTbl.end());
}

void Processor::enumerateLegalActions(int *state, unsigned maxFact, unsigned maxAction) {
	assert(false);
	ConditionController cc(this);
	cout << maxAction << "\n";
	for (unsigned i = 0; i < maxAction; i++) {
		cout << i << "\n";
	}
}

void Processor::computeDerivedPredClosures(derivations_list & drvs) {
	// Create and initialize controller
	ConditionController cc(this);
	do {
		if (superVerbose)
			cout << "Computing Deriveds" << std::endl;

		cc.resetFixedPointCheck();
		for (derivations_list::iterator itr = drvs.begin(); itr != drvs.end(); ++itr) {
			cc.resetParamList();
			//(*itr)->write2(cout,0) ;
			VecInt vi((*itr)->vtab->size());
			const parameter_symbol_list & psl = *(*itr)->head->getArgs();
			for (parameter_symbol_list::const_iterator pitr = psl.begin(); pitr != psl.end(); ++pitr) {
				// Store the names of the arguments so that we can figure out what to map them to later
				cc.argStrings.push_back((*pitr)->getName());
				//cout << "Pushing item onto list: " << (*pitr)->getName() << "\n";
			}
			derive(cc, **itr, vi, 0);
		}

	} while (!cc.hasFixedPoint());
	// Now see which facts are true
	if (superVerbose)
		cout << "Finally true facts: " << "\n";

	if (superVerbose)
		for (unsigned i = 0; i < maxPred; i++) {
			printState();
		}

}

bool Processor::checkTruth(ConditionController & cc, derivation_rule & dr) {
	cc.truthValue = true;
	dr.body->visit(&cc);
	return cc.truthValue;
}

bool Processor::satisfiesCondition(ConditionController & cc, goal *g) {
	assert(g);
	cc.truthValue = true;
	g->visit(&cc);
	return cc.truthValue;
}

void Processor::derive(ConditionController & cc, derivation_rule & dr, VecInt & vi, unsigned ind) {
	if (ind >= vi.size()) {
		//if (superVerbose) cout << dr.head->head->getName() << " ";
		//if (superVerbose) for (unsigned i = 0; i < vi.size(); i++) {
		//  cout << vi[i] << " " ;
		//}
		//if (superVerbose) cout << " || ";
		//if (superVerbose) for (StringToInt::const_iterator itr = cc.params.begin(); itr != cc.params.end(); ++itr) {
		//  cout << itr->first << ":" << itr->second << " ";
		//}
		int factNum = getIndex(dr.head->head->getName(), vi);
		//if (superVerbose) cout << "(" << factNum << ")";
		if (checkTruth(cc, dr)) {
			//assert(false); //cc.set(factNum);
			if (worldState.getTruthValue(factNum) != KNOWN_TRUE) {
				cc.foundNewTrueFact();
			}
			worldState.setTruthValue(factNum, KNOWN_TRUE);
			//if (superVerbose) cout << " is true";
		}
		//else if (superVerbose) cout << "is false";
		//if (superVerbose) cout << "\n";
	} else {
		const VecStr& argTypes = predTbl[dr.head->head->getName()];
		unsigned thisMax = typeTbl[argTypes[ind]].size();
		assert(ind < cc.argStrings.size());
		string argName = cc.argStrings[ind];
		for (unsigned j = 0; j < thisMax; j++) {
			vi[ind] = j;
			cc.params[argName] = j;
			derive(cc, dr, vi, ind + 1);
		}
	}
}

void Processor::enumerateInitFacts(effect_lists *initFacts) {
	if (superVerbose)
		cout << "INITIAL STATE" << endl;

	InitialStateVisitController isvc(this);
	initFacts->visit(&isvc);
	// This stuff should now be done during the visit
	//VecInt vi = isvc.initState;
	////for (VecInt::iterator itr = vi.begin(); itr != vi.end(); ++itr) {
	////  if (superVerbose) cout << getFact(*itr) << endl;
	////  Set(stateBits,*itr);
	////}
}
void Processor::generateOffsets() {
	int sumTotal = 0;
	for (StringToInt::const_iterator itr = counts.begin(); itr != counts.end(); ++itr) {
		offsets[itr->first] = sumTotal;
		if (superVerbose)
			cout << "Offset for " << itr->first << ": " << sumTotal << "\n";

		sumTotal += itr->second;
	}
}

const StringToBoundsEntry Processor::getPredName(const StringToBounds & map, unsigned index) {
	StringToBounds::const_iterator itr = map.begin();
	while (itr->second.upper < index && itr != map.end())
		++itr;

	assert(itr != map.end());
	return *itr;
}
StringToIntEntry Processor::getPredName(int index) {
	StringToInt::const_iterator currentOffsetItr = offsets.begin();
	StringToInt::const_iterator countsItr = counts.begin();
	while (countsItr != counts.end()) {
		if (countsItr->second + currentOffsetItr->second > index)
			break;

		++currentOffsetItr;
		++countsItr;
	}assert(currentOffsetItr != offsets.end());
	return *currentOffsetItr;
}

// Fluents
int Processor::getValue(const FunctionKey & key) {
	int result;
	FTitr itr = fluentVals.find(key);
	if (itr != fluentVals.end()) {
		result = itr->second;
	} else {
		result = INT_MAX; // Infinity
	}
	return result;
}

void Processor::setValue(const FunctionKey & key, int value) {
	//cout << "Assigning " << asIntString(key) << " = " << value << "\n";
	assert(value < 100);
	fluentVals[key] = value;
}

// Facts
string Processor::getFact(int index) {
	std::ostringstream os;
	StringToIntEntry stie = getPredName(index);
	const VecStr & predVars = predTbl[stie.first];
	const VecInt & mults = predMults[stie.first];
	int localOffset = index - stie.second;
	assert(localOffset >= 0);
	os << stie.first << "(";
	//<< localOffset
	//<< ")  ";
	unsigned var = 0;
	while (var < mults.size()) {
		string object = invObjectTbl[predVars[var]][localOffset / mults[var]];
		if (var > 0)
			os << ", ";

		os << object;
		localOffset = (localOffset % mults[var]);
		var++;
	}
	os << ")";
	return os.str();
}

string Processor::getFact(int index, bool & canChange) {
	std::ostringstream os;
	StringToIntEntry stie = getPredName(index);
	int predHeadId = predHeadTbl[stie.first];
	assert(predHeadId < (int)mutablePreds.size());
	canChange = (mutablePreds[predHeadId] == 1);
	const VecStr & predVars = predTbl[stie.first];
	const VecInt & mults = predMults[stie.first];
	int localOffset = index - stie.second;
	assert(localOffset >= 0);
	os << "(" << stie.first << " ";
	//<< localOffset
	//<< ")  ";
	unsigned var = 0;
	while (var < mults.size()) {
		string object = invObjectTbl[predVars[var]][localOffset / mults[var]];
		if (var > 0)
			os << " ";

		os << object;
		localOffset = (localOffset % mults[var]);
		var++;
	}
	os << ")";
	return os.str();
}

string Processor::getOperatorString(const string & name, const VecInt & args) {
	std::ostringstream os;
	os << name << "(";
	unsigned argsize = actionArgListTypes[name].size();
	assert(args.size() >= argsize);
	for (unsigned i = 0; i < argsize; i++) {
		if (i)
			os << ",";

		os << invObjectTbl[actionArgListTypes[name][i]][args[i]];
	}
	os << ")";
	return os.str();
}

void Processor::randomTests() {
	for (unsigned i = 0; i <= maxPred; i++) {
		StringToIntEntry stie = getPredName(i);
		cout << "Index: " << i << " <=> " << getFact(i) << "\n";
	}
}

int Processor::getNum(string pred, int arg0) {
	return offsets[pred] + arg0;
}

int Processor::getNum(string pred, int arg0, int arg1) {
	const VecInt & mults = predMults[pred];
	if (mults.empty()) {
		return offsets[pred];
	}
	return offsets[pred] + arg0 * mults[0] + arg1;
}

int Processor::getNum(string pred, int arg0, int arg1, int arg2) {
	const VecInt & mults = predMults[pred];
	if (mults.empty()) {
		return offsets[pred];
	}
	return offsets[pred] + arg0 * mults[0] + arg1 * mults[1] + arg2;
}

operator_ *Processor::getOpFromIndex(int index) {
	const StringToBoundsEntry stbe = getPredName(opsOffsets, index);
	const string & oName = stbe.first;
	assert(oName != "");
	StringToInt::const_iterator finder = opHeadTbl.find(oName);
	assert(finder != opHeadTbl.end());
	assert(finder->second < (int)vecOps.size());
	return vecOps[finder->second];
}

//operators
void Processor::decodeOperator(int index, string & oName, VecInt & oArgs, bool resize) {
	const StringToBoundsEntry stbe = getPredName(opsOffsets, index);
	oName = stbe.first;
	VecInt & mults = opsMults[oName];
	if (resize)
		oArgs.resize(mults.size());

	index -= stbe.second.lower;
	assert(index >= 0);
	unsigned var = 0;
	while (var < mults.size()) {
		oArgs[var] = index / mults[var];
		index %= mults[var];
		var++;
	}
}

string Processor::operatorIndexToString(int index) {
	string oName;
	VecInt oArgs;
	decodeOperator(index, oName, oArgs);
	string result = getOperatorString(oName, oArgs);
	return result;
}

VecInt Processor::operatorIndexToVector(int index) {
	string oName;
	VecInt oArgs;
	decodeOperator(index, oName, oArgs);
	VecInt result;
	result.push_back(opHeadTbl[oName]);
	result.insert(result.end(), oArgs.begin(), oArgs.end());
	return result;
}

VecStr Processor::getOperatorString(int index, const string & name, const VecStr & vars, const VecInt & mults) {
	// \TODO is the index absolute or relative to the offest?  It looks like I need to substract off an offset here.
	VecStr result;
	const StringToBoundsEntry stbe = getPredName(opsOffsets, index);
	//cout << "getOperatorString: " << stbe.first << " " << stbe.second.lower << " " << index << " " << stbe.second.upper << endl;
	assert(index >= 0);
	unsigned var = 0;
	while (var < mults.size()) {
		string object = invObjectTbl[vars[var]][index / mults[var]];
		result.push_back(object);
		index %= mults[var];
		var++;
	}
	return result;
}

int Processor::getIndex(const VecInt & args, int headId) {
	int result = fastOffsets[headId];
	const VecInt & mults = fastPredMults[headId];
	for (unsigned i = 0; i < mults.size(); i++) {
		result += mults[i] * args[i];
	}assert(result >= 0);
	return result;
}

int Processor::getIndex(const string & name, const VecInt & args) {
	const PredicateMultTable::const_iterator itr = predMults.find(name);
	assert(itr != predMults.end());
	const VecInt & mults = itr->second;
	const StringToBounds::const_iterator bitr = factsOffsets.find(name);
	assert(bitr != factsOffsets.end());
	const Bounds & bounds = bitr->second;
	assert(args.size() >= mults.size());
	int result = bounds.lower;
	for (unsigned i = 0; i < mults.size(); i++) {
		result += mults[i] * args[i];
	}
	return result;
}

int Processor::getIndex(const StringToBounds & offsets, const PredicateMultTable & multTbl, const string & name,
		const VecInt & args) {
	const PredicateMultTable::const_iterator itr = multTbl.find(name);
	assert(itr != multTbl.end());
	const VecInt & mults = itr->second;
	const StringToBounds::const_iterator bitr = offsets.find(name);
	assert(bitr != offsets.end());
	const Bounds & bounds = bitr->second;
	assert(args.size() >= mults.size());
	int result = bounds.lower;
	for (unsigned i = 0; i < mults.size(); i++) {
		result += mults[i] * args[i];
	}
	return result;
}

int Processor::getOperatorIndex(const string & name, const VecStr & args) {
	VecInt intArgs(args.size());
	for (unsigned i = 0; i < args.size(); i++) {
		intArgs[i] = objectTbl[args[i]];
	}
	return getOperatorIndex(name, intArgs);
}

int Processor::getOperatorIndex(const string & name, const VecInt & args) {
	return getIndex(opsOffsets, opsMults, name, args);
}

int Processor::getOperatorIndex(int offset, const VecInt & mults, const VecInt & args) {
	assert(false);
	int result = offset;
	for (unsigned i = 0; i < mults.size(); i++) {
		result += mults[i] * args[i];
	}
	return result;
}

VecVecKey Processor::fullApply(int index, WorldState & ws) {
	VecVecKey result;
	ws.clearUpdates();
	apply(index, ws);
	result = ws.obsListByPlyr;
	ws.applyUpdates();
	return result;
}

void Processor::apply(const VecInt & sequence, WorldState & ws) {
	for (unsigned t = 1; t < sequence.size(); t++) {
		//for (unsigned p = 0; p < ws.maxPred; p++) {
		//  assert (ws.getTruthValue(p) != UNKNOWN);
		//}
		NewConditionController ncc(this, ws);
		int index = sequence[t];
		shortApply(index, ws, ncc);
	}
}

void Processor::shortApply(int index, WorldState & ws, NewConditionController & ncc) {
	operator_ *op = getOpFromIndex(index);
	ncc.args = VecInt(op->getArgSizeNeeded(), -1);
	string dummy;
	decodeOperator(index, dummy, ncc.args);
	op->precondition->visit(&ncc);
	assert(ncc.truthValue == KNOWN_TRUE);
	ws.clearUpdates();
	apply(index, ws);
	ws.applyUpdates();
}

VecVecVecKey Processor::getObs(const VecInt & sequence, WorldState & ws) {
	VecVecVecKey result;
	for (unsigned t = 1; t < sequence.size(); t++) {
		//for (unsigned p = 0; p < ws.maxPred; p++) {
		//  assert (ws.getTruthValue(p) != UNKNOWN);
		//}
		NewConditionController ncc(this, ws);
		int index = sequence[t];
		operator_* op = getOpFromIndex(index);
		ncc.args = VecInt(op->getArgSizeNeeded(), -1);
		string dummy;
		decodeOperator(index, dummy, ncc.args);
		op->precondition->visit(&ncc);
		assert(ncc.truthValue == KNOWN_TRUE);
		result.push_back(fullApply(index, ws));
	}
	return result;
}

// applying actions
void Processor::apply(int opIndex, WorldState & ws) {
	string oName;
	VecInt oArgs;
	decodeOperator(opIndex, oName, oArgs);
	apply(oName, oArgs, ws);
}

void Processor::apply(const string & name, const VecInt & args, WorldState & ws) {
	// Get the operator that has this name
	operator_ *op = opsFromParser[name];
	assert(op);
	// Initialize a NewConditionController
	NewConditionController ncc(this, ws);
	if (op->getArgSizeNeeded() > args.size()) {
		ncc.setArgSize(op->getArgSizeNeeded());
		copy(args.begin(), args.end(), ncc.args.begin());
	} else {
		ncc.args = args;
	}
	// assert that we're legal
	//ncc.truthValue = KNOWN_FALSE;
	//op->precondition->visit(&ncc);
	//assert (ncc.truthValue == KNOWN_TRUE);
	// Call visit on the effects
	op->effects->visit(&ncc);
	//worldState.applyUpdates();
	// Also update the observations
	//kb.push_back(ncc.obsListByPlyr);
}

void Processor::finalizeApply(WorldState & ws, bool readObs) {
	if (readObs)
		kb.push_back(ws.obsListByPlyr); // Need to do this first because applyUpdates will clear out the observations

	ws.applyUpdates();
}
void Processor::generateOps(const string & opName, const VecStr & argTypes, VecInt & args, unsigned level) {
	if (level == argTypes.size()) {
		if (superVerbose)
			cout << opName << "(";

		if (superVerbose)
			for (unsigned i = 0; i < level; i++) {
				if (i)
					cout << ", ";

				cout << invObjectTbl[argTypes[i]][args[i]];
			}

		if (superVerbose)
			cout << ") <=> ";

		const Bounds & bounds = opsOffsets[opName];
		if (superVerbose)
			cout << getOperatorIndex(bounds.lower, opsMults[opName], args);

		if (superVerbose)
			cout << "\n";

	} else {
		unsigned thisMax = typeTbl[argTypes[level]].size();
		//cout << "thisMax: " << opName << " " << argTypes[level] << thisMax << endl;
		for (unsigned i = 0; i < thisMax; i++) {
			args[level] = i;
			generateOps(opName, argTypes, args, level + 1);
		}
	}

}

void Processor::enumerateOperators() {
	if (superVerbose)
		cout << "ENUMERATE OPERATORS" << endl;

	VecInt args; // Will work for up to
	string opName;
	for (PredicateTable::const_iterator itr = opsTbl.begin(); itr != opsTbl.end(); ++itr) {
		opName = itr->first;
		const VecStr & argTypes = itr->second;
		args.resize(argTypes.size());
		generateOps(opName, argTypes, args, 0);
	}
}

int Processor::encodeFact(const proposition *p, const StringToInt & params) {
	const parameter_symbol_list & vsl = *p->getArgs();
	VecInt indexes;
	for (parameter_symbol_list::const_iterator itr = vsl.begin(); itr != vsl.end(); itr++) {
		const string& arg = (*itr)->getName();
		//ostringstream os;
		StringToInt::const_iterator index = params.find(arg);
		if (index == params.end()) {
			index = objectTbl.find(arg);
			//labels.push_back((*itr)->getName());
			//} else {
			assert(index != objectTbl.end());
			// reference to a constant in action def that hasn't been defined
		}
		//os << index->second;
		indexes.push_back(index->second);
		//labels.push_back(os.str());
	}
	int factNum = getIndex(factsOffsets, predMults, p->getHead()->getName(), indexes);
	cout << "(" << factNum << ")";
	return factNum;
}

int Processor::encodeGroundedFact(const proposition *p) {
	const parameter_symbol_list & vsl = *p->getArgs();
	VecInt indexes(vsl.size());
	int i = 0;
	for (parameter_symbol_list::const_iterator itr = vsl.begin(); itr != vsl.end(); ++itr, ++i) {
		const string& arg = (*itr)->getName();
		StringToInt::const_iterator index = objectTbl.find(arg);
		assert(index != objectTbl.end());
		// reference to a constant in action def that hasn't been defined
		indexes[i] = index->second;
	}
	int factNum = getIndex(factsOffsets, predMults, p->getHead()->getName(), indexes);
	//cout << "(" << factNum << ")";
	return factNum;
}

void Processor::initializeStaticWorldState() {
	WorldState::predHeadTbl = this->predHeadTbl;
	WorldState::funcHeadTbl = this->funcHeadTbl;
	WorldState::fastOffsets = this->fastOffsets;
	WorldState::fastPredMults = this->fastPredMults;
	this->initialWorld = this->worldState; // save a copy of the initial state for later use
}

string Processor::printState(int *x, int max) {
	ostringstream os;
	os << "CURRENT STATE" << "\n";
	for (int i = max; i >= 0; i--) {
		os << (IsSet(x,i) ? "1" : "0");
	}
	os << "\n";
	for (int i = 0; i <= max; i++) {
		if (IsSet(x,i)) {
			bool canChange;
			string s = getFact(i, canChange);
			canChange = true;
			if (canChange) {
				os << s << "\n";
			}
		}
	}
	for (FTitr itr = fluentVals.begin(); itr != fluentVals.end(); ++itr) {
		assert(!itr->first.empty());
		int funcId = itr->first[0];
		assert(funcId < (int)mutableFuncs.size());
		if (mutableFuncs[funcId] == 1) {
			os << asIntString(itr->first) << "=" << (itr)->second << "\n";
		}
	}
	return os.str();
}

string Processor::printPartialState(const WorldState & ws) {
	ostringstream os;
	os << "CURRENT PARTIAL STATE (whoseturn = " << ws.getWhoseTurn() << ")\n";
	//for (int i = ws.maxPred; i >= 0; i--)
	//{
	//  //os << (IsSet(x,i) ? "1" : "0");
	//  os << ws.getTruthValue(i);
	//}
	//os << "\n";
	for (unsigned i = 0; i <= ws.maxPred; i++) {
		TruthState ts = ws.getTruthValue(i);
		bool canChange = false;
		string s = getFact(i, canChange);
		if (canChange && ts != KNOWN_FALSE) {
			os << s << "    " << ws.getTruthValue(i) << "\n";
		}
	}

	//for (FTitr itr = ws.fluentVals.begin(); itr != ws.fluentVals.end(); ++itr) {
	//   os << asIntString(itr->first) << "=" << (itr)->second << "\n";
	// }
	return os.str();
}

string Processor::printState(const WorldState & ws, bool includeKnowledgeBases) {
	ostringstream os;
	os << "STATE (whoseturn = " << ws.getWhoseTurn() << ")\n";
	for (int i = ws.maxPred; i >= 0; i--) {
		//os << (IsSet(x,i) ? "1" : "0");
		os << ws.getTruthValue(i);
	}
	os << "\n";
	for (unsigned i = 0; i <= ws.maxPred; i++) {
		if (ws.getTruthValue(i) == KNOWN_TRUE) {
			bool canChange;
			string s = getFact(i, canChange);
			//canChange = true;
			if (canChange) {
				os << s << "\n";
			}
		}

	}

	for (FTitr itr = ws.fluentVals.begin(); itr != ws.fluentVals.end(); ++itr) {
		assert(!itr->first.empty());
		int funcId = itr->first[0];
		assert(funcId < (int)mutableFuncs.size());
		if (mutableFuncs[funcId] == 1) {
			os << inverseFuncs[funcId] << " " << asIntString(itr->first) << "=" << (itr)->second << "\n";
		} else {
			//os << inverseFuncs[funcId] << " " << asIntString(itr->first) << "=" << (itr)->second << " NOT MUTABLE: " << itr->first[0] << "\n";
		}
	}
	if (includeKnowledgeBases) {
		for (int p = 1; p < ws.getNRoles(); p++) {
			os << printKnowledgeState(p) << "\n";
		}
	}

	return os.str();
}

void Processor::printState() {
	cout << printState(worldState) << endl;
	unsigned p = 0;
	if (nRoles > 2)
		p++; // Don't print out for chance (this is a kludge)

	for (; p < nRoles; p++) {
		cout << printKnowledgeState(p) << "\n";
	}
}

string Processor::observationToString(VecInt & currentObs) {
	assert(!currentObs.empty());
	ostringstream os;
	assert(currentObs[0] < (int)this->observationIntToString.size());
	os << "(" << this->observationIntToString[currentObs[0]];
	int observationHeadId = currentObs[0];
	VecInt & obsParamTypeIds = this->obsParamTypeTbl[observationHeadId];
	for (int j = 1; j < (int) (((((((((((((currentObs.size()))))))))))))); j++) {
		os << " ";
		int paramIndex = j - 1;
		int paramTypeId = obsParamTypeIds[paramIndex];
		if (paramTypeId == UVAL) {
			os << "??";
		} else {
			if (currentObs[j] == UVAL) {
				os << "?";
			} else if (currentObs[j] < 0) {
				os << this->getObjectName(paramTypeId, -1 - currentObs[j]);
			} else {
				os << this->getObjectName(paramTypeId, currentObs[j]);
			}

		}

	}

	os << ")";
	return os.str();
}

string Processor::printKnowledgeState(int player) {
	ostringstream os;
	os << "KB" << player << "\n";
	for (unsigned t = 0; t < kb.size(); t++) {
		assert(player < (int)kb[t].size());
		os << t << ": ";
		for (unsigned o = 0; o < kb[t][player].size(); o++) {
			VecInt& currentObs = kb[t][player][o];
			os << this->observationToString(currentObs) << "; ";
		}
		os << "\n";
	}
	return os.str();
}

// Revelations
bool Processor::isRevAlreadyPresent(const string & name) {
	return obsHeadTbl.find(name) != obsHeadTbl.end();
}

unsigned Processor::newRevelationHeadId(const string & name, unsigned nArgs) {
	assert(obsHeadTbl.size() == obsParamTypeTbl.size());
	unsigned result = obsHeadTbl.size();
	obsParamTypeTbl.push_back(VecInt(nArgs));
	obsHeadTbl[name] = result;
	return result;
}

bool Processor::isPredAlreadyPresent(const string & name) {
	return predHeadTbl.find(name) != predHeadTbl.end();
}

unsigned Processor::newPredHeadId(const string & name, unsigned nArgs) {
	assert(predHeadTbl.size() == predParamTypeTbl.size());
	unsigned result = predHeadTbl.size();
	predParamTypeTbl.push_back(VecInt(nArgs));
	predHeadTbl[name] = result;
	return result;
}

string Processor::getOperatorName(int opHeadId) {
	assert(opHeadId < (int)operatorIntToString.size());
	return this->operatorIntToString[opHeadId];
}

void Processor::setStaticEvaluationFunction(StaticEvaluationFunction *evaluator) {
	assert(evaluator != NULL);
	this->evaluator = evaluator;
}

int Processor::getPredId(const string predName) {
	return this->predHeadTbl[predName];
}

string Processor::getHistory(const VecInt & gameHistory) {
	std::ostringstream os;
	os << "History\n";
	for (unsigned i = 1; i < gameHistory.size(); i++) {
		os << this->operatorIndexToString(gameHistory[i]) << "\n";
	}
	os << "EndHistory\n";
	return os.str();
}

// Does the same things as getHistory, so these could be combined
string Processor::getFormattedLegalMoves(const VecInt & moves) {
	std::ostringstream os;
	os << "Legal Moves\n";
	for (unsigned i = 0; i < moves.size(); i++) {
		os << i << " " << this->operatorIndexToString(moves[i]) << "\n";
	}
	return os.str();
}

string Processor::winnerDeclarationString(const VecPayoff & payoffs) const {
	string result = "";
	if (payoffs.size() == 3) {
		if (payoffs[1] > payoffs[2]) {
			result = "Player 1 wins!";
		} else if (payoffs[2] > payoffs[1]) {
			result = "Player 2 wins!";
		} else {
			result = "Tie game!";
		}

	}

	return result;
}

unsigned Processor::itemCount(const string & name) {
	return this->typeCardinalities[this->typeNameIds[name]];
}

unsigned Processor::predicateId(const string & name) {
	return this->predHeadTbl[name];
}

// Info set stuff
VecInt Processor::getPartialOperator(const VecInt & observation, const VecInt & argMapper) {
	assert(!argMapper.empty());
	assert(observation.size() == argMapper.size());
	unsigned opIndex = argMapper[argMapper.size() - 1];
	assert(opIndex < vecOps.size());
	operator_ *op = vecOps[opIndex];
	assert(op);
	unsigned nArgs = op->parameters->size();
	VecInt result(nArgs + 1, UVAL);
	result[0] = opIndex;
	for (unsigned i = 0; i < argMapper.size() - 1; i++) {
		assert(result.size() == nArgs+1);
		if ((argMapper[i] == UVAL) ^ (observation[i + 1] == UVAL))
			return VecInt();
		if (argMapper[i] < 0) {
			if (argMapper[i] != observation[i + 1])
				return VecInt(); // If one of the arguments in the observation is a constant, it must match
		} else if (argMapper[i] != UVAL) {
			assert(i < observation.size());
			assert(argMapper[i]+1 < (int)result.size());
			//assert (observation[i+1] < (int)nArgs);
			result[argMapper[i] + 1] = observation[i + 1];
		}
	}
	return result;
}

string Processor::getObjectName(int typeId, int objectId) {
	assert(typeId < (int)objectTblByTypeId.size());
	assert(objectId < (int)objectTblByTypeId[typeId].size());
	return objectTblByTypeId[typeId][objectId];
}

string Processor::getDomainName() const {
	return domainName;
}

void Processor::setDomainName(string domainName) {
	this->domainName = domainName;
}

int Processor::getFunctionId(const string functionName) {
	return this->funcHeadTbl[functionName];
}

string Processor::getFormattedAction(int actionId) {
	return formatter->actionString(actionId);
}

string Processor::getFormattedState(const VecInt& gameHistory, WorldState& ws, const VecVecVecKey& kb) {
	std::ostringstream os;
//	os << "Game History: " << this->getHistory(gameHistory) << "\n";
	os << formatter->asString(gameHistory, ws, kb);
	return os.str();
}

NumScalar Processor::getEstimatedValue(const VecInt& gameHistory, WorldState& ws, const VecVecVecKey& kb) {
	return this->evaluator->getEstimatedValue(gameHistory, ws, kb);
}

void Processor::setFormatter(WorldStateFormatter *formatter) {
	assert(formatter != NULL);
	this->formatter = formatter;
}

}
;
// end namespace
//cout << "Common add effects\n";
//for (SetInt::const_iterator itr = addEffectsInter.begin(); itr != addEffectsInter.end(); ++itr) {
//    cout << "    " << getFact(*itr) << "\n";
//}
//cout << "Common del effects\n";
//for (SetInt::const_iterator itr = delEffectsInter.begin(); itr != delEffectsInter.end(); ++itr) {
//    cout << "    " << getFact(*itr) << "\n";
//}
//cout << "Possible add effects\n";
//for (SetInt::const_iterator itr = addSymDiff.begin(); itr != addSymDiff.end(); ++itr) {
//    cout << "    " << getFact(*itr) << "\n";
//}
//cout << "Possible del effects\n";
//for (SetInt::const_iterator itr = delSymDiff.begin(); itr != delSymDiff.end(); ++itr) {
//    cout << "    " << getFact(*itr) << "\n";
//}
