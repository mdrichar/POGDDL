#include <cstdio>
#include <iostream>
#include <cassert>
#include <cstdlib>

using namespace std;
int main(int argc, char* argv[])
{
  assert (argc == 3);
  int slotCount = atoi(argv[1]);
  int cardCount = atoi(argv[2]);
  printf ("(define (problem racko%d-%d)\n",slotCount,cardCount);
  printf ("(:domain racko)\n"); 
  printf ("(:objects\n     "); 
  for (unsigned slotIndex = 1; slotIndex <= slotCount; slotIndex++) {
    printf("u%02d d%02d ",slotIndex, slotIndex);
  }
  printf (" - slot\n     ");
  for (unsigned cardIndex = 1; cardIndex <= cardCount; cardIndex++) {
    printf("c%02d ",cardIndex);
  }
  printf (" - card)\n");
  printf ("(:init\n");
  printf ("     (inphase dealing)\n");
  printf ("     (current u01)\n");
  printf ("     (currentdrawn c01)\n");
  printf ("     (lastdeal d%02d)\n",slotCount);
  printf ("     (whoseturn chance)\n");
  printf ("     (oppof p1 p2)\n");
  printf ("     (oppof p2 p1)\n");
  for (unsigned cardIndex = 1; cardIndex <= cardCount; cardIndex++) {
    printf("     (at c%02d dealer)\n",cardIndex);
  }
  for (unsigned slotIndex = 1; slotIndex < slotCount; slotIndex++) {
    printf("     (nextdeal d%02d u%02d)\n", slotIndex, slotIndex);
    printf("     (nextdeal u%02d d%02d)\n", slotIndex + 1, slotIndex);
  }
  printf("     (nextdeal d%02d u%02d)\n", slotCount, slotCount);
  printf("     (nextdeal dealer d%02d)\n", slotCount);
  for (unsigned slotIndex = 1; slotIndex <= slotCount; slotIndex++) {
    printf("     (owns p1 u%02d)\n", slotIndex);
    printf("     (owns p2 d%02d)\n", slotIndex);
  }
  for (unsigned cardIndex = 1; cardIndex <= cardCount; cardIndex++) {
    printf("	 (same c%02d c%02d)\n", cardIndex, cardIndex); 
  }
  for (unsigned slotIndex = 1; slotIndex < slotCount; slotIndex++) {
    printf("     (successor-s u%02d u%02d)\n", slotIndex + 1, slotIndex);
    printf("     (successor-s d%02d d%02d)\n", slotIndex + 1, slotIndex);
  }
  for (unsigned cardIndex = 1; cardIndex < cardCount; cardIndex++) {
    printf("     (successor-c c%02d c%02d)\n", cardIndex + 1, cardIndex);
  }
  printf("))\n");
  return EXIT_SUCCESS;
}
  
//(define (problem racko5-20)
//(:domain racko)
//(:objects 
//	u1 u2 u3 u4 u5 d1 d2 d3 d4 d5 - slot
//	c01 c02 c03 c04 c05 c06 c07 c08 c09 c10 c11 c12 c13 c14 c15 c16 c17 c18 c19 c20 c21 c22 c23 c24 c25 - card)
//(:init
//	(at c01 dealer)
//	(at c02 dealer)
//	(at c03 dealer)
//	(at c04 dealer)
//	(at c05 dealer)
//	(at c06 dealer)
//	(at c07 dealer)
//	(at c08 dealer)
//	(at c09 dealer)
//	(at c10 dealer)
//	(at c11 dealer)
//	(at c12 dealer)
//	(at c13 dealer)
//	(at c14 dealer)
//	(at c15 dealer)
//	(at c16 dealer)
//	(at c17 dealer)
//	(at c18 dealer)
//	(at c19 dealer)
//	(at c20 dealer)
//	(at c21 dealer)
//	(at c22 dealer)
//	(at c23 dealer)
//	(at c24 dealer)
//	(at c25 dealer)
//	(inphase dealing)
//	(current u1)
//	(nextdeal u2 u1)
//	(nextdeal u3 u2)
//	(nextdeal u4 u3)
//	(nextdeal u5 u4)
//	(nextdeal d1 u5)
//	(nextdeal d2 d1)
//	(nextdeal d3 d2)
//	(nextdeal d4 d3)
//	(nextdeal d5 d4)
//	(nextdeal dealer d5)
//	(lastdeal d5)
//	(owns max u1)
//	(owns max u2)
//	(owns max u3)
//	(owns max u4)
//	(owns max u5)
//	(owns min d1)
//	(owns min d2)
//	(owns min d3)
//	(owns min d4)
//	(owns min d5)
//	(oppof min max)
//	(oppof max min)
//	(whoseturn chance)
//	(successor-s u2 u1)
//	(successor-s u3 u2)
//	(successor-s u4 u3)
//	(successor-s u5 u4)
//	(successor-s d2 d1)
//	(successor-s d3 d2)
//	(successor-s d4 d3)
//	(successor-s d5 d4)
//	(successor-c c02 c01)
//	(successor-c c03 c02)
//	(successor-c c04 c03)
//	(successor-c c05 c04)
//	(successor-c c06 c05)
//	(successor-c c07 c06)
//	(successor-c c08 c07)
//	(successor-c c09 c08)
//	(successor-c c10 c09)
//	(successor-c c11 c10)
//	(successor-c c12 c11)
//	(successor-c c13 c12)
//	(successor-c c14 c13)
//	(successor-c c15 c14)
//	(successor-c c16 c15)
//	(successor-c c17 c16)
//	(successor-c c18 c17)
//	(successor-c c19 c18)
//	(successor-c c20 c19)
//))
//
