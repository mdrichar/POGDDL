#include <stack>
#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>

using namespace std;

void printStack(stack<int> s)
{
  while (!s.empty()) {
    cout << s.top() << " ";
    s.pop();
  }
  cout << endl;
}

// TODO, check the implications of the top thing on the stack; assume that everything prior to that is ok
bool isSuitable(const stack<int>& s)
{
  switch (s.size()) {
    case 1:
      return s.top() != 2 && s.top() !=4;
    case 3:
      return true;
    case 2: 
      return s.top() != 1 && s.top() != 2;
  }
  return true;
}

bool okargs(vector<int>& args, int ind)
{
  switch (ind) {
    case 1:
      return args[ind] != 2 && args[ind] !=4;
    case 3:
      return true;
    case 2: 
      return args[ind] != 5 && args[ind] != 2;
  }
  return true;

}

int alt()
{
  int maxes[] = {3,4,2};
  vector<int> args(3,-1);
  int ptr = 0;
  while (ptr >= 0) {
    if (ptr >= (int)args.size()) {
      copy(args.begin(),args.end(),ostream_iterator<int>(cout," ")); cout << "\n";
      ptr--;
    } else if (args[ptr] >= maxes[ptr]) {
      args[ptr] = -1;
      ptr--;
    } else {
      args[ptr]++;
      if (okargs(args,ptr)) {
        ptr++;
      }
    }
  }
}

int main()
{
  alt();
  return 0;
  stack<int> s;
  unsigned size = 3; // number of arguments
  int limit = 4; // maximum value for each argument
  s.push(0); // push the first value for the first argument on the stack
  int sp = 0;
  while (!s.empty()) {
    if (!isSuitable(s)) { // advance the value at the current pointer until it's legit or goes over the limit
      do {
        s.top()++;
      } while (s.top() < limit && !isSuitable(s));
    } else if (s.size() == size) { // process solutions
      printStack(s);
    }

    if (s.top() <= limit && s.size() < size) { // advance the pointer and push the first value at the next level onto the stack
      s.push(0);
    } else if (s.top() < limit) { // advance the value at the current pointer
      s.top()++;
    } else {
      s.pop(); // We've maxed out the value at the current level, so we need to go back a level
      if (!s.empty()) s.top()++; // and try advancing there.
    }
  }
       return 0; 

}
