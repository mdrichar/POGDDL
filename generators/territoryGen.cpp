#include <iostream>
#include <cstdio>
#include <cstdlib>
using namespace std;

int main(int argc, char* argv[])
{
  int perside = 3;
  if (argc == 2) {
    perside = atoi(argv[1]);
  }
  int total = perside*perside;
  cout << "(define (problem terr" << perside << ")\n";
  cout << "(:domain territory)\n";
  cout << "(:objects\n";
  //cout << "	wr br - piece\n";
  cout << "	";
  for (unsigned i = 0; i < total; i++) {
    printf("s%02d ",i);
  }
  cout << " - square\n";
  cout << ")\n";
  cout << "(:init\n";
  cout << "	(whoseturn p1)\n";
  cout << "	(oppof p1 p2)\n";
  cout << "	(oppof p2 p1)\n";
  cout << "	(at p1 s00)\n";
  printf("     (at p2 s%02d)\n",total-1);
  for (unsigned i = 0; i < total; i++) {
    int r1 = i/perside;
    int c1 = i%perside;
    for (unsigned j = i+1; j < total; j++) {
      int r2 = j/perside;
      int c2 = j%perside;
      int deltar = r1 - r2;
      int deltac = c1 - c2;
      if (abs(deltar) <= 1 && abs(deltac) <= 1) {
        printf("     (adjacent s%02d s%02d)\n",i,j);
        printf("     (adjacent s%02d s%02d)\n",j,i);
      }
      //if (deltac == 0) {
      //  printf("     (rel s%02d s%02d north)\n",i,j);
      //  printf("     (rel s%02d s%02d south)\n",j,i);
      //}
      //if (deltar == 0) {
      //  printf("     (rel s%02d s%02d west)\n",i,j);
      //  printf("     (rel s%02d s%02d east)\n",j,i);
      //}
    }
  }
  //cout << "	(at wr s24)\n";
  //cout << "	(claimed p1 s24)\n";
  //cout << "	(first p1)\n";
  printf("     (detectable s%02d)\n",perside/2*perside+perside/2);
  cout << "	(= (nturns) 0)\n";
  cout << "	(= (score p1) 0)\n";
  cout << "	(= (score p2) 0)\n";
  cout << "))\n";
  return 0;
}
