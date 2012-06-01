/*
 * Copyright (C) 2008 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WComboBox>
#include <Wt/WSelectionBox>
#include <Wt/WTextArea>
#include <Wt/WText>
#include <Wt/WTabWidget>
#include "GameLogGenerator.h"
#include "processor.h"
#include <string>
#include <iostream>
#include <boost/version.hpp>
#include <algorithm>

#include "WorldStateFormatter.h"
#include "FormatterFactory.h"
#include "StaticEvaluationFunctionFactory.h"
#include "StaticEvaluationFunction.h"
#include "ActionGraph.h"
#include "GameModerator.h"
#include "Utilities.h"
#include "Node.h"

char* current_filename; // This is only necessary because parse_error.h declares it extern; can be worked around

using namespace Wt;
using namespace VAL;

/*
 * A simple hello world application class which demonstrates how to react
 * to events, read input, and give feed-back.
 */
class PoisomGui: public WApplication {
public:
	PoisomGui(const WEnvironment& env);

private:
	//WLineEdit *moveChooser;
	WText *greeting_;
	WTextArea *area_;
	WComboBox* domain;
	WComboBox* instance;
	WComboBox* player0;
	WComboBox* player1;
	WComboBox* player2;
	WLineEdit* playerId;
	WLineEdit* randomSeedForGame;
	WContainerWidget* selectors;
	WContainerWidget* humanSelector;
	WTabWidget* tabs;
	WContainerWidget* setup;
	WContainerWidget* replay;
	WTextArea *rules; // Display the domain file for the game
	WTextArea *init; // Display the initial conditions for the game
	WTextArea *rawState; // Display game state as 1s and 0s
	WTextArea *formattedState; // Display game state in human readable format
	WTextArea *history; // Display list of actions taken so far in the game
	WTextArea *replayArea;
	WSelectionBox *options; // Display legal moves
	WPushButton* play;
	WPushButton* start;
	WLineEdit* logFilenameWidget;

	WPushButton* forward;
	WPushButton* backward;
	WText* replayIndexText;

	VecNode actualNodes;

	// Replay functionality
	int replayIndex;
	string replayFile;
	int maxReplayIndexLimit;
	WLineEdit* replayFileWidget;
	WPushButton* loadReplayFileButton;
	bool fileHasBeenLoaded;
	VecInt replayHistory;

	// To be encapsulated
	GameLogGenerator glg;
	Processor* p;
	VecPlayerType vpt;
	WorldState current;
	VecInt gameHistory;
	VecVecVecKey playerObs;
	int lastSelected;
	int nSamples;
	int nOppSamples;
	string domainFile;
	string instanceFile;
	string logFile;

	WComboBox* createPlayerComboBox();
	void populateSetupTab();
	void populateReplayTab();
	void updateStateDescriptionWindows();
	void addSelector();
//	void greet();
	void startGame();
	void advanceReplay();
	void loadReplayFile();
	void rewindReplay();
	void continuePlay();
	void makeMove(int move);
	void humanPlayMove();
	void optionSelected();
	void populateRulesTab(const string& domain, const string& instance);
	void writeLog();
	WorldState getNthPosition(Processor* p, unsigned n, const VecInt& history);
	void displayNthPosition(unsigned n);

};

WComboBox* PoisomGui::createPlayerComboBox() {
	WComboBox* comboBox = new WComboBox();
	comboBox->addItem("Random");
	comboBox->addItem("Human");
	comboBox->addItem("MCTS");
	comboBox->addItem("Inference");
	comboBox->addItem("Heuristic");
	return comboBox;
}

