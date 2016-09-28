/*
 * tensor2vec.c
 *
 *  Created on: Dec 7th, 2015
 *      Author: Huan Gui, Fangbo Tao
 *  Method: tensor method with pairwise comparison
 *  Extension: load data from existing models
 *  Multiple relation type, each relation type corresponds to one file 
 *  the first line of each file specify the parameters settings 
 */

#include <assert.h>
#include <gsl/gsl_rng.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h> 
#include <unistd.h>

#define MAX_STRING 100
#define MAX_SENTENCE 1000000
#define MAX_SIGMOID 8.0
#define MAX_N_NODE_TYPE 10
#define MAX_N_ENTITY_EVENT 10000 // maxinum entity number of one type in a relation
#define MAX_N_RELATION_TYPE 10      // maximum number of relation types
#define MAX_PARAMETER 100       // maximum number of parameter of each relation type 

typedef double real;          // Precision of float numbers

const int mini_batch = 1;
const int sigmoid_table_size = 2000;
const long hash_size = 30000000;     // For each type of nodes, there are 3M nodes
const int node_count_inc = 1000;
const long neg_table_size = 1e8;
const real sigmoid_coeff = sigmoid_table_size / MAX_SIGMOID / 2.0;
real sample_power = 0.75;  // defined for negative sampling as the power of frequency

struct ClassNode{
  char * name;         // the string for the node
  real * degree;        // (out degree) current node as source node, respecting each type of associated edges, corresponding to each relation types
};

struct ClassHyperEdge{     // defined for directed edge
  long ** ends;
  int * counters;
  real weight;
};

int n_rela_type;      // the number of relations in each schema  
int n_node_type;

int n_node_type_relation[MAX_N_RELATION_TYPE]; 

char parameter_file[MAX_STRING];    // the file specifies all the parameters
char train_network_file[MAX_N_RELATION_TYPE][MAX_STRING];
char input_fix_embedding_file[MAX_N_NODE_TYPE][MAX_STRING];
char output_embedding[MAX_N_NODE_TYPE][MAX_STRING];
char output_folder[MAX_STRING], input_folder[MAX_STRING];

int id_map[MAX_N_RELATION_TYPE][MAX_N_NODE_TYPE];
int scoring[MAX_N_RELATION_TYPE]; 

struct ClassNode* nodes[MAX_N_NODE_TYPE];
struct ClassHyperEdge* edges[MAX_N_RELATION_TYPE];

int num_tensor_threads = 1, num_word2vec_threads = 0, dim = 200;

// create hash table and negative table for each type of nodes and edges, respectively
long *node_hash_table[MAX_N_NODE_TYPE], *neg_table[MAX_N_RELATION_TYPE][MAX_N_NODE_TYPE];

// The maximum size of each types of nodes, and the current count
long max_sz_node[MAX_N_NODE_TYPE], node_cnt[MAX_N_NODE_TYPE];
real rela_weight[MAX_N_RELATION_TYPE]; 

// total samples of edges to update parameters, and the current samples
long total_samples = 15, current_sample, edge_cnt[MAX_N_RELATION_TYPE], weight_edge_cnt;

real init_alpha = 0.025, alpha;

// the embedding of each type of node, relative to different types of nodes
real *emb_node[MAX_N_NODE_TYPE];

real *sigmoid_table;


//specify the types of nodes, and edge names
char node_names[MAX_N_NODE_TYPE][MAX_STRING];
char relation_names[MAX_N_RELATION_TYPE][MAX_STRING];

bool output_types[MAX_N_NODE_TYPE];
bool load_types[MAX_N_NODE_TYPE];
bool valid_types_global[MAX_N_NODE_TYPE]; 

bool valid_types[MAX_N_RELATION_TYPE][MAX_N_NODE_TYPE];
real beta[MAX_N_RELATION_TYPE][MAX_N_NODE_TYPE];
// the gradient_coeff should be different for each relation type 
real gradient_coeff[MAX_N_RELATION_TYPE][MAX_N_NODE_TYPE];

int center_type[MAX_N_RELATION_TYPE];

const gsl_rng_type * gsl_T;
gsl_rng * gsl_r;

clock_t start;

/////////////////////////////////////////////////////
// parameters for word2vec
bool word2vec = false;
char word_network_file[MAX_STRING];  
int *edge_source_id, *edge_target_id; 
double *edge_weight, * word_degree; 
int word_node_type = -1; 
long * word_neg_table;
long num_word_edges;
int word_relation_type = -1; 

real * context_word; 
int word_negative = 5; 
// for edge sampling 
long * word_alias;
double * word_prob; 
/////////////////////////////////////////////////////

// the hash function is the same for all types of nodes
inline unsigned long GetHash(char *input) {
  unsigned long hash = 5381;
  char c;
  while ((c = *input) != '\0') {
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    ++input;
  }
  return hash % hash_size;
}

// initialize the hash table for all types of nodes
void InitHashTable() {
  for (int node_type = 0; node_type < n_node_type; ++node_type) {
    node_hash_table[node_type] = (long *) malloc(hash_size * sizeof(long));
    for (long k = 0; k < hash_size; ++k) node_hash_table[node_type][k] = -1;
  }
}

void InsertHashTable(char *key, int value, int type) {
  unsigned long loc = GetHash(key);
  while (node_hash_table[type][loc] != -1) loc = (loc + 1) % hash_size;
  node_hash_table[type][loc] = value;
}

// hashing based on the name string
long SearchHashTable(char *key, int type) {
  unsigned long addr = GetHash(key);
  while (1) {
    if (node_hash_table[type][addr] == -1) return -1;
    if (!strcmp(key, nodes[type][node_hash_table[type][addr]].name))
      return node_hash_table[type][addr];
    addr = (addr + 1) % hash_size;
  }
  return -1;
}

