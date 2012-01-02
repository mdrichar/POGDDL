#ifndef __NODE_H__
#define __NODE_H__

#include "WorldState.h"
#include "processor.h"

class Node;
typedef std::vector<Node> VecNode;

struct ChildInfo
{
  int id; // index in the node list for this node
  int actionId; // The index of the action to execute at this node in order to reach the child node
  float cumRewards;
  float valueEst;
  ChildInfo() : id(-1),actionId(-1),cumRewards(0.0f) {}
};

class Node {
private:
    //Node(const Node& other);
    //Node& operator=(const Node& other);
public:
    static Processor* gproc; 
    static unsigned nSamples;
    static VecNode* nodes;
    static VecNode& nodeVec();
    static int count;
    static int next_id;
    static int nextId();
    static void list();

    Node();
    ~Node();

// Node info
    WorldState ws; // game state corresponding to this node in the tree
    int id; // index of this node in the Node vector
    int parentId; // index of the parent of this Node
    //int siblingId; // i.e., I am the nth child of my parent
    int depth; // distance from root
    int timesVisited;
    int lastChildVisited;
    bool terminal;
    VecPayoff payoffs;
    std::vector<ChildInfo> children;

    // Game tree properties


   std::string toString() const;

// Queries
   bool isTerminal() const;
   Node& getChild(int i) ;
   WorldState getState() const;
   int mctsChoice() const;

// Ops
    void initializeFromParent(const Node& parent, int id, int actionId);
    void initializeRoot(WorldState& ws);
    void select();
    void simulate();
    void backpropagate(const VecPayoff& payoffs);
    //float simulateBernoulli();
   void unvisitAll();
   void unvisitDescendants();
};

    //void clearChildren();
    //void clear();

// traversals
    //float alphabeta(int depth, float alpha, float beta, int& totalVisits, int& nExpansions, int& maxExpansions, int valueCode = 0);
    //void spawn();
    //void evaluateChildren();
    //void computePowurHeuristic(int& parentCnt, float& parentWins);
    //void orderChildren(SortType st, int depth = -1);
#endif
