#include "Node.h"
#include <cmath>
#include <sstream>
#include <numeric>
#include <limits>
#include <iomanip>


int Node::count;
int Node::next_id;
unsigned Node::nSamples = 1000;
VecNode* Node::nodes;
Processor* Node::gproc;
const bool nodeverbose = false;
//extern double get_clock();

// utility
float ucb1bonus(int countAll, int countThis)
{
  return (float)sqrt( 2.0*log((double)countAll) / (double)countThis);
}

// Constructors / Destructors / Assignments
Node::Node()
  : id(-1), parentId(-1), depth(0), timesVisited(0), lastChildVisited(-1), terminal(false)
{
  //depth = -1;
}

Node::~Node()
{
}

// I/O

std::string Node::toString() const
{
  std::ostringstream os;
  os << "id: " << id 
     << " pid: " << parentId
     << " tv: " << timesVisited
     << " lcv: " << lastChildVisited 
     << " trm: " << terminal 
     << "\n";
  return os.str();
}


VecNode& Node::nodeVec()
{
  return *nodes;
}

void Node::list()
{
  for (unsigned i = 0; i < nodeVec().size(); i++) {
    std::cout << nodeVec()[i].toString() << std::endl;
  }
}

int Node::nextId() 
{
  assert (next_id < (int)nodeVec().size());
  // Can't just resize because all the child pointers will be corrupted
  //if (next_id ==  nodeVec().size()) {
  //  Nodes::nodes.resize(nodeVec().size()*2);
  //}
  return next_id++;
}

void Node::unvisitAll()
{
  for (unsigned i = 0; i < nodeVec().size(); i++) {
    nodeVec()[i].timesVisited = 0;
    nodeVec()[i].id = -1;
    nodeVec()[i].parentId = -1;
    nodeVec()[i].children.clear();
  }
  Node::next_id = 0;
}

// Queries
bool Node::isTerminal() const
{
  return this->terminal;
}

WorldState Node::getState() const
{
  return ws; 
}

Node& Node::getChild(int i)
{
  assert (i < (int)children.size());
  assert (i >= 0);
  if (children[i].id == -1) {
    assert (children[i].actionId >= 0);
    assert (children[i].cumRewards == 0.0f);
    int new_id = Node::nextId();
    nodeVec()[new_id].initializeFromParent(*this,new_id,children[i].actionId);
    children[i].id = new_id;
  }
  assert (children[i].id < (int)nodeVec().size());
  return nodeVec()[children[i].id];
} 

void Node::initializeRoot(WorldState& ws)
{
  unvisitAll();
  this->id = Node::nextId();
  assert (this->id == 0); // Kludge.  For convenience, I want to assume later that the root is always at 0
  this->parentId = -1;
  this->depth = 0;
  this->ws = ws;
  this->timesVisited = 0;
  VecInt legalOps = gproc->legalOperators(ws); 
  if (legalOps.empty() || gproc->alwaysCheckPayoffs()) {
    gproc->computePayoffs(ws);
    int sum = gproc->sumPayoffs();; 
    if (sum > 0 || legalOps.empty()) {
      this->payoffs = gproc->payoffs;
      this->terminal = true;
    } else {
      this->payoffs.clear();
      this->terminal = false;
    } 
  } else {
    this->terminal = false;
    this->payoffs.clear();
  } 
  if (!this->terminal) {
    // implied: this->terminal = false;
    children.clear();
    children.resize(legalOps.size());
    for (unsigned i = 0; i < legalOps.size(); i++) {
      children[i].actionId = legalOps[i]; 
      children[i].valueEst = 0.0f;
      children[i].cumRewards= 0.0f;
    }
  }
  this->lastChildVisited = -1;
}