/* Add a node to the vertex set */
int AddNode(char *name, int node_type) {
  int length = strlen(name) + 1;
  if (length > MAX_STRING) length = MAX_STRING;

  // insert the node, and initial the value of name, node_type, and degree;
  nodes[node_type][node_cnt[node_type]].name = (char *) calloc(length, sizeof(char));
  nodes[node_type][node_cnt[node_type]].degree = (real *) calloc(n_rela_type, sizeof(real)); 

  if (length < MAX_STRING) {
    strcpy(nodes[node_type][node_cnt[node_type]].name, name);
  } else {
    strncpy(nodes[node_type][node_cnt[node_type]].name, name, MAX_STRING - 1);
    nodes[node_type][node_cnt[node_type]].name[MAX_STRING - 1] = '\0';
  }

  ++node_cnt[node_type];
  if (node_cnt[node_type] + 2 >= max_sz_node[node_type]) {
    max_sz_node[node_type] += node_count_inc;
    struct ClassNode *tmp = (struct ClassNode *) realloc(nodes[node_type],
        max_sz_node[node_type] * sizeof(struct ClassNode));
    if (tmp != NULL) {
      nodes[node_type] = tmp;
    } else {
      printf("fail to relocate\n");
    }
  }
  InsertHashTable(name, node_cnt[node_type] - 1, node_type);
  return node_cnt[node_type] - 1;
}

inline real FastSigmoid(real x) {
  if (x > MAX_SIGMOID) return 1;
  else if (x < -MAX_SIGMOID) return 0;
  return sigmoid_table[(int)((x + MAX_SIGMOID) * sigmoid_coeff)];
}

/////////////////////////////////////////////////////
// Read edges of words coocurrence
void ReadWordEdgeData() {
  FILE *fin; 
  char name_v1[MAX_STRING], name_v2[MAX_STRING], str[2 * MAX_STRING + 10000];

  long vid;
  double weight;

  fin = fopen(word_network_file, "rb");
  if (fin == NULL) {
    printf("ERROR: word network file not found!\n");
    exit(1);
  }
  num_word_edges = 0;
  while (fgets(str, sizeof(str), fin)) ++num_word_edges;
  fclose(fin);
  printf("Number of word-word Edges: %ld          %c", num_word_edges, 13);

  edge_source_id = (int *)malloc(num_word_edges * sizeof(int));
  edge_target_id = (int *)malloc(num_word_edges * sizeof(int));
  edge_weight = (double *)malloc(num_word_edges * sizeof(double));
  word_degree = (double *)malloc(200000 * sizeof(double)); 

  if (edge_source_id == NULL || edge_target_id == NULL || edge_weight == NULL) {
    printf("Error: memory allocation failed!\n");
    exit(1);
  }

  fin = fopen(word_network_file, "rb");

  for (long k = 0; k < num_word_edges; ++k) {
    fscanf(fin, "%s %s %lf", name_v1, name_v2, &weight);

    if (k % 500000 == 0) {
      printf("Reading word word edges: %.2lf%%            %c",
          k / (double)(num_word_edges + 1) * 100, 13);
      fflush(stdout);
    }

    vid = SearchHashTable(name_v1, word_node_type);
    if (vid == -1) {
      vid = AddNode(name_v1, word_node_type);
      // printf("Unknown words %s in line %ld ..Exiting...\n", name_v1, k);
    }
    word_degree[vid] += weight; 
    edge_source_id[k] = vid;

    vid = SearchHashTable(name_v2, word_node_type);
    if (vid == -1) {
      vid = AddNode(name_v2, word_node_type);
      // printf("Unknown words %s in line %ld ..Exiting...\n", name_v2, k);
    }
    word_degree[vid] += weight; 
    edge_target_id[k] = vid;
    edge_weight[k] = weight;
  }
  fclose(fin);
  printf("Finish reading word word edges... \t\t\t\t%c", 13);
} 

// negative sampling table 
void InitWordNegTable() {
  real word_sample_power = 0.75;  
  real sum = 0, cur_sum = 0, por = 0;
  long vid = 0;
  printf("Initializing negative table for word network...    %c", 13);
  fflush(stdout);
  word_neg_table = (long *)malloc(neg_table_size * sizeof(long));

  for (long k = 0; k < node_cnt[word_node_type]; ++k) {
    sum += pow(word_degree[k], word_sample_power);
  }
  for (long k = 0; k < neg_table_size; ++k) {
    if ((real)(k + 1) / neg_table_size > por) {
      cur_sum += pow(word_degree[vid], word_sample_power);
      por = cur_sum / sum;
      ++vid;
    }
    word_neg_table[k] = vid - 1;
  }
}

// The alias sampling algorithm, which is used to sample an edge in O(1) time. 
void InitAliasTable() {
  printf("Initializing alias table for word network...   %c", 13);
  fflush(stdout);

  word_alias = (long *)malloc(num_word_edges * sizeof(long));
  word_prob = (double *)malloc(num_word_edges * sizeof(double));
  if (word_alias == NULL || word_prob == NULL) {
    printf("Error: memory allocation failed!\n");
    exit(1);
  }

  double *norm_prob = (double *)malloc(num_word_edges*sizeof(double));
  long *large_block = (long *)malloc(num_word_edges*sizeof(long));
  long *small_block = (long *)malloc(num_word_edges*sizeof(long));
  if (norm_prob == NULL || large_block == NULL || small_block == NULL) {
    printf("Error: memory allocation failed!\n");
    exit(1);
  }

  double sum = 0;
  long cur_small_block, cur_large_block;
  long num_small_block = 0, num_large_block = 0;

  for (long k = 0; k < num_word_edges; ++k) sum += edge_weight[k];
  for (long k = 0; k < num_word_edges; ++k) {
    norm_prob[k] = edge_weight[k] * num_word_edges / sum;
  }

  for (long k = num_word_edges - 1; k >= 0; --k) {
    if (norm_prob[k] < 1) {
      small_block[num_small_block++] = k;
    } else {
      large_block[num_large_block++] = k;
    }
  }

  while (num_small_block && num_large_block) {
    cur_small_block = small_block[--num_small_block];
    cur_large_block = large_block[--num_large_block];
    word_prob[cur_small_block] = norm_prob[cur_small_block];
    word_alias[cur_small_block] = cur_large_block;
    norm_prob[cur_large_block] =
      norm_prob[cur_large_block] + norm_prob[cur_small_block] - 1;
    if (norm_prob[cur_large_block] < 1) {
      small_block[num_small_block++] = cur_large_block;
    } else {
      large_block[num_large_block++] = cur_large_block;
    }
  }

  while (num_large_block) word_prob[large_block[--num_large_block]] = 1;
  while (num_small_block) word_prob[small_block[--num_small_block]] = 1;

  free(norm_prob);
  free(small_block);
  free(large_block);
}

