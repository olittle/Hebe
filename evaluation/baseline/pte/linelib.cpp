#include "linelib.h"

line_node::line_node()
{
    node = NULL;
    node_size = 0;
    node_max_size = 1000;
    vector_size = 0;
    node_file[0] = 0;
    node_hash = NULL;
    neg_table = NULL;
    vec = NULL;
}

line_node::~line_node()
{
    if (node != NULL) {free(node); node = NULL;}
    node_size = 0;
    node_max_size = 0;
    vector_size = 0;
    node_file[0] = 0;
    if (node_hash != NULL) {free(node_hash); node_hash = NULL;}
    if (neg_table != NULL) {free(neg_table); neg_table = NULL;}
    if (vec != NULL) {free(vec); vec = NULL;}
}

void line_node::init_neg_table()
{
    int a, i;
    double total_pow = 0, d1;
    real power = 0.75;
    neg_table = (int *)malloc(neg_table_size * sizeof(int));
    for (a = 0; a < node_size; a++) total_pow += pow(node[a].cn, power);
    i = 0;
    d1 = pow(node[i].cn, power) / (real)total_pow;
    for (a = 0; a < neg_table_size; a++) {
        neg_table[a] = i;
        if (a / (real)neg_table_size > d1) {
            i++;
            d1 += pow(node[i].cn, power) / (real)total_pow;
        }
        if (i >= node_size) i = node_size - 1;
    }
}

int line_node::get_hash(char *word)
{
    unsigned long long a, hash = 0;
    for (a = 0; a < strlen(word); a++) hash = hash * 257 + word[a];
    hash = hash % hash_table_size;
    return hash;
}

int line_node::search(char *word)
{
    unsigned int hash = get_hash(word);
    while (1) {
        if (node_hash[hash] == -1) return -1;
        if (!strcmp(word, node[node_hash[hash]].word)) return node_hash[hash];
        hash = (hash + 1) % hash_table_size;
    }
    return -1;
}

int line_node::add_node(char *word, double cn)
{
    unsigned int hash, length = strlen(word) + 1;
    if (length > MAX_STRING) length = MAX_STRING;
    node[node_size].word = (char *)calloc(length, sizeof(char));
    strcpy(node[node_size].word, word);
    node[node_size].cn = cn;
    node_size++;
    // Reallocate memory if needed
    if (node_size + 2 >= node_max_size) {
        node_max_size += 1000;
        node = (struct struct_node *)realloc(node, node_max_size * sizeof(struct struct_node));
    }
    hash = get_hash(word);
    while (node_hash[hash] != -1) hash = (hash + 1) % hash_table_size;
    node_hash[hash] = node_size - 1;
    return node_size - 1;
}

int line_node::compare(const void *a, const void *b)
{
    double wei_a = ((struct struct_node *)a)->cn;
    double wei_b = ((struct struct_node *)b)->cn;
    if (wei_b > wei_a) return 1;
    else if (wei_b < wei_a) return -1;
    else return 0;
}

void line_node::sort_node()
{
    int a, size;
    unsigned int hash;
    // Sort the vocabulary and keep </s> at the first position
    std::sort(node, node + node_size);
    for (a = 0; a < hash_table_size; a++) node_hash[a] = -1;
    size = node_size;
    for (a = 0; a < size; a++) {
        // Hash will be re-computed, as after the sorting it is not actual
        hash = get_hash(node[a].word);
        while (node_hash[hash] != -1) hash = (hash + 1) % hash_table_size;
        node_hash[hash] = a;
    }
    node = (struct struct_node *)realloc(node, (node_size + 1) * sizeof(struct struct_node));
}

int line_node::get_vector_dim()
{
    return vector_size;
}

int line_node::get_num_nodes()
{
    return node_size;
}

real *line_node::get_vector()
{
    return vec;
}

struct struct_node *line_node::get_node()
{
    return node;
}

int line_node::get_neg_sample(int index)
{
    return neg_table[index];
}