void Node::initializeFromParent(const Node& parent, int id, int actionId)
{
  //clear();
  this->id = id;
  this->parentId = parent.id;
  this->depth = parent.depth + 1;
  this->timesVisited = 0;
  this->lastChildVisited = -1;
  // Now get the new game state associated with this node
  ws = parent.ws;
  VecVecKey vvk = gproc->fullApply(actionId,this->ws);
  VecInt legalOps = gproc->legalOperators(ws); 
  if (legalOps.empty() || gproc->alwaysCheckPayoffs()) {
    gproc->computePayoffs(ws);
    int sum = gproc->sumPayoffs(); 
    if (sum > 0 || legalOps.empty()) {
      this->payoffs = gproc->payoffs;
      this->terminal = true;
    } else {
      this->terminal = false;
      this->payoffs.clear();
    }
  } else {
    this->terminal = false; 
    this->payoffs.clear();
  }
  if (!this->terminal) {
    // implied: this->terminal = false;
    children.clear();
    children.resize(legalOps.size());
    for (unsigned i = 0; i < legalOps.size(); i++) {
      children[i].actionId = legalOps[i]; 
      children[i].valueEst = 0.0f;
      children[i].cumRewards= 0.0f;
    }
  }
}


int Node::mctsChoice() const
{
  //if (this->isTerminal()) return this->getState()->payoff();
  assert (!this->isTerminal());
  assert (!this->children.empty());
  int bestValueEstIndex = -1;
  float bestValueEst = -std::numeric_limits<float>::max(); //this->getChild(0).valueEst;
  if (nodeverbose) {
    for (unsigned i = 0; i < this->children.size(); i++) {
      printf("Child %d %2.3f ", i, this->children[i].valueEst);
      if (children[i].id == -1) {; // This child never got expanded
 	cout << "Unexpanded";	
      }
      cout << " " << gproc->operatorIndexToString(children[i].actionId) << "\n";
    }
  }
  for (unsigned i = 0; i < this->children.size(); i++) {
    if (children[i].id == -1) continue; // This child never got expanded
    if (this->children[i].valueEst > bestValueEst) {
      bestValueEst = this->children[i].valueEst;
      bestValueEstIndex = i;
      cout << "New best(" << bestValueEstIndex << ") " << gproc->operatorIndexToString(children[bestValueEstIndex].actionId) << "\n";
    }
  }
  assert (bestValueEstIndex != -1); // There is no data at this node to decide from 
  cout << "Final choice (" << bestValueEstIndex << ") " << gproc->operatorIndexToString(children[bestValueEstIndex].actionId) << "\n";
  return bestValueEstIndex;
}

void Node::select()
{
  if (nodeverbose) cout << "Select on " << id << "\n";
  if (this->isTerminal()) {
    if (nodeverbose) {
      cout << "Select: " << id << " is terminal\n";
      cout << gproc->printState(this->ws) << "\n";
    }
    //cout << "Choosing unvisited self " << id << endl;
    this->timesVisited++;
    if (parentId != -1) {
      assert(!payoffs.empty());
      assert (this->parentId < (int)nodeVec().size());
      nodeVec()[parentId].backpropagate(payoffs);
    }
    return;
  } 

  if (timesVisited == 0 && parentId != -1) { // this node is just being added to the search tree and is not the root (from which it makes no sense to simulate)
    if (nodeverbose) cout << "Simulating from " << this->id << "\n";
    //double startt = -get_clock();
    this->simulate();
    //startt += get_clock();
    //cout << "Elapsed time: " << startt << "\n";
    //exit(0);
    return;
  }
  assert (!this->children.empty());
  if (ws.getWhoseTurn() == 0) { // this is a chance node, so selection just rotates through all the children in turn
    this->lastChildVisited = (this->lastChildVisited + 1) % this->children.size();
    if (nodeverbose) cout << "Select: chance node (" << this->id << ") visiting child: " << lastChildVisited << "\n";
    this->getChild(lastChildVisited).select();
    return;
  }

  // This is the usual case -- the child to visit is determined by the ucb1 algorithm
  int bestIndex = -1;
  float bestTotal = -std::numeric_limits<float>::max();
  //cout << "First child's value is " << bestTotal << "(" << bestEst << "," << bestBonus << ")" << endl;
  for (unsigned i = 0; i < this->children.size(); i++) {
    if (this->children[i].id == -1) {
      //cout << "Choosing unvisited subsequent child " << this->getChild(i).id << endl;
      lastChildVisited = i;
      if (nodeverbose) cout << "Unvisited child selection " << i << " " << gproc->operatorIndexToString(this->children[i].actionId) << "\n";
      this->getChild(i).select();
      return;
    }
    float comparisonEst = this->children[i].valueEst;
    float comparisonBonus = ucb1bonus(this->timesVisited,this->getChild(i).timesVisited); 
    float comparisonTotal = comparisonEst + comparisonBonus;
    if (comparisonTotal > bestTotal) {
      if (nodeverbose) cout << "		Found improvement: " << i << " (" << comparisonEst << "+" << comparisonBonus << ") is better than " << bestIndex << " (" << bestTotal << ") \n";
      bestIndex = i;
      bestTotal = comparisonTotal;
    } else {
      if (nodeverbose) cout << "		NotFd improvement: " << i << " (" << comparisonEst << "+" << comparisonBonus << ") is not >  than " << bestIndex << " (" << bestTotal << ") \n";
      //cout << "Not upgrading" << endl;
    }
  }
  lastChildVisited = bestIndex;
  if (nodeverbose) cout << "UCB1 selection: " << bestIndex << " " << gproc->operatorIndexToString(this->children[bestIndex].actionId) <<  "\n";
  this->getChild(bestIndex).select();
}