void PoisomGui::populateSetupTab() {
	//	root()->addWidget(new WText("Your name, please ? ")); // show some text
	//moveChooser = new WLineEdit(); // allow text input
	//	nameEdit_->setFocus(); // give focus
	//
	//	WPushButton *b = new WPushButton("Greet me.", root()); // create a button
	//	b->setMargin(5, Left); // add 5 pixels margin
	setup = new WContainerWidget();
	setup->addWidget(new WBreak()); // insert a line break
	setup->addWidget(new WBreak()); // insert a line break
	setup->addWidget(new WText("Domain: "));
//    selectors = new WContainerWidget(setup);
	domain = new WComboBox();
	domain->addItem("Battleship");
	domain->addItem("Game of Pure Strategy");
	domain->addItem("Racko");
	domain->changed().connect(this, &PoisomGui::addSelector);
	setup->addWidget(domain);
	setup->addWidget(new WBreak());
	//domain->activated().connect(this, &HelloApplication::addSelector);
	instance = new WComboBox();
	setup->addWidget(new WText("Instance: "));
	setup->addWidget(instance);
	setup->addWidget(new WBreak());

	setup->addWidget(new WText("Player 0: "));
	this->player0 = createPlayerComboBox();
	setup->addWidget(player0);
	setup->addWidget(new WBreak());

	setup->addWidget(new WText("Player 1: "));
	this->player1 = createPlayerComboBox();
	player1->setCurrentIndex(1);
	setup->addWidget(player1);
	setup->addWidget(new WBreak());

	setup->addWidget(new WText("Player 2: "));
	this->player2 = createPlayerComboBox();
	player2->setCurrentIndex(2);
	setup->addWidget(player2);
	setup->addWidget(new WBreak());

	setup->addWidget(new WText("Player ID: "));
	this->playerId = new WLineEdit("12345");
	setup->addWidget(playerId);
	setup->addWidget(new WBreak());

	setup->addWidget(new WText("Log file: "));
	this->logFilenameWidget = new WLineEdit("logger");
	setup->addWidget(logFilenameWidget);
	setup->addWidget(new WBreak);

	setup->addWidget(new WText("Random Seed: "));
	this->randomSeedForGame = new WLineEdit("12345");
	setup->addWidget(randomSeedForGame);
	setup->addWidget(new WBreak());

	this->addSelector();

	play = new WPushButton("Play", setup);
	start = new WPushButton("Start", setup);
	start->setFocus();

}

/*
 * The env argument contains information about the new session, and
 * the initial request. It must be passed to the WApplication
 * constructor so it is typically also an argument for your custom
 * application constructor.
 */
PoisomGui::PoisomGui(const WEnvironment& env) :
		WApplication(env) {
	setTitle("General Game Player"); // application title
//	WText* title = new WText("<center><h3>POISOM General Game Player</h3></center>");
//	root()->addWidget(title);
//	root()->addWidget(new WBreak());
//	root()->addWidget(new WText("Your name, please ? ")); // show some text
	//moveChooser = new WLineEdit(); // allow text input
//	nameEdit_->setFocus(); // give focus
//
//	WPushButton *b = new WPushButton("Greet me.", root()); // create a button
//	b->setMargin(5, Left); // add 5 pixels margin
	populateSetupTab();
	populateReplayTab();

	root()->addWidget(new WBreak()); // insert a line break
	tabs = new WTabWidget(root());

	rules = new WTextArea();
	rules->setColumns(100);
	rules->setRows(45);
	init = new WTextArea();
	init->setColumns(100);
	init->setRows(45);

	rawState = new WTextArea();
	rawState->setColumns(100);
	rawState->setRows(45);
	humanSelector = new WContainerWidget();
	formattedState = new WTextArea(humanSelector);
	formattedState->setColumns(100);
	formattedState->setRows(15);
	humanSelector->addWidget(new WBreak());
	options = new WSelectionBox(humanSelector);
	options->setVerticalSize(20);
	history = new WTextArea();
	history->setColumns(100);
	history->setRows(45);

	tabs->addTab(setup, "Setup");
	tabs->addTab(rules, "Rules");
	tabs->addTab(init, "Initial State");
	tabs->addTab(rawState, "Raw State");
	tabs->addTab(humanSelector, "Formatted State");
	tabs->addTab(history, "Game History");
	tabs->addTab(replay, "Replay");
	tabs->setStyleClass("tabwidget");
	root()->addWidget(new WBreak()); // insert a line break

//	area_ = new WTextArea(root());

//	root()->addWidget(new WBreak());

//	greeting_ = new WText(root()); // empty text

	/*
	 * Connect signals with slots
	 *
	 * - simple Wt-way
	 */
//	b->clicked().connect(this, &HelloApplication::greet);
	start->clicked().connect(this, &PoisomGui::startGame);
	play->clicked().connect(this, &PoisomGui::humanPlayMove);
	forward->clicked().connect(this, &PoisomGui::advanceReplay);
	backward->clicked().connect(this, &PoisomGui::rewindReplay);
	loadReplayFileButton->clicked().connect(this, &PoisomGui::loadReplayFile);
	options->clicked().connect(this, &PoisomGui::optionSelected);

	/*
	 * - using an arbitrary function object (binding values with boost::bind())
	 */
	//moveChooser->enterPressed().connect(boost::bind(&HelloApplication::greet, this));
	this->gameHistory.push_back(-1); // no action at t=0

}