void line_node::init(char *file_name, int vector_dim)
{
    printf("DEBUG: STEP 0 %s\n", file_name);

    strcpy(node_file, file_name);
    vector_size = vector_dim;

    printf("DEBUG: STEP 1");
    
    node = (struct struct_node *)calloc(node_max_size, sizeof(struct struct_node));
    node_hash = (int *)calloc(hash_table_size, sizeof(int));
    for (int k = 0; k != hash_table_size; k++) node_hash[k] = -1;
    
    printf("DEBUG: STEP 2");


    FILE *fi = fopen(node_file, "rb");
    if (fi == NULL)
    {
        printf("ERROR: node file not found!\n");
        printf("%s\n", node_file);
        exit(1);
    }

    char word[MAX_STRING];
    double cn;
    node_size = 0;
    while (1)
    {
        if (fscanf(fi, "%s %lf", word, &cn) != 2) break;
        add_node(word, cn);
    }
    fclose(fi);
    
    printf("DEBUG: STEP 3");


    this->init_neg_table();
 
    printf("DEBUG: STEP 4");

   
    long long a, b;
    a = posix_memalign((void **)&vec, 128, (long long)node_size * vector_size * sizeof(real));
    if (vec == NULL) { printf("Memory allocation failed\n"); exit(1); }
    for (b = 0; b < vector_size; b++) for (a = 0; a < node_size; a++)
        vec[a * vector_size + b] = (rand() / (real)RAND_MAX - 0.5) / vector_size;
    
    printf("Reading nodes from file: %s, DONE!\n", node_file);
    printf("Node size: %d\n", node_size);
    printf("Node dims: %d\n", vector_size);
}

void line_node::output(char *file_name, int binary)
{
    FILE *fo = fopen(file_name, "wb");
    fprintf(fo, "%d %d\n", node_size, vector_size);
    for (int a = 0; a != node_size; a++)
    {
        fprintf(fo, "%s ", node[a].word);
        if (binary) for (int b = 0; b != vector_size; b++) fwrite(&vec[a * vector_size + b], sizeof(real), 1, fo);
        else for (int b = 0; b != vector_size; b++) fprintf(fo, "%lf ", vec[a * vector_size + b]);
        fprintf(fo, "\n");
    }
    fclose(fo);
}

line_link::line_link()
{
    node_u = NULL;
    node_v = NULL;
    expTable = NULL;
    ws = NULL;
    edge_cnt = 0;
    edge_u = NULL;
    edge_v = NULL;
    edge_w = NULL;
    link_file[0] = 0;
}

line_link::~line_link()
{
    node_u = NULL;
    node_v = NULL;
    if (expTable != NULL) {free(expTable); expTable = NULL;}
    if (ws != NULL) {ransampl_free(ws); ws = NULL;}
    edge_cnt = 0;
    if (edge_u != NULL) {free(edge_u); edge_u = NULL;}
    if (edge_v != NULL) {free(edge_v); edge_v = NULL;}
    if (edge_w != NULL) {free(edge_w); edge_w = NULL;}
    link_file[0] = 0;
}