void Node::simulate()
{
  static const bool simulateverbose = false;
  assert (!terminal); // These cases should have been taken care of in select
  assert (timesVisited == 0); // Otherwise, we shouldn't be simming from here
  assert (parentId != -1); // should not be simulating from root
  WorldState current = ws;
  int loops = 0;
  while (true) {
    ++loops;
    VecInt legalOps = gproc->legalOperators(current);
    if (legalOps.empty() || gproc->alwaysCheckPayoffs()) {
    //double startt = -get_clock();
      gproc->computePayoffs(current);
    //startt += VAL::get_clock();
    //cout << "computePayoffs time: " << startt << "\n";
      int sum = gproc->sumPayoffs(); 
      if (sum > 0 || legalOps.empty()) {
         if (nodeverbose) cout << std::setw(7) << id << std::setw(7) << depth << " Reached terminal in simulate. PAYOFFS: " << gproc->asString(gproc->payoffs) << "\n";
	 if (simulateverbose) cout << gproc->printPartialState(current);
         ++timesVisited;
         assert (this->parentId < (int)nodeVec().size());
         nodeVec()[parentId].backpropagate(gproc->payoffs);
         //cout << "Jumping out after " << loops << "\n";
         break;
      }
    }
    int rv = rand();
    int randChoice = rv % legalOps.size();
#ifdef PRANDOM
    cout << "RANDOM: " << rv << " " << " \% by " << legalOps.size() << " = " << randChoice << " ND\n";
#endif
    if (simulateverbose) cout << "Simulate: Applying move " << randChoice << " (" << gproc->operatorIndexToString(legalOps[randChoice]) << ")\n";
    gproc->fullApply(legalOps[randChoice], current); // ignore return value observations
  }
  //if (parentId != -1) {
  //  assert (this->parentId < (int)nodeVec().size());
  //  nodeVec()[parentId].backpropagate(gproc->payoffs);
  //}
}

void Node::backpropagate(const VecPayoff& payoffs)
{
  assert (lastChildVisited >=0 );
  //assert (!isTerminal());
  this->timesVisited++;
  int whoseturnId = ws.getWhoseTurn();
  assert (whoseturnId < (int)payoffs.size());
  if (whoseturnId != 0) {
    if (nodeverbose) cout << "Backpropagating: active player " << whoseturnId << " scores " << payoffs[whoseturnId] << "\n";
    this->children[lastChildVisited].cumRewards += payoffs[whoseturnId];
    this->children[lastChildVisited].valueEst = 
      this->children[lastChildVisited].cumRewards / this->getChild(lastChildVisited).timesVisited;
  }
  if (this->parentId != -1) {  
    assert (this->parentId < (int)nodeVec().size());
    nodeVec()[this->parentId].backpropagate(payoffs);
  } else {
    if (nodeverbose) cout << "BP reached root\n";
  }
}