inline long SampleAnEdge_Alias(double rand_value1, double rand_value2) {
  long k = num_word_edges * rand_value1;
  return rand_value2 < word_prob[k] ? k : word_alias[k];
}

inline void UpdateWord(real *vec_u, real *vec_v, real *vec_error, int label, real decay) {
  real x = 0, g;
  for (int k = 0; k < dim; ++k) x += vec_u[k] * vec_v[k];
  g = (label - FastSigmoid(x)) * alpha * gradient_coeff[word_relation_type][word_node_type];
  for (int k = 0; k < dim; ++k) vec_error[k] += g * vec_v[k];
  for (int k = 0; k < dim; ++k) vec_v[k] += g * vec_u[k] - decay * vec_v[k];
}
/////////////////////////////////////////////////////

/* Read edges of different types in the network */
void ReadEdgeData(int relation) {
  FILE *fin;
  char name[MAX_SENTENCE], str[MAX_SENTENCE];  
  real weight;
  long *temp_nodes = (long *)malloc(MAX_N_ENTITY_EVENT * sizeof(long));
  char *token, c = '\0';
  int pos, s; 

  // Get how many number of edges we have
  fin = fopen(train_network_file[relation], "rb");

  printf("input: %s \n", train_network_file[relation]);
  edge_cnt[relation] = 0;
  while (fgets(str, sizeof(str), fin)) ++edge_cnt[relation];
  fclose(fin);
  printf("Number of relations (%s): %ld          \n", relation_names[relation], edge_cnt[relation]);
  // read the edge data
  edges[relation] = (struct ClassHyperEdge *)malloc(edge_cnt[relation] * sizeof(struct ClassHyperEdge));
  for (long i = 0; i < edge_cnt[relation]; ++i) {
    edges[relation][i].ends = (long **) malloc(n_node_type_relation[relation] * sizeof(long *));
    edges[relation][i].counters = (int *) malloc(n_node_type_relation[relation] * sizeof(int));
  }
  if (edges[relation] == NULL) {
    printf("Error: memory allocation for edges failed!\n");
    exit(1);
  }

  fin = fopen(train_network_file[relation], "rb");
  char delim[] = "#";
  int map_node_type;

  for (long k = 0; k < edge_cnt[relation]; ++k) {
    for (int node_type = 0; node_type < n_node_type_relation[relation]; ++node_type) {

      map_node_type = id_map[relation][node_type];  
      pos = 0;
      do { // read one line
        c = fgetc(fin);
        if (c == EOF || c == '\n' || c == '\t') break;
        else if (pos < MAX_SENTENCE - 1) name[pos++] = (char)c;
      } while (true);
      name[pos] = 0;

      s = 0;
      token = strtok(name, delim);
      while (token != NULL) {
        long node_id = SearchHashTable(token, map_node_type);

        if (node_id == -1) {
          node_id = AddNode(token, map_node_type);
        }

        temp_nodes[s++] = node_id;  
        token = strtok(NULL, delim);
      }

      edges[relation][k].counters[map_node_type] = s;
      edges[relation][k].ends[map_node_type] = (long *) malloc(s * sizeof(long));
      for (int i = 0; i < s; ++i) {
        edges[relation][k].ends[map_node_type][i] = temp_nodes[i];
        nodes[map_node_type][edges[relation][k].ends[map_node_type][i]].degree[relation] += fabs(1.0 / s);
      }
    }
    if (c == '\t') {
      fscanf(fin, "%lf", &weight);
      do { // read the rest of the line
        c = fgetc(fin);
      } while (c != EOF && c != '\n');
    } else {
      weight = 1.0;
    }
    assert(weight > 0);
    edges[relation][k].weight = weight;

    if (k % 50000 == 0) {
      printf("Loading edges : %.3lf%%%c", k / (real)(edge_cnt[relation] + 1) * 100, 13);
      fflush(stdout);
    }
  }
  fclose(fin);
  free(temp_nodes);
}

/* Read the fixed embedding of terms */
void ReadFixEmbedding(int node_type) {
  FILE *fin;
  char str[(dim + 1) * MAX_STRING + 1000];
  long node_id;
  long fix_node_count = node_cnt[node_type], count = 0;
  char *token;

  if (access(input_fix_embedding_file[node_type], F_OK) == -1) {
    printf("Can't read the file %s \n", input_fix_embedding_file[node_type]);
    return;  
  }

  // Get how many number of edges we have
  fin = fopen(input_fix_embedding_file[node_type], "rb");

  // printf("debug: finish init\n");
  fgets(str, sizeof(str), fin);
  while (fgets(str, sizeof(str), fin)) {
    // printf("debug: parse %s\n", str);
    if (count >= fix_node_count) break;
    ++count;

    token = strtok(str, " ");
    node_id = SearchHashTable(token, node_type);
    if (node_id == -1) {
      continue;
    }
    for (int k = 0; k < dim; ++k) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        printf("Error: embedding length dismatch!\n");
        exit(1);
      }
      emb_node[node_type][node_id * dim + k] = atof(token) / 20;
    }
  }
  fclose(fin);
  printf("Embedding of node %s imported!\n", node_names[node_type]);
}

inline long SampleAnEdge(int relation, gsl_rng * & gsl_r_local) {
  return gsl_rng_uniform_int(gsl_r_local, edge_cnt[relation]); 
}

void InitNodeVector(int node_type, real *emb_node) {
  long offset;
  int num_valid_types = 0;
  for (int i = 0; i < n_node_type; ++i) {
    if (valid_types_global[i]) ++num_valid_types;
  }
  for (long i = 0; i < node_cnt[node_type]; ++i) {
    offset = i * dim;
    for (int k = 0; k < dim; ++k) {
      emb_node[offset + k] = (gsl_rng_uniform(gsl_r) - 0.5) / dim;
    }
  }
}