void line_link::init(char *file_name, line_node *p_u, line_node *p_v, int negative, int adaptive_grad)
{
    strcpy(link_file, file_name);
    node_u = p_u;
    node_v = p_v;
    neg_samples = negative;
    ad_grad = adaptive_grad;
    if (node_u->get_vector_dim() != node_v->get_vector_dim())
    {
        printf("ERROR: vector dimsions are not same!\n");
        exit(1);
    }
    
    char str[2 * MAX_STRING + 10000];
    FILE *fi = fopen(link_file, "rb");
    if (fi == NULL)
    {
        printf("ERROR: link file not found!\n");
        printf("%s\n", link_file);
        exit(1);
    }
    edge_cnt = 0;
    while (fgets(str, sizeof(str), fi)) edge_cnt++;
    fclose(fi);
    
    char word_u[MAX_STRING], word_v[MAX_STRING];
    int u, v;
    double wei;
    edge_u = (int *)malloc(edge_cnt * sizeof(int));
    edge_v = (int *)malloc(edge_cnt * sizeof(int));
    edge_w = (double *)malloc(edge_cnt * sizeof(double));
    if (edge_u == NULL || edge_v == NULL || edge_w == NULL)
    {
        printf("Error: memory allocation failed!\n");
        exit(1);
    }
    
    fi = fopen(link_file, "rb");
    long long actual_edge_cnt = 0;
    for (int k = 0; k != edge_cnt; k++)
    {
        fscanf(fi, "%s %s %lf", word_u, word_v, &wei);
        
        // if (k % 30000 == 0){
        //     printf("Debug: %s %s %lf\n", word_u, word_v, wei);
        //     // break;
        // }

        if (k % 10000 == 0)
        {
            printf("Reading edges: %.3lf%%%c             ", k / (double)(edge_cnt + 1) * 100, 13);
            fflush(stdout);
        }
        
        u = node_u->search(word_u);
        v = node_v->search(word_v);
        
        if (u == -1 || v == -1) continue;
        
        edge_u[actual_edge_cnt] = u;
        edge_v[actual_edge_cnt] = v;
        edge_w[actual_edge_cnt] = wei;
        actual_edge_cnt++;
    }
    fclose(fi);
    
    ws = ransampl_alloc(edge_cnt);
    ransampl_set(ws, edge_w);
    
    expTable = (real *)malloc((EXP_TABLE_SIZE + 1) * sizeof(real));
    for (int i = 0; i < EXP_TABLE_SIZE; i++) {
        expTable[i] = exp((i / (real)EXP_TABLE_SIZE * 2 - 1) * MAX_EXP); // Precompute the exp() table
        expTable[i] = expTable[i] / (expTable[i] + 1);                   // Precompute f(x) = x / (x + 1)
    }
    
    if (ad_grad == 1)
    {
        long long a, b;
        int node_size, vector_size;
        
        node_size = node_u->get_num_nodes();
        vector_size = node_u->get_vector_dim();
        a = posix_memalign((void **)&grad_u, 128, (long long)node_size * vector_size * sizeof(double));
        if (grad_u == NULL) { printf("Memory allocation failed\n"); exit(1); }
        for (b = 0; b < vector_size; b++) for (a = 0; a < node_size; a++)
            grad_u[a * vector_size + b] = 0.000001;
        
        node_size = node_v->get_num_nodes();
        vector_size = node_v->get_vector_dim();
        a = posix_memalign((void **)&grad_v, 128, (long long)node_size * vector_size * sizeof(double));
        if (grad_v == NULL) { printf("Memory allocation failed\n"); exit(1); }
        for (b = 0; b < vector_size; b++) for (a = 0; a < node_size; a++)
            grad_v[a * vector_size + b] = 0.000001;
    }
    
    printf("Reading edges from file: %s, DONE!\n", link_file);
    printf("Edge size: %lld\n", edge_cnt);
}

void ::line_link::train_sample(real *error_vec, real alpha, double rand_num_1, double rand_num_2, unsigned long long &rand_index)
{
    long long edge_id, l1, l2;
    int target, label, u, v, vector_size;
    real f, g;
    
    edge_id = ransampl_draw(ws, rand_num_1, rand_num_2);
    u = edge_u[edge_id];
    v = edge_v[edge_id];
    
    vector_size = node_u->get_vector_dim();
    l1 = u * vector_size;
    for (int c = 0; c != vector_size; c++) error_vec[c] = 0;
    
    if (ad_grad == 0)
    {
        for (int d = 0; d < neg_samples + 1; d++)
        {
            if (d == 0)
            {
                target = v;
                label = 1;
            }
            else
            {
                rand_index = rand_index * (unsigned long long)25214903917 + 11;
                target = node_v->get_neg_sample((rand_index >> 16) % neg_table_size);
                if (target == v) continue;
                label = 0;
            }
            l2 = target * vector_size;
            f = 0;
            for (int c = 0; c != vector_size; c++) f += (node_u->get_vector())[l1 + c] * (node_v->get_vector())[l2 + c];
            if (f > MAX_EXP) g = (label - 1) * alpha;
            else if (f < -MAX_EXP) g = (label - 0) * alpha;
            else g = (label - expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
            for (int c = 0; c != vector_size; c++) error_vec[c] += g * (node_v->get_vector())[l2 + c];
            for (int c = 0; c != vector_size; c++) (node_v->get_vector())[l2 + c] += g * (node_u->get_vector())[l1 + c];
        }
        
        for (int c = 0; c != vector_size; c++) (node_u->get_vector())[l1 + c] += error_vec[c];
    }
    else
    {
        for (int d = 0; d < neg_samples + 1; d++)
        {
            if (d == 0)
            {
                target = v;
                label = 1;
            }
            else
            {
                rand_index = rand_index * (unsigned long long)25214903917 + 11;
                target = node_v->get_neg_sample((rand_index >> 16) % neg_table_size);
                if (target == v) continue;
                label = 0;
            }
            l2 = target * vector_size;
            f = 0;
            for (int c = 0; c != vector_size; c++) f += (node_u->get_vector())[l1 + c] * (node_v->get_vector())[l2 + c];
            if (f > MAX_EXP) g = label - 1;
            else if (f < -MAX_EXP) g = label - 0;
            else g = label - expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))];
            for (int c = 0; c != vector_size; c++) error_vec[c] += g * (node_v->get_vector())[l2 + c];
            for (int c = 0; c != vector_size; c++)
            {
                grad_v[l2 + c] += g * g * (node_u->get_vector())[l1 + c] * (node_u->get_vector())[l1 + c];
                (node_v->get_vector())[l2 + c] += alpha * g * (node_u->get_vector())[l1 + c] / sqrt(grad_v[l2 + c]);
            }
        }
        
        for (int c = 0; c != vector_size; c++)
        {
            grad_u[l1 + c] += error_vec[c] * error_vec[c];
            (node_u->get_vector())[l1 + c] += alpha * error_vec[c] / sqrt(grad_u[l1 + c]);
        }
    }
}

