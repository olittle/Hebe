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
#include <time.h>
#include <pthread.h>
#include <gsl/gsl_rng.h>
#include "linelib.h"

char file_path[MAX_STRING], output_file[MAX_STRING];
int binary = 0, num_threads = 1, vector_size = 100, negative = 5;
long long samples = 1, edge_count_actual;
real alpha = 0.025, starting_alpha;

const gsl_rng_type * gsl_T;
gsl_rng * gsl_r;
ransampl_ws* ws;

// Original GTE data
// line_node word0, word1, doc0, label0;
// line_link link_ww, link_dw, link_lw;

// DBLP data
// line_node paper, term, author, venue;
// line_link link_pt, link_pa, link_pv;

// Line simulation
line_node entity0, entity1;
line_link link_ee;


void *TrainModelThread(void *id) {
    long long edge_count = 0, last_edge_count = 0;
    unsigned long long next_random = (long long)id;
    real *error_vec = (real *)calloc(vector_size, sizeof(real));
    

    while (1)
    {
        // if ((edge_count + 1) % 10000 == 0)
        //     printf("%d\n", edge_count);

        //judge for exit
        if (edge_count > samples / num_threads + 2) break;
        
        if (edge_count - last_edge_count>10000)
        {
            edge_count_actual += edge_count - last_edge_count;
            last_edge_count = edge_count;
            printf("%cAlpha: %f Progress: %.3lf%%", 13, alpha, (real)edge_count_actual / (real)(samples + 1) * 100);
            fflush(stdout);
            alpha = starting_alpha * (1 - edge_count_actual / (real)(samples + 1));
            if (alpha < starting_alpha * 0.0001) alpha = starting_alpha * 0.0001;
        }
        
        // link_ww.train_sample(error_vec, alpha, gsl_rng_uniform(gsl_r), gsl_rng_uniform(gsl_r), next_random);
        // link_dw.train_sample(error_vec, alpha, gsl_rng_uniform(gsl_r), gsl_rng_uniform(gsl_r), next_random);
        // link_lw.train_sample(error_vec, alpha, gsl_rng_uniform(gsl_r), gsl_rng_uniform(gsl_r), next_random);


        // DBLP data
        // link_pt.train_sample(error_vec, alpha, gsl_rng_uniform(gsl_r), gsl_rng_uniform(gsl_r), next_random);
        // link_pa.train_sample(error_vec, alpha, gsl_rng_uniform(gsl_r), gsl_rng_uniform(gsl_r), next_random);
        // link_pv.train_sample(error_vec, alpha, gsl_rng_uniform(gsl_r), gsl_rng_uniform(gsl_r), next_random);

        link_ee.train_sample(error_vec, alpha, gsl_rng_uniform(gsl_r), gsl_rng_uniform(gsl_r), next_random);    
        
        edge_count++;
    }
    free(error_vec);
    pthread_exit(NULL);
}

void TrainModel() {
    long a, b;
    FILE *fo;
    pthread_t *pt = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    starting_alpha = alpha;
    char file_name[MAX_STRING];
    
    // ====== Original GTE ========
    // sprintf(file_name, "%sword.set", file_path);
    // word0.init(file_name, vector_size);
    // sprintf(file_name, "%sword.set", file_path);
    // word1.init(file_name, vector_size);
    // sprintf(file_name, "%sdoc.set", file_path);
    // doc0.init(file_name, vector_size);
    // sprintf(file_name, "%slabel.set", file_path);
    // label0.init(file_name, vector_size);
    
    // sprintf(file_name, "%sww.net", file_path);
    // link_ww.init(file_name, &word0, &word1, negative);
    // sprintf(file_name, "%sdw.net", file_path);
    // link_dw.init(file_name, &doc0, &word1, negative);
    // sprintf(file_name, "%slw.net", file_path);
    // link_lw.init(file_name, &label0, &word1, negative);

    // ====== DBLP ========
    // sprintf(file_name, "%spaper.set", file_path);
    // paper.init(file_name, vector_size);
    // sprintf(file_name, "%sterm.set", file_path);
    // term.init(file_name, vector_size);
    // sprintf(file_name, "%svenue.set", file_path);
    // venue.init(file_name, vector_size);
    // sprintf(file_name, "%sauthor.set", file_path);
    // author.init(file_name, vector_size);
    
    // sprintf(file_name, "%spaper_term.net", file_path);
    // link_pt.init(file_name, &paper, &term, negative);
    // sprintf(file_name, "%spaper_author.net", file_path);
    // link_pa.init(file_name, &paper, &author, negative);
    // sprintf(file_name, "%spaper_venue.net", file_path);
    // link_pv.init(file_name, &paper, &venue, negative);

    //====== LINE simulation =======
    sprintf(file_name, "%sentity.set", file_path);
    entity0.init(file_name, vector_size);
    sprintf(file_name, "%sentity.set", file_path);
    entity1.init(file_name, vector_size);
    
    sprintf(file_name, "%sentity_entity.net", file_path);
    link_ee.init(file_name, &entity0, &entity1, negative);
   
    gsl_rng_env_setup();
    gsl_T = gsl_rng_rand48;
    gsl_r = gsl_rng_alloc(gsl_T);
    gsl_rng_set(gsl_r, 314159265);
    
    clock_t start = clock();
    printf("Training process:\n");
    printf("Number of threads: %d\n", num_threads);
    for (a = 0; a < num_threads; a++) pthread_create(&pt[a], NULL, TrainModelThread, (void *)a);
    for (a = 0; a < num_threads; a++) pthread_join(pt[a], NULL);
    printf("\n");
    clock_t finish = clock();
    printf("Total time: %lf\n", (double)(finish - start) / CLOCKS_PER_SEC);
    
    entity0.output(output_file, binary);
}