/* Initialize the vertex embedding and the context embedding */
void InitVector()
{
  printf("Initializing node vector ...                        %c", 13);
  fflush(stdout);
  // initialize the target and context embedding of each type of node 
  for (int node_type = 0; node_type < n_node_type; ++node_type) {
    posix_memalign((void **)& emb_node[node_type], 128, 
        node_cnt[node_type] * dim * sizeof(real));
    if (emb_node[node_type] == NULL) {
      printf("Error: memory allocation failed\n");
      exit(1);
    }
    InitNodeVector(node_type, emb_node[node_type]); 
  }

  if (word2vec) {
    printf("Initialize context vector ...                     %c", 13); 
    posix_memalign((void **)& context_word, 128,
        node_cnt[word_node_type] * dim * sizeof(real));
    if (context_word == NULL) {
      printf("Error: memory allocation failed\n");
      exit(1);
    }
    InitNodeVector(word_node_type, context_word);
  }
}

/* Sample negative vertex samples according to vertex degrees */
/* Use the negative sample from the dest node type to update the source type */
void InitNegTable(int relation, int node_type) {
  real sum = 0, cur_sum = 0, por = 0;
  long vid = 0;
  printf("Initializing negative table for %s of relation %s...           %c", node_names[node_type], relation_names[relation], 13);
  fflush(stdout);
  neg_table[relation][node_type] = (long *)malloc(neg_table_size * sizeof(long));

  for (long k = 0; k < node_cnt[node_type]; ++k) {
    sum += pow(nodes[node_type][k].degree[relation], sample_power);
  }
  for (long k = 0; k < neg_table_size; ++k) {
    if ((real)(k + 1) / neg_table_size > por) {
      cur_sum += pow(nodes[node_type][vid].degree[relation], sample_power);
      por = cur_sum / sum;
      ++vid;
    }
    neg_table[relation][node_type][k] = vid - 1;
  }
}

/* Fastly compute sigmoid function */
void InitSigmoidTable() {
  real x;
  printf("Initializing sigmoid table ...                     %c", 13);
  fflush(stdout);
  sigmoid_table = (real *)malloc((sigmoid_table_size + 1) * sizeof(real));
  for (int k = 0; k < sigmoid_table_size; ++k) {
    x = 2 * (real)(MAX_SIGMOID * k) / sigmoid_table_size - MAX_SIGMOID;
    sigmoid_table[k] = 1 / (1 + exp(-x));
  }
}

inline long SampleNegativeEdge(int relation, int node_type, gsl_rng * & gsl_r_local) {
  return neg_table[relation][node_type][gsl_rng_uniform_int(gsl_r_local, neg_table_size)]; 
}

// the probability of a node type to be sampled
inline int SampleTarget(int relation, gsl_rng * & gsl_r_local) {
  if (scoring[relation] == 2) return center_type[relation];
  int type = 0;
  do {
    type = gsl_rng_uniform_int(gsl_r_local, n_node_type_relation[relation]);
    type = id_map[relation][type];
  } while (!valid_types[relation][type]);
  return type;
}

inline void ScoreContext(int relation, long * sampled_node_ids, int target_node_type, real *context_score) {
  // score(a,b,c,d) = a'*b + b'*c + b'*d
  long offset;
  if (scoring[relation] == 2) { 
    assert(target_node_type == center_type[relation]);
  }
  memset(context_score, 0, dim * sizeof(real));
  // first deal with word type because of the possible word2vec
  if (target_node_type != word_node_type && sampled_node_ids[word_node_type] >= 0) {
    offset = sampled_node_ids[word_node_type] * dim;
    if (word2vec) {
      for (int k = 0; k < dim; ++k) {
        context_score[k] +=
          emb_node[word_node_type][offset + k] * 0.5 + context_word[offset + k] * 0.5;
      }
    } else {
      for (int k = 0; k < dim; ++k) {
        context_score[k] += emb_node[word_node_type][offset + k];
      }
    }
  }
  for (int i = 0; i < n_node_type; ++i) {
    if (target_node_type != i && sampled_node_ids[i] >= 0 && word_node_type != i) {
      offset = sampled_node_ids[i] * dim;
      for (int k = 0; k < dim; ++k) {
        context_score[k] += emb_node[i][offset + k];
      }
    }
  }
}


inline void UpdateTensor(int relation, long *sampled_node_ids, int target_node_type,
    long neg_node_offset, real *context_score, real *diff_score) {
  real f = 0, g = 0;
  long offset;
  real decay;

  // context_score keeps a term for gradient computation.
  // It is computed from the nodes in the current edge
  // excluding those belongs to the target_node_type
  ScoreContext(relation, sampled_node_ids, target_node_type, context_score);

  offset = sampled_node_ids[target_node_type] * dim;
  for (int k = 0; k < dim; ++k) {
    diff_score[k] = emb_node[target_node_type][offset + k];
  }
  for (int k = 0; k < dim; ++k) {
    diff_score[k] -= emb_node[target_node_type][neg_node_offset + k];
    f += context_score[k] * diff_score[k];
  }
  g = (1 - FastSigmoid(f)) * alpha;
  for (int k = 0; k < dim; ++k) diff_score[k] *= g;
  for (int k = 0; k < dim; ++k) context_score[k] *= g;

  // update words
  // as long word_node_type has nonnegative id, then the field is valid 
  if (target_node_type != word_node_type && sampled_node_ids[word_node_type] >= 0) {
    offset = sampled_node_ids[word_node_type] * dim;
    if (word2vec) {
      decay = beta[relation][word_node_type] * alpha * 0.5;
      for (int k = 0; k < dim; ++k) {
        emb_node[word_node_type][offset + k] +=
          (diff_score[k] * 0.5 - decay * emb_node[word_node_type][offset + k]) * gradient_coeff[relation][word_node_type];
      }
      for (int k = 0; k < dim; ++k) {
        context_word[offset + k] +=
          (diff_score[k] * 0.5 - decay * context_word[offset + k]) * gradient_coeff[relation][word_node_type];
      } 
    } else {
      decay = beta[relation][word_node_type] * alpha;
      for (int k = 0; k < dim; ++k) {
        emb_node[word_node_type][offset + k] +=
          (diff_score[k] - decay * emb_node[word_node_type][offset + k]) * gradient_coeff[relation][word_node_type];
      }
    }
  }

  // updates the sampled nodes of other types (may skip the first dimension, i.e., term)
  for (int other_type = 0; other_type != n_node_type; ++other_type) {
    if (other_type == target_node_type || !valid_types[relation][other_type] ||
        sampled_node_ids[other_type] < 0 || other_type == word_node_type) continue;
    offset = sampled_node_ids[other_type] * dim;
    decay = beta[relation][other_type] * alpha;
    for (int k = 0; k < dim; ++k) {
      emb_node[other_type][offset + k] +=
        (diff_score[k] - decay * emb_node[other_type][offset + k]) * gradient_coeff[relation][other_type];
    }
  }

  // updates positive and negative sampled node
  offset = sampled_node_ids[target_node_type] * dim;
  if (target_node_type == word_node_type && word2vec) {
    decay = beta[relation][word_node_type] * alpha * 0.5;

    for (int k = 0; k < dim; ++k) {
      emb_node[word_node_type][offset + k] +=
        (context_score[k] * 0.5 - decay * emb_node[word_node_type][offset + k]) * gradient_coeff[relation][word_node_type];
    }
    for (int k = 0; k < dim; ++k) {
      context_word[offset + k] +=
        (context_score[k] * 0.5 - decay * context_word[offset + k]) * gradient_coeff[relation][word_node_type];
    } 

  } else{
    decay = beta[relation][target_node_type] * alpha;
    for (int k = 0; k < dim; ++k) {
      // gradient update and shrinkage due to regularization
      emb_node[target_node_type][offset + k] +=
        (context_score[k] - decay * emb_node[target_node_type][offset + k]) * gradient_coeff[relation][target_node_type];
    }
    for (int k = 0; k < dim; ++k) {
      emb_node[target_node_type][neg_node_offset + k] -=
        (context_score[k] + decay * emb_node[target_node_type][neg_node_offset + k]) * gradient_coeff[relation][target_node_type];
    }
  }
}

