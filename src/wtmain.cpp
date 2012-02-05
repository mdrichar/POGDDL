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
class HelloApplication : public WApplication
{
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
  WTextArea *preds;
  WPushButton* start;
  GameLogGenerator glg;

  void addSelector();


  void greet();
  void startGame();
};

/*
 * The env argument contains information about the new session, and
 * the initial request. It must be passed to the WApplication
 * constructor so it is typically also an argument for your custom
 * application constructor.
*/
HelloApplication::HelloApplication(const WEnvironment& env)
  : WApplication(env)
{
  setTitle("General Game Player");                               // application title

  root()->addWidget(new WText("Your name, please ? "));  // show some text
  nameEdit_ = new WLineEdit(root());                     // allow text input
  nameEdit_->setFocus();                                 // give focus


  WPushButton *b = new WPushButton("Greet me.", root()); // create a button
  b->setMargin(5, Left);                                 // add 5 pixels margin
  start = new WPushButton("Start",root());
  start->setMargin(5,Left);

  root()->addWidget(new WBreak());                       // insert a line break

  selectors = new WContainerWidget(root());
  domain = new WComboBox(selectors);
  domain->addItem("Battleship");
  domain->addItem("Racko");
  domain->addItem("End Game");
  domain->changed().connect(this, &HelloApplication::addSelector);
  //domain->activated().connect(this, &HelloApplication::addSelector);
  instance = new WComboBox(selectors);
  this->addSelector();

  root()->addWidget(new WBreak());                       // insert a line break
  tabs = new WTabWidget(root());
  bits = new WTextArea();
  bits->setColumns(20);
  bits->setRows(10);
  preds = new WTextArea();
  preds->setColumns(10);
  preds->setRows(20);
  tabs->addTab(bits,"Bits");
  tabs->addTab(preds,"Preds");
  root()->addWidget(new WBreak());                       // insert a line break


  area_ = new WTextArea(root());

  root()->addWidget(new WBreak());

  greeting_ = new WText(root());                         // empty text

  /*
   * Connect signals with slots
   *
   * - simple Wt-way
   */
  b->clicked().connect(this, &HelloApplication::greet);
  start->clicked().connect(this, &HelloApplication::startGame);

  /*
   * - using an arbitrary function object (binding values with boost::bind())
   */
  nameEdit_->enterPressed().connect
    (boost::bind(&HelloApplication::greet, this));
}

void HelloApplication::addSelector()
{
  instance->clear();
  if (domain->currentText() == "Battleship") {
	  instance->addItem("b11");
	  instance->addItem("b12");
  } else if (domain->currentText() == "Racko") {
	  instance->addItem("r11");
	  instance->addItem("r12");
  } else if (domain->currentText() == "End Game"){
	  instance->addItem("e11");
	  instance->addItem("e12");
  } else {
	  instance->addItem("default");
  }


}

void HelloApplication::startGame()
{
	Processor::checkPayoffs = false; glg.generateGames(string("../logs/EndGame/endgame"), string("../domains/EndGame.pog"), string("../problems/EndGame/endgame-1.pog"), 10);


}

void HelloApplication::greet()
{
  /*
   * Update the text, using text input into the nameEdit_ field.
   */
  greeting_->setText("You have been greeted, " + nameEdit_->text());
  area_->setText(domain->currentText() + " " + instance->currentText());
}

WApplication *createApplication(const WEnvironment& env)
{
  /*
   * You could read information from the environment to decide whether
   * the user has permission to start a new application
   */
  return new HelloApplication(env);
}

int main(int argc, char **argv)
{
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
  return WRun(argc, argv, &createApplication);
}
