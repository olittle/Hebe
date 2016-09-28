//  Copyright 2013 Google Inc. All Rights Reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

#define MAX_SIGMOID 6
typedef float real;  				// Precision of float numbers


const long long max_size = 2000;         // max length of strings
const long long N = 40;            // number of closest words that will be shown
const long long max_w = 50;              // max length of vocabulary entries

real *sigmoid_table;
const int sigmoid_table_size = 1000;

/* Fastly compute sigmoid function */
void InitSigmoidTable(){
  real x;
  int k;
  sigmoid_table = (real *)malloc((sigmoid_table_size + 1) * sizeof(real));
  for (k = 0; k != sigmoid_table_size; k++)
  {
    x = 2 * MAX_SIGMOID * k / sigmoid_table_size - MAX_SIGMOID;
    sigmoid_table[k] = 1 / (1 + exp(-x));
  }
}

real FastSigmoid(real x)
{
  if (x > MAX_SIGMOID) return 1;
  else if (x < -MAX_SIGMOID) return 0;
  int k = (x + MAX_SIGMOID) * sigmoid_table_size / MAX_SIGMOID / 2;
  return sigmoid_table[k];
}

int ArgPos(char *str, int argc, char **argv) {
  int a;
  for (a = 1; a < argc; a++)
    if (!strcmp(str, argv[a])) {
      if (a == argc - 1) {
        printf("Argument missing for %s\n", str);
        exit(1);
      }
      return a;
    }
  return -1;
}

int main(int argc, char **argv) {
  FILE *f;
  char st1[max_size];
  char file_target[max_size], file_context[max_size], st[100][max_size];
  float dist, len;
  long long targets, contexts, size, a, b, c, cn, bi[100];

  //	char ch;
  float *M_target, *M_context;
  char *vocab_target, *vocab_context;
  //	char word[max_size];
  if (argc < 2) {
    printf(
        "Usage: ./pair_distance  <FILE> \n");
    return 0;
  }

  strcpy(file_target, argv[1]);
  if(argc == 3)
    strcpy(file_context, argv[2]);

  InitSigmoidTable();

  // read target
  f = fopen(file_target, "rb");
  if (f == NULL) {
    printf("Input file not found\n");
    return -1;
  }

  fscanf(f, "%lld", &targets);
  fscanf(f, "%lld", &size);

  printf("targets: %lld, size, %lld\n", targets, size);

  vocab_target = (char *) malloc(
      (long long) targets * max_w * sizeof(char));
  M_target = (float *) malloc(
      (long long) targets * (long long) size * sizeof(float));
  if (M_target == NULL) {
    printf("Cannot allocate memory: %lld MB    %lld  %lld\n",
        (long long) targets * size * sizeof(float) / 1048576, targets, size);
    return -1;
  }

  for (b = 0; b < targets; b++) {
    fscanf(f, "%s", &vocab_target[b * max_w]);
    for (a = 0; a < size; a++)
      fscanf(f, "%f", &M_target[a + b * size]);
    len = 0;
    for (a = 0; a < size; a++)
      len += M_target[a + b * size] * M_target[a + b * size];
    len = sqrt(len);
    for (a = 0; a < size; a++)
      M_target[a + b * size] /= len;
  }

  fclose(f);

  if(argc == 2)
  {
    contexts = targets;
    M_context = M_target;
    vocab_context = vocab_target;  
  }
  else{
    f = fopen(file_context, "rb");
    if (f == NULL) {
      printf("Input file not found\n");
      return -1;
    }

    fscanf(f, "%lld", &contexts);
    fscanf(f, "%lld", &size);

    printf("contexts: %lld, size, %lld\n", contexts, size);

    vocab_context = (char *) malloc(
        (long long) contexts * max_w * sizeof(char));
    M_context = (float *) malloc(
        (long long) contexts * (long long) size * sizeof(float));
    if (M_context == NULL) {
      printf("Cannot allocate memory: %lld MB    %lld  %lld\n",
          (long long) contexts * size * sizeof(float) / 1048576, contexts, size);
      return -1;
    }

    for (b = 0; b < contexts; b++) {
      fscanf(f, "%s", &vocab_context[b * max_w]);
      for (a = 0; a < size; a++)
        fscanf(f, "%f", &M_context[a + b * size]);
      len = 0;
      for (a = 0; a < size; a++)
        len += M_context[a + b * size] * M_context[a + b * size];
      len = sqrt(len);
      for (a = 0; a < size; a++)
        M_context[a + b * size] /= len;

    }
    fclose(f);
  }

  while (1) {
    printf("Enter word or sentence (EXIT to break): ");
    a = 0;
    while (1) {
      st1[a] = fgetc(stdin);
      if ((st1[a] == '\n') || (a >= max_size - 1)) {
        st1[a] = 0;
        break;
      }
      a++;
    }
    if (!strcmp(st1, "EXIT"))
      break;
    cn = 0;
    b = 0;
    c = 0;
    while (1) {
      st[cn][b] = st1[c];
      b++;
      c++;
      st[cn][b] = 0;
      if (st1[c] == 0)
        break;
      if (st1[c] == ' ') {
        cn++;
        b = 0;
        c++;
      }
    }
    cn++;

    a = 0; 
    for (b = 0; b < targets; b++)
      if (!strcmp(&vocab_target[b * max_w], st[a]))
        break;
    if (b == targets)
    {
      b = -1;
      printf("Out of dictionary word! %s \n", st[a]);
      continue;
    }
    bi[0] = b; 
    printf("\nWord: %s  Position: %lld\n", st[a], bi[a]);

    a = 1; 
    for (b = 0; b < contexts; b++)
      if (!strcmp(&vocab_context[b * max_w], st[a]))
        break;
    if (b == contexts)
    {
      b = -1;
      printf("Out of dictionary word!\n");
      continue;
    }
    bi[1] = b; 
    printf("\nWord: %s  Position: %lld\n", st[a], bi[a]);

    dist = 0;
    for (a = 0; a < size; a++)
      dist += M_target[a + bi[0] * size] * M_context[a + bi[1] * size];
    printf("\n %s : %s -> %f\n", st[0], st[1], dist);
  }
  return 0;
}
