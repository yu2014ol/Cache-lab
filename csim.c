#include <getopt.h>
#include "cachelab.h"

int main(int argc, char **argv) {
  int hit_count;
  int miss_count;
  int eviction_count;
  int opt;
  char *str = "vs:E:b:t:";
  while (getopt(argc, argv, str) != -1) {
    
  } 

  printSummary(hit_count, miss_count, eviction_count);
  return 0;
}