void *TrainWord2VecThread(void *id) {
  long long u, v, lu, lv, target, curedge;
  real label, decay;

  real *vec_error = (real *)calloc(dim, sizeof(real));

  real *embed_word = emb_node[word_node_type]; 

  gsl_rng * gsl_r_local = gsl_rng_alloc(gsl_T);
  gsl_rng_set(gsl_r_local, (unsigned long int) id); 

  while (1) {
    if (current_sample > total_samples) {
      break;
    }
    curedge = SampleAnEdge_Alias(gsl_rng_uniform(gsl_r), gsl_rng_uniform(gsl_r));
    u = edge_source_id[curedge];
    v = edge_target_id[curedge];

    lu = u * dim;
    memset(vec_error, 0, dim * sizeof(real)); 
    bool direction = gsl_rng_uniform(gsl_r) > 0.5;
    decay = beta[word_relation_type][word_node_type] * alpha * 0.5 * gradient_coeff[word_relation_type][word_node_type];
    // NEGATIVE SAMPLING
    for (int d = 0; d < word_negative + 1; ++d) {
      if (d == 0) {
        target = v;
        label = 1;
      } else {
        target = word_neg_table[gsl_rng_uniform_int(gsl_r_local, neg_table_size)];
        label = 0;
      }
      lv = target * dim;
      if (direction) {
        UpdateWord(&(embed_word[lu]), &(context_word[lv]), vec_error, label, decay);
      } else {
        UpdateWord(&(context_word[lu]), &(embed_word[lv]), vec_error, label, decay);
      }
    }

    if (direction) {
      for (int k = 0; k < dim; ++k) {
        emb_node[word_node_type][k + lu] +=
          vec_error[k] - word_negative * emb_node[word_node_type][k + lu] * decay;
      }
    } else {
      for (int k = 0; k < dim; ++k) {
        context_word[k + lu] +=
          vec_error[k] - word_negative * context_word[k + lu] * decay;
      }
    }
  }
  free(vec_error);
  pthread_exit(NULL);
}

void *TrainTensorThread(void *id) {
  long count = 0, last_count = 0;
  long limit = total_samples / num_tensor_threads + 2;

  long *sampled_node_ids = (long *) malloc(n_node_type * sizeof(long));
  real *context_score = (real *) calloc(dim, sizeof(real));
  real *diff_score = (real *) calloc(dim, sizeof(real));

  int target_node_type;
  int target_node_count;
  long neg_node_id, neg_node_offset;
  ClassHyperEdge curedge;

  gsl_rng * gsl_r_local = gsl_rng_alloc(gsl_T);
  gsl_rng_set(gsl_r_local, (unsigned long int) id); 

  real ratio;
  clock_t now;

  int m; 
  real rand_value; 
  while (1)
  {
    if (count > limit) {
      current_sample += count - last_count; 
      break ;
    }
    if (count - last_count == 10000) {
      current_sample += count - last_count;
      last_count = count;
      ratio = current_sample / (real)(total_samples + 1);
      if (count % 100000 == 0) {
        now=clock();
        printf("%cAlpha: %f  Progress: %.2lf%%  Events/thread/sec: %.2fk",
            13, alpha, ratio * 100,
            current_sample / ((real)(now - start + 1) / (real)CLOCKS_PER_SEC * 1000));
        fflush(stdout);
      }
      alpha = init_alpha * (1 - ratio);
    }
    ++count;

    rand_value = gsl_rng_uniform(gsl_r_local);  
    for (m = 0; m < n_rela_type; m ++) {
      if (rand_value < rela_weight[m]) break; 
      else rand_value -= rela_weight[m]; 
    }
    if(m == n_rela_type)
    {
      printf("invalid type.\n");  
      continue; 
    }

    curedge = edges[m][SampleAnEdge(m, gsl_r_local)];

    for (int i = 0; i < mini_batch; ++i) {
      // sample a type of node as target
      target_node_type = SampleTarget(m, gsl_r_local);

      target_node_count = curedge.counters[target_node_type];
      if (target_node_count == 0) continue;
      neg_node_id = SampleNegativeEdge(m, target_node_type, gsl_r_local);

      // if the sampled neg_node_id appears in the edge, then skip this iteration
      bool skip = false;
      for (int j = 0; j < target_node_count; ++j) {
        if (neg_node_id == curedge.ends[target_node_type][j]) skip = true;
      }
      if (skip) continue;
      neg_node_offset = neg_node_id * dim;

      for (int node_type = 0; node_type < n_node_type; ++node_type) {
        //if (!valid_types[node_type]) continue;
        // keeps the number of nodes belong to type node_type for faster access
        if (valid_types[m][node_type] && curedge.counters[node_type] != 0) {
          sampled_node_ids[node_type] =
            curedge.ends[node_type][gsl_rng_uniform_int(gsl_r_local, curedge.counters[node_type])];
        } else {
          sampled_node_ids[node_type] = -1;
        }
      }
      UpdateTensor(m, sampled_node_ids, target_node_type, neg_node_offset,
          context_score, diff_score);
    }
  }

  free(sampled_node_ids);
  free(context_score);
  free(diff_score);
  pthread_exit(NULL);
}