void PoisomGui::addSelector() {
	instance->clear();
	if (domain->currentText() == "Battleship") {
		instance->addItem("battleship-2.4.pog");
		//instance->addItem("b12");
	} else if (domain->currentText() == "Game of Pure Strategy") {
		instance->addItem("gops-4.0.pog");
		//instance->addItem("r12");
	} else if (domain->currentText() == "Racko") {
		instance->addItem("racko-5.20.pog");
		//instance->addItem("e12");
	} else {
		instance->addItem("default");
	}

}

void PoisomGui::startGame() {
	//Processor::checkPayoffs = false; glg.generateGames(string("../logs/EndGame/endgame"), string("../domains/EndGame.pog"), string("../problems/EndGame/endgame-1.pog"), 10);
	glg.logString = "defaultLog";

	int gameIndex = this->domain->currentIndex();
	switch (gameIndex) {
	case 0:
		domainFile = "../domains/LongBattleship.pog";
		instanceFile = "../problems/Battleship/battleship-2.4.pog";
		Processor::checkPayoffs = true;
		break;
	case 1:
		domainFile = "../domains/Gops.pog";
		instanceFile = "../problems/Gops/gops-4.0.pog";
		Processor::checkPayoffs = false;
		break;
	case 2:
		domainFile = "../domains/Racko.pog";
		instanceFile = "../problems/Racko/racko-5.20.pog";
		Processor::checkPayoffs = true;
		break;

	}

	this->populateRulesTab(domainFile, instanceFile);
	analysis* an_analysis = GameParser::parseGame(domainFile, instanceFile);

	this->p = new VAL::Processor(an_analysis);

	WorldStateFormatter* formatter = FormatterFactory::createFormatter(p);
	formatter->setProcessor(p);
	StaticEvaluationFunction* evaluator = StaticEvaluationFunctionFactory::createEvaluator(p);
	evaluator->setProcessor(p);

	this->vpt = VecPlayerType(p->initialWorld.getNRoles());
	//vpt[0] = P_HUMAN;
	vpt[0] = P_RANDOM; // Chance player
	vpt[1] = P_HUMAN;
	if (vpt.size() == 3) {
		vpt[2] = P_MCTS;
		if (this->player2->currentIndex() == 0) {
			vpt[2] = P_RANDOM;
		}
	}
	GameModerator::manySamples = 10;
	GameModerator::fewSamples = 10;
	GameModerator::maxSize = 10;
	nSamples = GameModerator::manySamples;
	nOppSamples = GameModerator::fewSamples;
	this->current = this->p->initialWorld;
	ActionGraph::fluentHistory.resize(1);
	this->playerObs = VecVecVecKey(p->initialWorld.getNRoles());
	this->options->hide();
	srand(2650);

	Node::nodes = &(this->actualNodes);
	Node::nodeVec().resize(10000);
	Node::gproc = p;
	continuePlay();

}

//void HelloApplication::greet() {
//	/*
//	 * Update the text, using text input into the moveChooser field.
//	 */
//	//greeting_->setText("You have been greeted, " + moveChooser->text());
//	area_->setText(domain->currentText() + " " + instance->currentText());
//}

WApplication *createApplication(const WEnvironment& env) {
	/*
	 * You could read information from the environment to decide whether
	 * the user has permission to start a new application
	 */
	WApplication* app = new PoisomGui(env);
	app->setCssTheme("polished");
	app->useStyleSheet("style/everywidget.css");
	app->useStyleSheet("style/combostyle.css");
	app->useStyleSheet("style/dragdrop.css");
	return app;
}

int main(int argc, char **argv) {
	/*
	 * Your main method may set up some shared resources, but should then
	 * start the server application (FastCGI or httpd) that starts listening
	 * for requests, and handles all of the application life cycles.
	 *
	 * The last argument to WRun specifies the function that will instantiate
	 * new application objects. That function is executed when a new user surfs
	 * to the Wt application, and after the library has negotiated browser
	 * support. The function should return a newly instantiated application
	 * object.
	 */
	boost::function<Wt::WApplication* (const Wt::WEnvironment&)> func = &createApplication;
	return WRun(argc, argv, func);
}

