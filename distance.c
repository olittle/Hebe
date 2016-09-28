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
const long long N = 20;            // number of closest words that will be shown
const long long max_w = 1000;              // max length of vocabulary entries

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
  char *bestw[max_w];
  char file_context[max_size], file_target[max_size], st[100][max_size];
  float dist, bestd[max_w], vec[max_size];
  float len;
  long long contexts, targets, size, a, b, c, d, cn, bi[100];
  //	char ch;
  float *M_context, *M_target;
  char *vocab_context, *vocab_target;
  //	char word[max_size];

  if (argc < 2) {
    printf(
        "Usage: ./distance  <FILE> [<FILE>] \n");
    return 0;
  }

  strcpy(file_context, argv[1]);

  if(argc > 2)
    strcpy(file_target, argv[2]);

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
        (long long) contexts * size * sizeof(float) / 1048576, contexts,
        size);
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

  if(argc > 2){
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
    for (a = 0; a < max_w; a++)
      bestw[a] = (char *) malloc(max_size * sizeof(char));
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
  }
  else
  {
    for (a = 0; a < max_w; a++)
      bestw[a] = (char *) malloc(max_size * sizeof(char));
    M_target = M_context;
    vocab_target = vocab_context; 
    targets = contexts;  
  }
  
  while (1) {
    for (a = 0; a < N; a++)
      bestd[a] = 0;
    for (a = 0; a < N; a++)
      bestw[a][0] = 0;
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
    for (a = 0; a < cn; a++) {
      for (b = 0; b < contexts; b++)
        if (!strcmp(&vocab_context[b * max_w], st[a]))
          break;
      if (b == contexts)
        b = -1;
      bi[a] = b;
      printf("\nWord: %s  Position: %lld\n", st[a], bi[a]);
      if (b == -1) {
        printf("Out of dictionary word!\n");
        break;
      }
    }
    if (b == -1)
      continue;
    printf(
        "\n                                              Word       Cosine distance\n------------------------------------------------------------------------\n");
    for (a = 0; a < size; a++)
      vec[a] = 0;
    for (b = 0; b < cn; b++) {
      if (bi[b] == -1)
        continue;
      for (a = 0; a < size; a++)
        vec[a] += M_context[a + bi[b] * size];
    }

    for (a = 0; a < N; a++)
      bestd[a] = -100000;
    for (a = 0; a < N; a++)
      bestw[a][0] = 0;

    for (c = 0; c < targets; c++) {
      a = 0;
      dist = 0;
      for (a = 0; a < size; a++)
        dist += vec[a] * M_target[a + c * size];

      for (a = 0; a < N; a++) {
        if (dist > bestd[a]) {
          for (d = N - 1; d > a; d--) {
            bestd[d] = bestd[d - 1];
            strcpy(bestw[d], bestw[d - 1]);
          }
          bestd[a] = dist;
          strcpy(bestw[a], &vocab_target[c * max_w]);
          break;
        }
      }
    }
    for (a = 0; a < N; a++)
      printf("%50s\t\t%f\n", bestw[a], bestd[a]);
  }
  return 0;
}