// output the embeddings of nodes and the context embedding
void Output() {
  // for each type of nodes, output the embeddings
  char file_name[MAX_STRING];
  FILE *fo;

  // output the embeddings of nodes
  for (int node_type = 0; node_type < n_node_type; ++node_type) {
    if (!output_types[node_type]) continue;
    printf("storing the embedding of nodes %s\n", node_names[node_type]);
    sprintf(file_name, "%s/%s_embedding.txt", output_folder, node_names[node_type]);
    fo = fopen(file_name, "wb");
    fprintf(fo, "%ld %d\n", node_cnt[node_type], dim);
    for (long i = 0; i < node_cnt[node_type]; ++i) {
      fprintf(fo, "%s ", nodes[node_type][i].name);
      long offset = i * dim;
      for (int k = 0; k < dim; ++k) {
        fprintf(fo, "%lf ", emb_node[node_type][offset + k]);
      }
      fprintf(fo, "\n");
    }
    fclose(fo);
  }
  if (word2vec && output_types[word_node_type]) {
    printf("storing the context embedding of nodes %s\n", node_names[word_node_type]);
    sprintf(file_name, "%s/context_embedding.txt", output_folder);
    fo = fopen(file_name, "wb");
    fprintf(fo, "%ld %d\n", node_cnt[word_node_type], dim);
    for (long i = 0; i < node_cnt[word_node_type]; ++i) {
      fprintf(fo, "%s ", nodes[word_node_type][i].name);
      long offset = i * dim;
      for (int k = 0; k < dim; ++k) {
        fprintf(fo, "%lf ", context_word[offset + k]);
      }
      fprintf(fo, "\n");
    }
    fclose(fo);
  }
}

void TrainTensor() {
  gsl_rng_env_setup();
  gsl_T = gsl_rng_rand48;
  gsl_r = gsl_rng_alloc(gsl_T);
  gsl_rng_set(gsl_r, 314159265);

  printf("Node types: %d \n", n_node_type);
  for (int i = 0; i < n_node_type; i ++) {
    printf("%s ", node_names[i]);
  }
  printf("\n");

  InitHashTable();

  printf("Hash Table\n");

  weight_edge_cnt = 0;
  for (int m = 0; m < n_rela_type; m ++) {
    printf("Start to read relations of %s\n", relation_names[m]);  
    ReadEdgeData(m);
    //weight_edge_cnt += (long) (edge_cnt[m] * rela_weight[m]); 
  }

  printf("--------------------------------\n");
  for (int k = 0; k < n_node_type; ++k) {
    printf("Number of nodes (%s): %ld          \n", node_names[k], node_cnt[k]);
  }

  // map each ege into corresponding
  total_samples = (long) (1000000 * total_samples);

  printf("--------------------------------\n");
  printf("Samples: %ldM \n", total_samples / 1000000);
  //printf("Samples: %ldM (%ld iter.)\n",
  //    total_samples / 1000000,  total_samples / weight_edge_cnt);
  printf("Dimension: %d\n", dim);
  printf("Initial alpha: %.3lf\n", alpha);
  printf("Word2Vec: %s      Thread Ratio: %.2f\n",
      word2vec? "Yes": "No", (real) num_word2vec_threads / num_tensor_threads);

  printf("Valid types during optimization:\n");
  for (int m = 0; m < n_rela_type; ++m) {
    printf("\tRelation %s:  \n", relation_names[m]); 
    for (int i = 0; i < n_node_type_relation[m]; ++i) {
      printf("\t\t%s %d \n ", node_names[id_map[m][i]], valid_types[m][id_map[m][i]]);
    }
    printf("\n"); 
  }
  printf("\n");
  printf("L2 regularization coefficients:\n");
  for (int m = 0; m < n_rela_type; ++m) {
    printf("\tRelation %s:  \n", relation_names[m]); 
    for (int i = 0; i < n_node_type_relation[m]; ++i) {
      printf("\t\t%s %.2f\n", node_names[id_map[m][i]], beta[m][id_map[m][i]]);
    }
  }
  printf("\n");

  printf("Gradient coefficients:\n");
  real max_value;

  for (int m = 0; m < n_rela_type; ++m) {
    max_value = 0; 
    printf("\tRelation %s:  \n", relation_names[m]); 
    for (int i = 0; i < n_node_type; ++i) {
      if (i == center_type[m]) continue;
      if (valid_types[m][i]) {
        gradient_coeff[m][i] = pow(node_cnt[i], 1.2);
        if (gradient_coeff[m][i] > max_value) {
          max_value = gradient_coeff[m][i];
        }
      }
    }
    for (int i = 0; i < n_node_type; ++i) {
      if (i == center_type[m] || !valid_types[m][i]) {
        gradient_coeff[m][i] = 1;
      } else {
        gradient_coeff[m][i] /= max_value;
      }
      gradient_coeff[m][i] /= mini_batch;
      if (i == center_type[m] || valid_types[m][i])
        printf("\t\t%s %.4f\n", node_names[i], gradient_coeff[m][i]);
    }
  }
  printf("\n");


  printf("Scoring function: \n");
  for (int m = 0; m < n_rela_type; m ++) {
    printf("\tRelation %s: %d \n", relation_names[m], scoring[m]);
  }
  printf("\n");

  printf("Relation type weight: \n");
  for (int m = 0; m < n_rela_type; ++m) {
    printf("\tRelation %s: %.3f \n", relation_names[m], rela_weight[m]); 
  } 
  printf("\n");

  printf("--------------------------------\n");

  if (word2vec) {
    printf("Start initializing word2vec... \t\t\t\t%c", 13); 
    ReadWordEdgeData(); 
    InitWordNegTable();
    InitAliasTable();  
  }

  InitVector();

  for (int i = 0; i < n_node_type; ++i) {
    if (load_types[i] && valid_types_global[i]) ReadFixEmbedding(i);
    free(node_hash_table[i]);
  }

  for (int m = 0; m < n_rela_type; ++m){
    if (scoring[m] == 2){
      InitNegTable(m, center_type[m]);
    }
    else {
      for (int i = 0; i < n_node_type_relation[m]; ++i) {
        InitNegTable(m, id_map[m][i]); 
      }
    } 
  }

  InitSigmoidTable();
  printf("Finish initialization!\t\t\t\t\t\t\t\t\t\t\t %c", 13);

  struct timeval t1, t2;
  gettimeofday(&t1, NULL);
  start = clock();
  pthread_t *pt = 
    (pthread_t *)malloc((num_tensor_threads + num_word2vec_threads) * sizeof(pthread_t));

  for (long a = 0; a < num_tensor_threads; ++a) {
    pthread_create(&pt[a], NULL, TrainTensorThread, (void *)a);
  }
  for (long a = 0; a < num_word2vec_threads; ++a) {
    pthread_create(&pt[a + num_tensor_threads], NULL,
        TrainWord2VecThread, (void *) (a + num_tensor_threads));
  }

  for (int a = 0; a < num_tensor_threads; ++a) pthread_join(pt[a], NULL);
  for (int a = 0; a < num_word2vec_threads; ++a) pthread_cancel(pt[a + num_tensor_threads]);

  gettimeofday(&t2, NULL);
  printf("\nTotal optimization time: %.2lf minitues\n", (real)(t2.tv_sec - t1.tv_sec) / 60);
  Output();
}

