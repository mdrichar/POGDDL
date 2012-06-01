#include <iostream>
#include <sys/time.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>
using namespace std;

double  get_clock() {
   struct timeval tv; int ok;
   struct timezone tz;
   ok = gettimeofday(&tv, &tz);
   if (ok<0) { printf("gettimeofday error");  }
   return (tv.tv_sec * 1.0 + tv.tv_usec * 1.0E-6);
}

int main()
{
  int NOPS = 1000;
  double a[NOPS];
  double b[NOPS];
  double c[NOPS];
  for (int i = 0; i < NOPS; i++) {
    a[i] = rand() / 25.6789;
    b[i] = rand() / 327.897;
  }
  double elapsed = -get_clock();
  for (int j = 0; j < 10000; j++) {
    for (int i = 0; i < NOPS; i++) {
      c[i] = sin(b[i]);
    }
  }
  elapsed += get_clock();
  cout << elapsed << endl;
  return 0;
}