void PoisomGui::updateStateDescriptionWindows() {
	VecInt canDo = this->p->legalOperators(current);
	options->clear();
	for (unsigned i = 0; i < canDo.size(); i++) {
		lastSelected = -1;
		options->addItem(this->p->getFormattedAction(canDo[i]));
		int rowsToShow = std::min((unsigned) 20, canDo.size());
		options->setVerticalSize(rowsToShow);
		options->show();
	}
	string legalMoveString = this->p->getFormattedLegalMoves(canDo);
	// update tabs
	this->rawState->setText(this->p->printState(current) + " " + legalMoveString);
//	std::cout << this->p->printState(current) << std::endl;
	this->history->setText(this->p->getHistory(this->gameHistory));
//	std::cout << this->p->getHistory(this->gameHistory) << std::endl;
	this->formattedState->setText(this->p->getFormattedState(this->gameHistory, current, this->p->kb)); // + " " + legalMoveString);
//	std::cout << "FormattedState: " << this->p->getFormattedState(this->gameHistory, current, this->p->kb) << std::endl;
}

void PoisomGui::continuePlay() {
	std::ostringstream os;
	bool gameOver = false;
	VecInt canDo = this->p->legalOperators(current);
	string legalMoveString = this->p->getFormattedLegalMoves(canDo);
	// First check to see if game is over
	if (canDo.empty() || this->p->alwaysCheckPayoffs()) {
		this->p->computePayoffs(current);
		int sumPayoffs = this->p->sumPayoffs();
		if (sumPayoffs > 0 || canDo.empty()) {
			gameOver = true;
			// Game over; Display final info
			os << "Game Over\n";
			os << "Payoffs: " << this->p->asString(this->p->payoffs) << "\n";
			os << this->p->winnerDeclarationString(this->p->payoffs) << "\n";
			writeLog();
//			if (pverbose)
//				cout
//						<< "FinalState*********************************************************************************\n"
//						<< this->p->printState(current) << "\n";
//			gameLogger.close();
			//return this->p->payoffs;
		}
	}
	// update tabs
	updateStateDescriptionWindows();

	if (gameOver) {
		this->formattedState->setText(formattedState->text() + os.str());
	} else {
		int wt = current.getWhoseTurn();
		PlayerType currentPlayerType = this->vpt[wt];
		if (currentPlayerType == P_RANDOM) {
			int randomMoveIndex = rand() % canDo.size();
			makeMove(canDo[randomMoveIndex]);
		} else if (currentPlayerType != P_HUMAN) {
			int chosenMove = GameModerator::chooseMove(currentPlayerType, this->p, canDo, wt, this->p->kb,
					this->p->initialWorld, this->nSamples, this->nOppSamples);
			assert(chosenMove != -1);
			makeMove(chosenMove);
		}
	}
}

void PoisomGui::makeMove(int chosenAction) {
	this->p->apply(chosenAction, current);
	this->p->finalizeApply(current);
	gameHistory.push_back(chosenAction);
//gameLogger.append(chosenAction);
	ActionGraph::fluentHistory.push_back(current.getFluents());
	continuePlay();
}

void PoisomGui::humanPlayMove() {
	int whoseTurn = this->current.getWhoseTurn();
	if (vpt[whoseTurn] == P_HUMAN) {
		//const string humanMoveString = this->moveChooser->text().narrow();
		//int humanMove = atoi(humanMoveString.c_str());
		int humanMove = options->currentIndex();
		VecInt canDo = this->p->legalOperators(current);
		cout << "HPM size: " << canDo.size() << " entered: " << humanMove << " index: " << canDo[humanMove] << " text:"
				<< this->p->operatorIndexToString(canDo[humanMove]) << "\n";
		if (humanMove < (int) canDo.size()) {
			options->hide();
			makeMove(canDo[humanMove]);

		}
	}
}

void PoisomGui::optionSelected() {
//	options->setCurrentIndex(2);
//	const std::set<int>& selected = options->selectedIndexes();
//	cout << "Selected " << selected.size() << ": ";
//	for (std::set<int>::const_iterator itr = selected.begin(); itr != selected.end(); ++itr) {
//		cout << *itr << " ";
//	}
	int newIndex = options->currentIndex();
	if (newIndex == this->lastSelected) {
		cout << "Second click\n";
		this->humanPlayMove();
	} else {
		cout << "Setting: " << options->currentIndex() << "\n";
		this->lastSelected = newIndex;
	}
}

