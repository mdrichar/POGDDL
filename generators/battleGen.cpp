#include <cstdio>
#include <iostream>
#include <cassert>
#include <cstdlib>

using namespace std;
int main(int argc, char* argv[])
{
  assert (argc == 3);
  int shipCount = atoi(argv[1]);
  int gridPointsPerSide = atoi(argv[2]);


  printf("(define (problem b1)\n");
  printf("(:domain battleship)\n");
  printf("(:objects\n");
  printf("        ");
  for (unsigned gridIndex = 1; gridIndex <= gridPointsPerSide; gridIndex++) {
    printf("col%02d ",gridIndex);        
  }
  printf("- col\n");
  printf("        ");
  for (unsigned gridIndex = 1; gridIndex <= gridPointsPerSide; gridIndex++) {
    printf("row%02d ",gridIndex);        
  }
  printf("- row\n");
  printf("        destroyer cruiser submarine battleship carrier - ship\n");
  printf(")\n");
  printf("(:init\n");
  printf("        (size2 destroyer)\n");
  printf("        (size3 cruiser)\n");
  printf("        (size3 submarine)\n");
  printf("        (size4 battleship)\n");
  printf("        (size5 carrier)\n");
  printf("        (whoseturn p1)\n");
  printf("        (inphase placing)\n");
  printf("        (= (shipssunk p1) 0)\n");
  printf("        (= (shipssunk p2) 0)\n");
  printf("        (= (totalhits p1) 0)\n");
  printf("        (= (totalhits p2) 0)\n");
  printf("        (= (needtosink) %d)\n",shipCount);
  printf("        (= (nhits p1 destroyer) 0)\n");
  printf("        (= (nhits p2 destroyer) 0)\n");
  printf("        (= (nhits p1 cruiser) 0)\n");
  printf("        (= (nhits p2 cruiser) 0)\n");
  printf("        (= (nhits p1 submarine) 0)\n");
  printf("        (= (nhits p2 submarine) 0)\n");
  printf("        (= (nhits p1 battleship) 0)\n");
  printf("        (= (nhits p2 battleship) 0)\n");
  printf("        (= (nhits p1 carrier) 0)\n");
  printf("        (= (nhits p2 carrier) 0)\n");
  printf("        (= (maxhits destroyer) 1)\n");
  printf("        (= (maxhits cruiser) 2)\n");
  printf("        (= (maxhits submarine) 2)\n");
  printf("        (= (maxhits battleship) 3)\n");
  printf("        (= (maxhits carrier) 4)\n");
  printf("        (current p1 destroyer) \n");
  printf("        (next p1 p2 destroyer destroyer)                \n");
  printf("        (next p2 p1 destroyer cruiser)\n");
  printf("        (next p1 p2 cruiser cruiser)\n");
  printf("        (next p2 p1 cruiser submarine) \n");
  printf("        (next p1 p2 submarine submarine)\n");
  printf("        (next p2 p1 submarine battleship)\n");
  printf("        (next p1 p2 battleship battleship)\n");
  printf("        (next p2 p1 battleship carrier)\n");
  printf("        (next p1 p2 carrier carrier)\n");
  printf("        (next p2 p1 carrier destroyer)\n");
  switch (shipCount) {
    case 1: 
	printf("        (last p2 destroyer)\n"); 
	break;
    case 2: 
	printf("        (last p2 cruiser)\n"); 
	break;
    case 3: 
	printf("        (last p2 submarine)\n"); 
	break;
    case 4: 
	printf("        (last p2 battleship)\n"); 
	break;
    case 5: 
	printf("        (last p2 carrier)\n"); 
	break;

  }
  printf("        (opponent p1 p2)\n");
  printf("        (opponent p2 p1)\n");
  for (unsigned gridIndex = 1; gridIndex < gridPointsPerSide; gridIndex++) {
    printf("        (adjacentc col%02d col%02d)\n", gridIndex, gridIndex + 1);
  }
  for (unsigned gridIndex = 1; gridIndex < gridPointsPerSide; gridIndex++) {
    printf("        (adjacentr row%02d row%02d)\n", gridIndex, gridIndex + 1);
  }
  printf("))\n");
  return EXIT_SUCCESS;
}