int ArgPos(char *str, int argc, char **argv) {
  for (int a = 0; a < argc; ++a) {
    if (!strcmp(str, argv[a])) {
      if (a == argc - 1) {
        printf("Argument missing for %s\n", str);
        exit(1);
      }
      return a;
    }
  }
  return -1;
}

void printSyntax()
{
  printf("Options for each relation per line:\n");
  printf("Parameters for training:\n");
  printf("\t-type <int>\n");
  printf("\t\tNumber of types of nodes; default is 2\n");
  printf("\t-negative <int>\n");
  printf("\t\tDeprecated: number of negative examples; default is 1\n");
  printf("\t-name <string> <string> ... <string>\n");
  printf("\t\tSet the name of the types of nodes\n");
  printf("\t-choose <binary string> \n");
  printf("\t\tSet the types of nodes to output\n");
  printf("\t-valid <binary string> \n");
  printf("\t\tSet the types of nodes to be activated during optimization\n");
  printf("\t-beta <float> <float> ... <float> \n");
  printf("\t\tSet the regularization coefficients for all node types\n");
  printf("\t-load <binary string> ...\n");
  printf("\t\tLoad the pretrained types of nodes\n");
}

bool ReadParameters(char * sParaFile)
{
  int argc[MAX_N_RELATION_TYPE], i, j, k, lenLine; 
  char *argv[MAX_N_RELATION_TYPE][MAX_PARAMETER], line[MAX_SENTENCE], argt[MAX_SENTENCE];

  FILE *f = fopen(sParaFile, "rb");
  if (f == NULL) {
    printf("EORROR: parameter file not found!\n ");
    printSyntax(); 
    return false; 
  }  

  while (fgets(line, MAX_SENTENCE, f) != NULL) {
    n_rela_type += 1;           // increase the relation type number by 1
    lenLine = strlen(line);
    j = -1;
    i = 0;

    while (i <= lenLine && line[i] != '\n' && line[i] != '\r') { 
      while (line[i] == ' ' || line[i] == '\n' || line[i] == '\r') {
        i ++;
        if (i >= lenLine) break;
      }
      if (i >= lenLine) break;

      k = 0;
      while (line[i] != ' ' && line[i] != '\n' && line[i] != '\r' && i < lenLine)
      {
        argt[k ++] = line[i++]; 
      }
      if (k > 0) {
        argt[k] = 0;
        argv[n_rela_type - 1][++j] = (char *) malloc(k); 
        strcpy(argv[n_rela_type - 1][j], argt);  
      }
      else 
        i++;
    }
    argc[n_rela_type - 1] = j + 1; 
  }

//  for (i = 0; i < n_rela_type; i ++) {
//    printf("parameters : %d \n", argc[i]); 
//    for (j = 0; j < argc[i]; j ++) {
//      printf(" |%s| ", argv[i][j]);
//    } 
//    printf("\n"); 
//  }

  for (i = 0; i < n_node_type; ++i) {
    output_types[i] = false;
    valid_types_global[i] = false; 
    load_types[i] = false; 
  }
  for (int i = 0; i < n_rela_type; ++i) {
    scoring[i] = -1; 
  }

  for (int m = 0; m < n_rela_type; m ++) {
    if ((i = ArgPos((char *) "-type", argc[m], argv[m])) > 0)
      n_node_type_relation[m] = atoi(argv[m][i+1]);

    if ((i = ArgPos((char *) "-scoring", argc[m], argv[m])) > 0)
      scoring[m] = atoi(argv[m][i+1]); 

    if ((i = ArgPos((char *) "-name", argc[m], argv[m])) > 0) {
      for (j = 0; j < n_node_type_relation[m]; ++j) {
        // check if the current node name exists in the global node name array 
        for (k = 0; k < n_node_type; ++k) {
          if (strcmp(node_names[k], argv[m][i + j + 1]) == 0) break;
        }
        if (k == n_node_type) {
          strcpy(node_names[n_node_type ++], argv[m][i + j + 1]);
          if (strcmp(node_names[n_node_type - 1], "term") == 0) {
            word_node_type = n_node_type - 1; 
          }
        }
        // the jth node type of relation m is the k-th global node type
        id_map[m][j] = k; 
      }
    }
  }


  for (int m = 0; m < n_rela_type; m ++) {
    if (n_node_type_relation[m] == 2 && id_map[m][0] == word_node_type && id_map[m][1] == word_node_type) {
      word_relation_type = m;
      word2vec = true; 
      break;  
    }
  } 

  for (int m = 0; m < n_rela_type; m ++) {

    // initialize the pramameters for each relation type 
    for (i = 0; i < n_node_type; ++i) {
      valid_types[m][i] = false;
      beta[m][i] = 0.0;
      gradient_coeff[m][i] = 1.0 / mini_batch;
    }

    if ((i = ArgPos((char *) "-choose", argc[m], argv[m])) >= 0) {
      for (int j = 0; j < n_node_type_relation[m]; ++j) {
        output_types[id_map[m][j]] = (argv[m][i + 1][j] == '1');
      }
    }

    if ((i = ArgPos((char *) "-beta", argc[m], argv[m])) >= 0) {
      for (int j = 0; j < n_node_type_relation[m]; ++j) {
        beta[m][id_map[m][j]] = atof(argv[m][i + 1 + j]);
      }
    }

    if ((i = ArgPos((char *) "-valid", argc[m], argv[m])) >= 0) {
      for (int j = 0; j < n_node_type_relation[m]; ++j) {
        valid_types[m][id_map[m][j]] = (argv[m][i + 1][j] == '1');
        if(valid_types[m][id_map[m][j]])
          valid_types_global[id_map[m][j]] = true;
      }
    }

    if ((i = ArgPos((char *) "-load", argc[m], argv[m])) >= 0) {
      for (int j = 0; j < n_node_type_relation[m]; ++j) {
        if (argv[m][i + 1][j] == '1') {
          load_types[id_map[m][j]] = true; 
        }
      }
    }

    if ((i = ArgPos((char *) "-center", argc[m], argv[m])) >= 0) {
      center_type[m] = id_map[m][atoi(argv[m][i + 1])];
    }

    if ((i = ArgPos((char *) "-relation", argc[m], argv[m])) >= 0) {
      strcpy(relation_names[m], argv[m][i + 1]);
      sprintf(train_network_file[m], "%s/events-%s.txt", input_folder, relation_names[m]);
      printf("Input %d: %s\n", m, train_network_file[m]); 
    }

    if ((i = ArgPos((char *) "-weight", argc[m], argv[m])) >= 0) {
      rela_weight[m] = atof(argv[m][i + 1]); 
    } 

  }

  real rela_weight_total = 0; 
  for (int m = 0; m < n_rela_type; ++m) {
    rela_weight_total += rela_weight[m];
  }
  for (int m = 0; m < n_rela_type; ++m) {
    rela_weight[m] /= rela_weight_total;
  }


  for (int i = 0; i < n_node_type; ++i) {
    nodes[i] = 
      (struct ClassNode *) calloc(node_count_inc, sizeof(struct ClassNode));
  }

  for(int m = 0; m < n_rela_type; ++m) {
    if (scoring[m] == 2) {
      assert(valid_types[m][center_type[m]] &&
          "When scoring function is 2, valid_types[center_type] must be true\n");
    }
  }
  sprintf(word_network_file, "%s/words_edges.txt", input_folder);

  printf("Finish reading parameters for each relation type.\n"); 
  return true; 
}

