#include <cstdio>
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <cmath>

using namespace std;
int main(int argc, char* argv[])
{
  assert (argc == 2);
  int cardCount = atoi(argv[1]);
  assert(cardCount >= 5);
  //int powerCard = atoi(argv[2]);


  printf("(define (problem difference-%d)\n",cardCount);
  printf("(:domain difference)\n");
  printf("(:objects\n");
  printf("        s11 s12 s21 s22 - slot\n        ");
  for (unsigned cardIndex = 1; cardIndex <= cardCount; cardIndex++) {
    printf("c%02d ",cardIndex);        
  }
  printf("- card\n");
  printf(")\n");
  printf("(:init\n");
  printf("        (oppof p1 p2)\n");
  printf("        (oppof p2 p1)\n");
  printf("        (whoseturn chance)\n");
  printf("        (inphase drawing)\n");
  printf("        (pending p1)\n");
  printf("        (first p1)\n");
  printf("        (empty s12)\n");
  printf("        (empty s22)\n");
  printf("        (owns p1 s11)\n");
  printf("        (owns p1 s12)\n");
  printf("        (owns p2 s21)\n");
  printf("        (owns p2 s22)\n");
  for (unsigned cardIndex = 1; cardIndex <= cardCount - 3; cardIndex++) {
	printf("        (dealer c%02d)\n", cardIndex);
  }
  printf("        (at c%02d s11)\n",cardCount-2);
  printf("        (at c%02d s21)\n",cardCount-1);
  printf("        (center c%02d)\n",cardCount);
  printf("        (= (score p1) 0)\n");
  printf("        (= (score p2) 0)\n");
  for (int cardIndex1 = 1; cardIndex1 < cardCount ; cardIndex1++) {
	  for (int cardIndex2 = cardIndex1 + 1; cardIndex2 <= cardCount ; cardIndex2++) {
		if (cardIndex1 != cardIndex2) {
			printf("        (= (diff c%02d c%02d) %d)\n", cardIndex1, cardIndex2, (int)abs(cardIndex1 - cardIndex2));
			printf("        (= (diff c%02d c%02d) %d)\n", cardIndex2, cardIndex1, (int)abs(cardIndex1 - cardIndex2));
		}
	  }
  }
  printf("))\n");
  return EXIT_SUCCESS;
}
