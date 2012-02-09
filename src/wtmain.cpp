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
#include <Wt/WTextArea>
#include <Wt/WText>
#include <Wt/WTabWidget>
#include "GameLogGenerator.h"
#include "processor.h"
#include <string>
#include <boost/version.hpp>

char* current_filename; // This is only necessary because parse_error.h declares it extern; can be worked around

using namespace Wt;
using namespace VAL;

/*
 * A simple hello world application class which demonstrates how to react
 * to events, read input, and give feed-back.
 */
class HelloApplication: public WApplication {
public:
	HelloApplication(const WEnvironment& env);

private:
	WLineEdit *nameEdit_;
	WText *greeting_;
	WTextArea *area_;
	WComboBox* domain;
	WComboBox* instance;
	WContainerWidget* selectors;
	WTabWidget* tabs;
	WTextArea *bits;
	WTextArea *formatted;
	WTextArea *history;
	WPushButton* start;

	// To be encapsulated
	GameLogGenerator glg;
	Processor* p;
	VecPlayerType vpt;
	WorldState current;
	VecInt gameHistory;
	VecVecVecKey playerObs;

	void addSelector();
	void greet();
	void startGame();
	void continuePlay();

};

/*
 * The env argument contains information about the new session, and
 * the initial request. It must be passed to the WApplication
 * constructor so it is typically also an argument for your custom
 * application constructor.
 */
HelloApplication::HelloApplication(const WEnvironment& env) :
		WApplication(env) {
	setTitle("General Game Player"); // application title

//	root()->addWidget(new WText("Your name, please ? ")); // show some text
	nameEdit_ = new WLineEdit(root()); // allow text input
//	nameEdit_->setFocus(); // give focus
//
//	WPushButton *b = new WPushButton("Greet me.", root()); // create a button
//	b->setMargin(5, Left); // add 5 pixels margin
	start = new WPushButton("Start", root());
	start->setFocus();

//	start->setMargin(5, Left);

	root()->addWidget(new WBreak()); // insert a line break

	selectors = new WContainerWidget(root());
	domain = new WComboBox(selectors);
	domain->addItem("Battleship");
	domain->addItem("Racko");
	domain->addItem("End Game");
	domain->changed().connect(this, &HelloApplication::addSelector);
	//domain->activated().connect(this, &HelloApplication::addSelector);
	instance = new WComboBox(selectors);
	this->addSelector();

	root()->addWidget(new WBreak()); // insert a line break
	tabs = new WTabWidget(root());
	bits = new WTextArea();
	bits->setColumns(100);
	bits->setRows(45);
	formatted = new WTextArea();
	formatted->setColumns(100);
	formatted->setRows(45);
	history = new WTextArea();
	history->setColumns(100);
	history->setRows(45);

	tabs->addTab(bits, "Bits");
	tabs->addTab(formatted, "State");
	tabs->addTab(history,"History");
	tabs->setStyleClass("tabwidget");
	root()->addWidget(new WBreak()); // insert a line break

	area_ = new WTextArea(root());

	root()->addWidget(new WBreak());

	greeting_ = new WText(root()); // empty text

	/*
	 * Connect signals with slots
	 *
	 * - simple Wt-way
	 */
//	b->clicked().connect(this, &HelloApplication::greet);
	start->clicked().connect(this, &HelloApplication::startGame);

	/*
	 * - using an arbitrary function object (binding values with boost::bind())
	 */
	nameEdit_->enterPressed().connect(boost::bind(&HelloApplication::greet, this));
}

void HelloApplication::addSelector() {
	instance->clear();
	if (domain->currentText() == "Battleship") {
		instance->addItem("b11");
		instance->addItem("b12");
	} else if (domain->currentText() == "Racko") {
		instance->addItem("r11");
		instance->addItem("r12");
	} else if (domain->currentText() == "End Game") {
		instance->addItem("e11");
		instance->addItem("e12");
	} else {
		instance->addItem("default");
	}

}

void HelloApplication::startGame() {
	//Processor::checkPayoffs = false; glg.generateGames(string("../logs/EndGame/endgame"), string("../domains/EndGame.pog"), string("../problems/EndGame/endgame-1.pog"), 10);
	glg.logString = "defaultLog";
	std::string domain = "../domains/EndGame.pog";
	std::string instance = "../problems/EndGame/endgame-1.pog";
	analysis* an_analysis = GameParser::parseGame(domain, instance);
	this->p = new VAL::Processor(an_analysis);
	this->vpt = VecPlayerType(p->initialWorld.getNRoles());
	//vpt[0] = P_HUMAN;
	vpt[0] = P_RANDOM; // Chance player
	vpt[1] = P_RANDOM;
	if (vpt.size() == 3) {
		vpt[2] = P_RANDOM;
	}
	this->current = this->p->initialWorld;
	this->playerObs = VecVecVecKey(p->initialWorld.getNRoles());
	continuePlay();



}

void HelloApplication::greet() {
	/*
	 * Update the text, using text input into the nameEdit_ field.
	 */
	greeting_->setText("You have been greeted, " + nameEdit_->text());
	area_->setText(domain->currentText() + " " + instance->currentText());
}

WApplication *createApplication(const WEnvironment& env) {
	/*
	 * You could read information from the environment to decide whether
	 * the user has permission to start a new application
	 */
	WApplication* app = new HelloApplication(env);
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

inline void HelloApplication::continuePlay()
{
	VecInt canDo = this->p->legalOperators(current);
	// First check to see if game is over
	if (canDo.empty() || this->p->alwaysCheckPayoffs()) {
		this->p->computePayoffs(current);
		int sumPayoffs = this->p->sumPayoffs();
		if (sumPayoffs > 0 || canDo.empty()) {
			// Game over; Display final info
//
//				cout << "Payoffs: " << this->p->asString(this->p->payoffs) << "\n";
//			if (pverbose)
//				cout
//						<< "FinalState*********************************************************************************\n"
//						<< this->p->printState(current) << "\n";
//			gameLogger.close();
			//return this->p->payoffs;
		}
	}
	// update tabs
	this->bits->setText(this->p->printState(current));
	this->history->setText(this->p->getHistory(this->gameHistory));
	//this->formatted->setText(this->p->getFormattedState(this->gameHistory,current,this->p->kb));

}