int main(int argc, char **argv) {
  int i;
  strcpy(output_folder, "output");
  strcpy(input_folder, "data");

  if (argc == 1) {
    printf("\t Tensor2Vec: embedding of higer-order relational data\n");
    printf("Options:"); 
    printf("\tPlease specify the parameter file!\n");
    printf("\t-para <file> \n");
    printf("\t\tProvide the parameter file on relations\n");
    printf("\t-size <int>\n");
    printf("\t\tSet dimension of vertex embeddings; default is 200\n");
    printf("\t-scoring <int>\n");
    printf("\t\tScoring function, 0: complete element-wise product,"
        "1: complete pairwise inner product, 2: pairwise inner product with central type\n");
    printf("\t-iter <int>\n");
    printf("\t\tSet the number of training samples as the number of iterations of number of edges\n");
    printf("\t-tthread <int>\n");
    printf("\t\tUse <int> threads for tensor updating(default 1)\n");
    printf("\t-wthread <int>\n");
    printf("\t\tUse <int> threads for word2vec updating(default 1)\n");
    printf("\t-negative <int>\n");
    printf("\t\tSample <int> negative samples for word2vec updating(default 1)\n");
    printf("\t-alpha <float>\n");
    printf("\t\tSet the starting learning rate; default is 0.025\n");
    printf("\t-input input folder ...\n");
    printf("\t\tSet the folder data which read data\n");
    printf("\t-output output folder ...\n");
    printf("\t\tSet the folder to which output data\n");
    return 0;
  }

  // global parameters shared across all the relations
  if ((i = ArgPos((char *) "-size", argc, argv)) > 0)
    dim = atoi(argv[i + 1]);
  if ((i = ArgPos((char *) "-iter", argc, argv)) > 0)
    total_samples = atoi(argv[i + 1]);
  if ((i = ArgPos((char *) "-alpha", argc, argv)) > 0)
    init_alpha = atof(argv[i + 1]);
  if ((i = ArgPos((char *) "-tthread", argc, argv)) > 0) {
    num_tensor_threads = atoi(argv[i + 1]);
  }

  //////////////////////////////////////////////////////
  if ((i = ArgPos((char *) "-wthread", argc, argv)) > 0) {
    num_word2vec_threads = atoi(argv[i + 1]);
    if (num_word2vec_threads > 0) word2vec = true;
  }
  if ((i = ArgPos((char *) "-negative", argc, argv)) > 0) {
    word_negative = atoi(argv[i + 1]);
  }
  ///////////////////////////////////////////////////

  if ((i = ArgPos((char *) "-output", argc, argv)) > 0)
    strcpy(output_folder, argv[i + 1]);
  if ((i = ArgPos((char *) "-input", argc, argv)) > 0)
    strcpy(input_folder, argv[i + 1]);

  if ((i = ArgPos((char *) "-para", argc, argv)) > 0)
    ReadParameters(argv[i + 1]);
  else
  {
    printf("\tPlease specify the parameter file!");
    return 0;
  }

  for (int i = 0; i < n_node_type; ++i) {
    sprintf(input_fix_embedding_file[i],
        "%s/%s_embedding.txt", input_folder, node_names[i]);
  }
  alpha = init_alpha;
  TrainTensor();
  return 0;
}
