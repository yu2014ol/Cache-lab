#include <getopt.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cachelab.h"

int64_t **buffer;
int hit_count = 0;
int miss_count = 0;
int eviction_count = 0;
int verbose = 0;

int load(int *count, int set, int E, int tag);
void deal(FILE *file, int s, int E, int b);

int main(int argc, char **argv) {
  int opt;

  int s;  // number of set bits
  int b;  // number of block bits
  int S;  // number of set
  int E;  // number of line per set
          // int C;  // capacity
  char *file_name;
  while ((opt = getopt(argc, argv, "vs:E:b:t:")) != -1) {
    switch (opt) {
      case 'v':
        verbose = 1;
        break;
      case 's':
        S = pow(2, atoi(optarg));
        s = atoi(optarg);
        break;
      case 'E':
        E = atoi(optarg);
        break;
      case 'b':
        b = atoi(optarg);
        break;
      case 't':
        file_name = optarg;
        break;
      default:
        printf("parameter error!");
        exit(1);
    }
  }
  buffer = (int64_t **)malloc(sizeof(int64_t *) * S);
  for (int i = 0; i < S; ++i) {
    buffer[i] = (int64_t *)malloc(sizeof(int64_t) * E);
  }
  for (int i = 0; i < S; ++i) {
    for (int j = 0; j < E; ++j) {
      buffer[i][j] = 0;
    }
  }
  FILE *file = fopen(file_name, "r");
  deal(file, s, E, b);
  fclose(file);
  printSummary(hit_count, miss_count, eviction_count);
  return 0;
}

void deal(FILE *file, int s, int E, int b) {
  char str[50];
  int S = pow(2, s);
  int count[S * E]; // LRU
  // int valid[S * E]; // 有效位
  memset(count, 0, sizeof(count));
  // memset(valid, 0, sizeof(valid));
  int set = 0;
  // int offset;
  int64_t tag = 0;
  const char *msg[30] = {"", "hit", "miss", "miss eviction"};
  while (fgets(str, 100, file) != NULL) {
    int op = 0;
    int64_t num;
    char str_buf[50];
    int ret_val;
    if (str[0] != ' ') {
      continue;
    }
    int i = 1;
    for (; str[i] != '\n'; ++i) {
      str_buf[i - 1] = str[i];
    }
    str_buf[i - 1] = '\0';
    char *token = strtok(str, " ");
    if (token[0] == 'L') {
      op = 1;
    } else if (token[0] == 'S') {
      op = 2;
    } else if (token[0] == 'M') {
      op = 3;
    } else {
      continue;
    }
    token = strtok(NULL, ",");
    num = strtoll(token, NULL, 16);
    set = (S - 1) & (num >> b);
    tag = num >> (s + b);
    if (op == 1 || op == 2) {  // 'L'操作
      ret_val = load(count, set, E, tag);
      if (verbose) {
        printf("%s %s\n", str_buf, msg[ret_val]);
      }
    } else {
      ret_val = load(count, set, E, tag);
      if (verbose) {
        int word_2 = load(count, set, E, tag);
        printf("%s %s %s\n", str_buf, msg[ret_val], msg[word_2]);
      } else {
        load(count, set, E, tag);
      }   
    }
  }
}

int load(int *count, int set, int E, int tag) {
  int max_count = set;  // LRU,最近最少使用

  for (int i = 0; i < E; ++i) {
    count[set + i]++;
    if (count[max_count] < count[set + i]) {  // 找到LRU目标
      max_count = set + i;
    }
  }
  for (int i = 0; i < E; ++i) {
    if ((buffer[set][i] & (~(1L << 63))) == tag) {  // 命中     
      if (buffer[set][i] & (1L << 63)) {
        count[set + i] = 0;
        hit_count++;
        return 1;
      } else {
        miss_count++;
        buffer[set][i] = buffer[set][i] | (1L << 63);  // 有效位置位
        return 2;
      }   
    }
  }
  for (int i = 0; i < E; ++i) {
    if ((buffer[set][i] & (1L << 63)) == 0) {  //未命中，有空行
      miss_count++;
      buffer[set][i] = tag | (1L << 63);
      count[set + i] = 0;
      // buffer[set][i] = buffer[set][i] | 0x01;  // 有效位置位
      return 2;
    }
  }
  miss_count++;
  eviction_count++;  //未命中，无空行
  count[max_count] = 0;
  buffer[set][max_count - set] = tag | (1L << 63);
  return 3;
}