void ::line_link::train_sample_square(real *error_vec, real alpha, double rand_num_1, double rand_num_2, unsigned long long &rand_index)
{
    long long edge_id, lu, lv, lw;
    int u, v, w, vector_size;
    real s_uv, s_uw;
    
    edge_id = ransampl_draw(ws, rand_num_1, rand_num_2);
    u = edge_u[edge_id];
    v = edge_v[edge_id];
    rand_index = rand_index * (unsigned long long)25214903917 + 11;
    w = node_v->get_neg_sample((rand_index >> 16) % neg_table_size);
    
    vector_size = node_u->get_vector_dim();
    
    lu = u * vector_size;
    lv = v * vector_size;
    lw = w * vector_size;
    
    s_uv = 0;
    for (int c = 0; c != vector_size; c++)
        s_uv += ((node_u->get_vector())[lu + c] - (node_v->get_vector())[lv + c]) * ((node_u->get_vector())[lu + c] - (node_v->get_vector())[lv + c]);
    s_uw = 0;
    for (int c = 0; c != vector_size; c++)
        s_uw += ((node_u->get_vector())[lu + c] - (node_v->get_vector())[lw + c]) * ((node_u->get_vector())[lu + c] - (node_v->get_vector())[lw + c]);
    
    if (s_uw - s_uv < 1)
    {
        for (int c = 0; c != vector_size; c++) error_vec[c] = 0;
        
        for (int c = 0; c != vector_size; c++)
            error_vec[c] += alpha * ((node_v->get_vector())[lv + c] - (node_v->get_vector())[lw + c]);
        for (int c = 0; c != vector_size; c++)
            (node_v->get_vector())[lv + c] += alpha * ((node_u->get_vector())[lu + c] - (node_v->get_vector())[lv + c]);
        for (int c = 0; c != vector_size; c++)
            (node_v->get_vector())[lw + c] += alpha * ((node_v->get_vector())[lw + c] - (node_u->get_vector())[lu + c]);
        for (int c = 0; c != vector_size; c++)
            (node_u->get_vector())[lu + c] += error_vec[c];
    }
}


void linelib_output_batch(char *file_name, int binary, line_node **array_line_node, int array_length)
{
    int total_node_size = 0;
    for (int k = 0; k != array_length; k++) total_node_size += array_line_node[k]->node_size;
    int vector_size = array_line_node[0]->vector_size;
    for (int k = 1; k != array_length; k++) if (array_line_node[k]->vector_size != vector_size)
    {
        printf("Error: vector dimensions are not equivalent!\n");
        exit(1);
    }
    
    FILE *fo = fopen(file_name, "wb");
    fprintf(fo, "%d %d\n", total_node_size, vector_size);
    for (int k = 0; k != array_length; k++) for (int a = 0; a != array_line_node[k]->node_size; a++)
    {
        fprintf(fo, "%s ", array_line_node[k]->node[a].word);
        if (binary) for (int b = 0; b != vector_size; b++) fwrite(&(array_line_node[k]->vec[a * vector_size + b]), sizeof(real), 1, fo);
        else for (int b = 0; b != vector_size; b++) fprintf(fo, "%lf ", array_line_node[k]->vec[a * vector_size + b]);
        fprintf(fo, "\n");
    }
    fclose(fo);
}
