#include <iostream>
#include <cstdio>
#include <cstdlib>
using namespace std;

int main()
{
  cout << "(define (problem k1)\n";
  cout << "(:domain kriegspiel)\n";
  cout << "(:objects\n";
  cout << "	wr br - piece\n";
  cout << "	";
  for (unsigned i = 0; i < 64; i++) {
    printf("s%02d ",i);
  }
  cout << " - square\n";
  cout << ")\n";
  cout << "(:init\n";
  cout << "	(whoseturn white)\n";
  cout << "	(oppof white black)\n";
  cout << "	(oppof black white)\n";
  for (unsigned i = 0; i < 64; i++) {
    int r1 = i/8;
    int c1 = i%8;
    for (unsigned j = i+1; j < 64; j++) {
      int r2 = j/8;
      int c2 = j%8;
      int deltar = r1 - r2;
      int deltac = c1 - c2;
      if (abs(deltar) <= 1 && abs(deltac) <= 1) {
        printf("     (adjacent s%02d s%02d)\n",i,j);
        printf("     (adjacent s%02d s%02d)\n",j,i);
      }
      if (deltac == 0) {
        printf("     (rel s%02d s%02d north)\n",i,j);
        printf("     (rel s%02d s%02d south)\n",j,i);
      }
      if (deltar == 0) {
        printf("     (rel s%02d s%02d west)\n",i,j);
        printf("     (rel s%02d s%02d east)\n",j,i);
      }
      //if (deltar == deltac) {
      //  printf("     (rel s%02d s%02d east)\n",i,j);
      //  printf("     (rel s%02d s%02d west)\n",j,i);
      //}
    }
  }
  cout << "	(at wr s24)\n";
  cout << "	(claimed white s24)\n";
  cout << "	(first white)\n";
  cout << "	(= (score white) 0)\n";
  cout << "	(= (score black) 0)\n";
  cout << "))\n";
  return 0;
}