int ArgPos(char *str, int argc, char **argv) {
    int a;
    for (a = 1; a < argc; a++) if (!strcmp(str, argv[a])) {
        if (a == argc - 1) {
            printf("Argument missing for %s\n", str);
            exit(1);
        }
        return a;
    }
    return -1;
}

int main(int argc, char **argv) {
    //dblp input
    //./pte -path ../preprocess/ -output ./pte_vec.txt -binary 0 -size 200 -negative 5 -samples 100 -threads 20

    int i;
    if (argc == 1) {
        printf("LINE: Large Information Network Embedding toolkit v 0.1b\n\n");
        printf("Options:\n");
        printf("Parameters for training:\n");
        printf("\t-train <file>\n");
        printf("\t\tUse network data from <file> to train the model\n");
        printf("\t-output <file>\n");
        printf("\t\tUse <file> to save the resulting vectors\n");
        printf("\t-debug <int>\n");
        printf("\t\tSet the debug mode (default = 2 = more info during training)\n");
        printf("\t-binary <int>\n");
        printf("\t\tSave the resulting vectors in binary moded; default is 0 (off)\n");
        printf("\t-size <int>\n");
        printf("\t\tSet size of word vectors; default is 100\n");
        printf("\t-order <int>\n");
        printf("\t\tThe type of the model; 1 for first order, 2 for second order; default is 2\n");
        printf("\t-negative <int>\n");
        printf("\t\tNumber of negative examples; default is 5, common values are 5 - 10 (0 = not used)\n");
        printf("\t-samples <int>\n");
        printf("\t\tSet the number of training samples as <int>Million\n");
        printf("\t-threads <int>\n");
        printf("\t\tUse <int> threads (default 1)\n");
        printf("\t-alpha <float>\n");
        printf("\t\tSet the starting learning rate; default is 0.025\n");
        printf("\nExamples:\n");
        printf("./line -train net.txt -output vec.txt -debug 2 -binary 1 -size 200 -order 2 -negative 5 -samples 100\n\n");
        return 0;
    }
    output_file[0] = 0;
    if ((i = ArgPos((char *)"-path", argc, argv)) > 0) strcpy(file_path, argv[i + 1]);
    if ((i = ArgPos((char *)"-output", argc, argv)) > 0) strcpy(output_file, argv[i + 1]);
    if ((i = ArgPos((char *)"-binary", argc, argv)) > 0) binary = atoi(argv[i + 1]);
    if ((i = ArgPos((char *)"-size", argc, argv)) > 0) vector_size = atoi(argv[i + 1]);
    if ((i = ArgPos((char *)"-negative", argc, argv)) > 0) negative = atoi(argv[i + 1]);
    if ((i = ArgPos((char *)"-samples", argc, argv)) > 0) samples = atoi(argv[i + 1])*(long long)(1000000);
    if ((i = ArgPos((char *)"-alpha", argc, argv)) > 0) alpha = atof(argv[i + 1]);
    if ((i = ArgPos((char *)"-threads", argc, argv)) > 0) num_threads = atoi(argv[i + 1]);
    TrainModel();
    return 0;
}