string intToString(int x) {
	std::ostringstream os;
	os << x;
	return os.str();
}

void PoisomGui::advanceReplay() {
	if (this->fileHasBeenLoaded && replayIndex < maxReplayIndexLimit) {
		replayIndex++;
		string newText = intToString(replayIndex);
		this->replayIndexText->setText(intToString(replayIndex));
		displayNthPosition(replayIndex);
	}
}

void PoisomGui::rewindReplay() {
	if (this->fileHasBeenLoaded && replayIndex > 0) {
		replayIndex--;
		this->replayIndexText->setText(intToString(replayIndex));
		displayNthPosition(replayIndex);
	}

}

void PoisomGui::loadReplayFile() {
	if (this->p == NULL) {
		this->replayArea->setText("Null game");
		return;
	}
	WString filenameWString = this->replayFileWidget->text();
	string filenameString = filenameWString.narrow();
	std::ifstream gameLogInFile(filenameString.c_str(), std::ios::in);
	string oneGameString;
	getline(gameLogInFile, oneGameString);
	std::istringstream oneGameStream(oneGameString);
	std::ostringstream os;
	if (!oneGameStream.eof()) {
		unsigned moveCount = 0;
		oneGameStream >> moveCount;
		if (moveCount > 0) {
			this->maxReplayIndexLimit = moveCount;
		}
		this->replayHistory = VecInt(moveCount, -1);
		for (unsigned i = 0; i < moveCount; i++) {
			unsigned chosenAction;
			oneGameStream >> chosenAction;
			this->replayHistory[i] = (int) chosenAction;
		}
		for (unsigned j = 1; j < this->replayHistory.size(); j++) {
			os << p->operatorIndexToString(this->replayHistory[j]) << "\n";
		}
	}
	//this->replayArea->setText(os.str());
	displayNthPosition(0);
	this->fileHasBeenLoaded = true;
}

void PoisomGui::populateRulesTab(const string & domain, const string & instance) {
	string domainLines = Utilities::file_as_string(domain);
//	cout << domainLines << std::endl;
	this->rules->setText(domainLines);
	string initLines = Utilities::file_as_string(instance);
	this->init->setText(initLines);

}

void PoisomGui::populateReplayTab() {
	replayIndex = 0;
	replay = new WContainerWidget;
	this->replayFileWidget = new WLineEdit("battleLog", replay);
	this->loadReplayFileButton = new WPushButton("Load", replay);
	replay->addWidget(new WBreak);
	backward = new WPushButton("<", replay);
	replayIndexText = new WText("0", replay);
	forward = new WPushButton(">", replay);
	replay->addWidget(new WBreak);
	this->fileHasBeenLoaded = false;

	replayArea = new WTextArea(replay);
	replayArea->setColumns(100);
	replayArea->setRows(45);

}

void PoisomGui::writeLog() {
	this->logFile = this->logFilenameWidget->text().narrow();
	std::ofstream gameLogFile(logFile.c_str(), std::ios::out | std::ios::app);
	gameLogFile << gameHistory.size() << " ";
	copy(gameHistory.begin(), gameHistory.end(), std::ostream_iterator<unsigned>(gameLogFile, " "));
	gameLogFile << "\n";
	gameLogFile.close();
}

inline WorldState PoisomGui::getNthPosition(Processor *p, unsigned n, const VecInt & history) {
	WorldState current = this->p->initialWorld;
	ActionGraph::fluentHistory.resize(1);
	VecVecVecKey obs = VecVecVecKey(p->initialWorld.getNRoles());
	this->p->payoffs = VecPayoff(p->initialWorld.getNRoles());
	this->p->kb = obs;
	assert(history.size() > 0);
	assert(n < history.size() - 1);
	for (unsigned i = 1; i <= n; i++) {
		this->p->apply(history[i], current);
		this->p->finalizeApply(current);
		ActionGraph::fluentHistory.push_back(current.getFluents());

	}
	return current;
}

inline void PoisomGui::displayNthPosition(unsigned n) {
	WorldState current = getNthPosition(p, n, replayHistory);
	std::ostringstream os;
	os << this->p->getFormattedState(this->gameHistory, current, this->p->kb) << "\n";
	if (n < replayHistory.size()) {
		os << "Chosen move: " << p->getFormattedAction(replayHistory[n + 1]) << "\n";
	}
	replayArea->setText(os.str());
}

