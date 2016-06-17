#include "common.h"
#include "function.h"

/* number of arguments (descendants) required by each terminal function in GP in the following order:
SUM, SUB, MUL, DIV, EXP, SQRT, LOG and ABS */
const int N_ARGS_FUNCTION[] =  {2,2,2,2,1,1,1,1};

/* Agent-related functions */
/* It creates an agent
Parameters:
n: number of decision variables
opt_id: identifier of the optimization technique */
Agent *CreateAgent(int n, int opt_id){
    if((n < 1) || opt_id < 1){
        fprintf(stderr,"\nInvalid parameters @CreateAgent.\n");
        return NULL;
    }
    
    Agent *a = NULL;
    a = (Agent *)malloc(sizeof(Agent));
    a->v = NULL;
    a->xl = NULL;
    a->fit = DBL_MAX;
    a->pfit = DBL_MAX;
    a->n = n;
    
    switch (opt_id){
        case _PSO_:
        case _BA_:
        case _FPA_:
        case _FA_:
        case _GA_:
        case _GP_:
            a->x = (double *)calloc(n,sizeof(double));
            if(opt_id != _GP_) a->v = (double *)calloc(n,sizeof(double));
            if(opt_id == _PSO_) a->xl = (double *)calloc(n,sizeof(double));
        break;
        default:
            free(a);
            fprintf(stderr,"\nInvalid optimization identifier @CreateAgent\n");
        return NULL;
        break;
    }
    
    return a;
}

/* It deallocates an agent
Parameters:
a: address of the agent to be deallocated
opt_id: identifier of the optimization technique */
void DestroyAgent(Agent **a, int opt_id){
    Agent *tmp = NULL;
    
    tmp = *a;
    if(!tmp){
        fprintf(stderr,"\nAgent not allocated @DestroyAgent.\n");
        exit(-1);
    }
    
    switch (opt_id){
        case _PSO_:
        case _BA_:
        case _FPA_:
        case _FA_:
        case _GA_:
        case _GP_:
            if(tmp->x) free(tmp->x);
            if(tmp->v) free(tmp->v);
            if(opt_id == _PSO_) if(tmp->xl) free(tmp->xl);
        break;
        default:
            fprintf(stderr,"\nInvalid optimization identifier @DestroyAgent.\n");
        break;
    }
    
    free(tmp);
}

/* It checks whether a given agent has excedeed boundaries
Parameters:
s: search space
a: agent */
void CheckAgentLimits(SearchSpace *s, Agent *a){
    if((!s) || (!a)){
        fprintf(stderr,"\nInvalid input parameters @CheckAgentLimits.\n");
        exit(-1);
    }
    
    int j;

    for(j = 0; j < a->n; j++){
        if(a->x[j] < s->LB[j]) a->x[j] = s->LB[j];
        else if(a->x[j] > s->UB[j]) a->x[j] = s->UB[j];
    }
}

/* It copies an agent
Parameters:
a: agent
opt_id: identifier of the optimization technique */
Agent *CopyAgent(Agent *a, int opt_id){
    if(!a){
        fprintf(stderr,"\nAgent not allocated @CopyAgent.\n");
        exit(-1);
    }
    
    Agent *cpy = NULL;
    cpy = CreateAgent(a->n, opt_id);
    
    switch (opt_id){
        case _PSO_:
        case _BA_:
        case _FPA_:
        case _FA_:
        case _GA_:
            memcpy(cpy->x, a->x, a->n*sizeof(double));
            memcpy(cpy->v, a->v, a->n*sizeof(double));
            if(opt_id == _PSO_) memcpy(cpy->xl, a->xl, a->n*sizeof(double));
        break;
        default:
            fprintf(stderr,"\nInvalid optimization identifier @CopyAgent.\n");
            DestroyAgent(&cpy, opt_id);
            return NULL;
        break;
    }
    
    return cpy;
}

/* It generates a new agent according to each technique
Paremeters:
s: search space
opt_id: identifier of the optimization technique */
Agent *GenerateNewAgent(SearchSpace *s, int opt_id){
    if(!s){
        fprintf(stderr,"\nSearch space not allocated @GenerateNewAgent.\n");
        exit(-1);
    }
    
    Agent *a = NULL;
    int j;
        
    switch (opt_id){
        case _PSO_:
        break;
        case _BA_:
            a = CreateAgent(s->n, _BA_);
            
            /* The factor 0.001 limits the step sizes of random walks */ 
            for(j = 0; j < s->n; j++)
                a->x[j] = s->g[j]+0.001*GenerateUniformRandomNumber(0,1); 
        break;
        case _FPA_:
        break;
        case _FA_:
        break;
        case _GA_:
        break;
        default:
            fprintf(stderr,"\nInvalid optimization identifier @GenerateNewAgent.\n");
            return NULL;
        break;
    }
    
    return a;
}

/**************************/

/* Search Space-related functions */
/* It creates a search space
Parameters:
m: number of agents
n: number of decision variables
opt_id: identifier of the optimization technique
additional parameters: related to each technique
PSO, BA, FPA and FA: do not require additional parameters
GP: it requires the minimum and maximum depth of a tree, number of terminals and a matrix (char **) with the terminals' names  */
SearchSpace *CreateSearchSpace(int m, int n, int opt_id, ...){
    SearchSpace *s = NULL;
    va_list arg;
    
    if((m < 1) || (n < 1) || (opt_id < 1)){
        fprintf(stderr,"\nInvalid parameters @CreateSearchSpace.\n");
        return NULL;
    }
    int i, n_terminals;
    
    va_start(arg, opt_id);
    s = (SearchSpace *)malloc(sizeof(SearchSpace));
    s->m = m;
    s->n = n;
    s->gfit = DBL_MAX;
    
    if(opt_id != _GP_){ /* GP uses a different structure than that of others */
        s->a = (Agent **)malloc(s->m*sizeof(Agent *));
        s->a[0] = CreateAgent(s->n, opt_id);
        if(s->a[0]){ /* Here, we verify whether opt_id is valid or not. In the latter case, function CreateAgent returns NULL. */
            for(i = 1; i < s->m; i++)
                s->a[i] = CreateAgent(s->n, opt_id);
        }else{
            free(s->a);
            free(s);
            fprintf(stderr,"\nInvalid optimization identifier @CreateSearchSpace.\n");
            return NULL;
        }
    
        switch (opt_id){
            case _PSO_:
            case _BA_:
            case _FPA_:
            case _FA_:
            case _GA_:
                s->g = (double *)calloc(s->n,sizeof(double));
            break;
        }
    }else{
        if(opt_id == _GP_){
            s->min_depth = va_arg(arg, int);
            s->max_depth = va_arg(arg, int);
            s->n_terminals = va_arg(arg, int);
            s->n_constants = va_arg(arg, int);
            s->n_functions = va_arg(arg, int);            
            s->terminal = va_arg(arg, char **);
            s->constant = va_arg(arg, double *);
            s->function = va_arg(arg, char **);
    
            s->T = (Node **)malloc(s->m*sizeof(Node *));
            for(i = 0; i < s->m; i++)
                s->T[i] = GROW(s, s->min_depth, s->max_depth);
            
            s->a = (Agent **)malloc(s->n_terminals*sizeof(Agent *));
            for(i = 0; i < s->n_terminals; i++)
                s->a[i] = CreateAgent(s->n, _GP_);
        }
        
    }
    
    s->LB = (double *)malloc(s->n*sizeof(double));
    s->UB = (double *)malloc(s->n*sizeof(double));
    
    va_end(arg);
    
    return s;
}

/* It deallocates a search space
Parameters:
s: address of the search space to be deallocated
opt_id: identifier of the optimization technique */
void DestroySearchSpace(SearchSpace **s, int opt_id){
    SearchSpace *tmp = NULL;
    int i;
    
    tmp = *s;
    if(!tmp){
        fprintf(stderr,"\nSearch space not allocated @DestroySearchSpace.\n");
        exit(-1);
    }
    
    if(opt_id != _GP_){ /* GP uses a different structure than that of others */
    
        for(i = 0; i < tmp->m; i++)
            if(tmp->a[i]) DestroyAgent(&(tmp->a[i]), opt_id);
        free(tmp->a);
    
        switch (opt_id){
            case _PSO_:
            case _BA_:
            case _FPA_:
            case _FA_:
            case _GA_:
                if(tmp->g) free(tmp->g);
            break;
            default:
                fprintf(stderr,"\nInvalid optimization identifier @DestroySearchSpace.\n");
            break;       
        }
    }
    else{
        if(opt_id == _GP_){
            for(i = 0; i < tmp->m; i++)
                if(tmp->T[i]) DestroyTree(&(tmp->T[i]));
            if(tmp->T) free(tmp->T);
            
            for(i = 0; i < tmp->n_terminals; i++){
                if(tmp->a[i]) DestroyAgent(&(tmp->a[i]), opt_id);
                if(tmp->terminal[i]) free(tmp->terminal[i]);
            }
            if(tmp->a) free(tmp->a);
            if(tmp->terminal) free(tmp->terminal);
            
            for(i = 0; i < tmp->n_functions; i++)
                if(tmp->function[i]) free(tmp->function[i]);
            free(tmp->function);
            
            if(tmp->constant) free(tmp->constant);
        }
    }
    
    if(tmp->LB) free(tmp->LB);
    if(tmp->UB) free(tmp->UB);
    
    free(tmp);
}

/* It initializes an allocated search space
Parameters:
s: search space */
void InitializeSearchSpace(SearchSpace *s, int opt_id){
    if(!s){
        fprintf(stderr,"\nSearch space not allocated @InitializeSearchSpace.\n");
        exit(-1);
    }
    
    int i, j;
    
    switch (opt_id){
        case _PSO_:
        case _BA_:
        case _FPA_:
        case _FA_:
        case _GA_:
            for(i = 0; i < s->m; i++){
                for(j = 0; j < s->n; j++)
                s->a[i]->x[j] = (double)randinter((float)s->LB[j],(float) s->UB[j]);
            }
            
        break;
        case _GP_:
            for(i = 0; i < s->n_terminals; i++){
                for(j = 0; j < s->n; j++)
                s->a[i]->x[j] = (double)randinter((float)s->LB[j],(float) s->UB[j]);
            }
        break;
    }
}

/* It shows a search space
Parameters:
s: search space */
void ShowSearchSpace(SearchSpace *s){
    if(!s){
        fprintf(stderr,"\nSearch space not allocated @ShowSearchSpace.\n");
        exit(-1);
    }
    
    int i, j;
    fprintf(stderr,"\nSearch space with %d agents and %d decision variables\n", s->m, s->n);
    for(i = 0; i < s->m; i++){
        fprintf(stderr,"\nAgent %d-> ", i);
        for(j = 0; j < s->n; j++)
            fprintf(stderr,"x[%d]: %f   ", j, s->a[i]->x[j]);
        fprintf(stderr,"fitness value: %f", s->a[i]->fit);
    }
    fprintf(stderr,"\n-----------------------------------------------------\n");
}

/* It evaluates a search space
 * This function only evaluates each agent and sets its best fitness value,
 * as well as it sets the global best fitness value and agent.
Parameters:
s: search space
EvaluateFun: pointer to the function used to evaluate particles (agents)
arg: list of additional arguments */
void EvaluateSearchSpace(SearchSpace *s, int opt_id, prtFun Evaluate, va_list arg){
    if(!s){
        fprintf(stderr,"\nSearch space not allocated @EvaluateSearchSpace.\n");
        exit(-1);
    }
    
    int i, j;
    double f;
    va_list argtmp;
    
    va_copy(argtmp, arg);
    switch (opt_id){
        case _BA_:
        case _FPA_:
        case _FA_:
        case _GA_:
            for(i = 0; i < s->m; i++){
                f = Evaluate(s->a[i], arg); /* It executes the fitness function for agent i */
        
                if(f < s->a[i]->fit) /* It updates the fitness value */
                    s->a[i]->fit = f;
        
                if(s->a[i]->fit < s->gfit){ /* It updates the global best value and position */
                    s->gfit = s->a[i]->fit;
                    for(j = 0; j < s->n; j++)
                        s->g[j] = s->a[i]->x[j];
                }
        
                va_copy(arg, argtmp);
            }
        break;
        case _PSO_:
            for(i = 0; i < s->m; i++){
                f = Evaluate(s->a[i], arg); /* It executes the fitness function for agent i */
        
                if(f < s->a[i]->fit){ /* It updates the local best value and position */
                    s->a[i]->fit = f;    
                    for(j = 0; j < s->n; j++) 
                    s->a[i]->xl[j] = s->a[i]->x[j];
                }
            
                if(s->a[i]->fit < s->gfit){ /* It updates the global best value and position */
                    s->gfit = s->a[i]->fit;
                    for(j = 0; j < s->n; j++)
                        s->g[j] = s->a[i]->x[j];
                }
        
                va_copy(arg, argtmp);
            }
        break;
        default:
            fprintf(stderr,"\n Invalid optimization identifier @EvaluateSearchSpace.\n");
                     
        break;
    }
}
/**************************/

/* General-purpose functions */
/* It generates a random number uniformly distributed between low and high
Parameters:
low: lower bound
high: upper bound
This algorithm has been inspired by: http://www.cprogramming.com/tutorial/random.html */
double GenerateUniformRandomNumber(double low, double high){
    return randinter(low, high);
}

/* It generates a random number drawn from a Gaussian (normal) distribution
Parameters:
mean: mean of the distribution
variance: variance of the distribution */
double GenerateGaussianRandomNumber(double mean, double variance){
    return randGaussian(mean, variance);
}

/* It generates an n-dimensional array drawn from a Levy distribution
 * The formulation used here is based on the paper "Multiobjective Cuckoo Search for Design Optimization", X.-S. Yang and S. Deb, Computers & Operations Research, 2013.
Parameters:
n: dimension of the output array
beta: input parameter used in the formulation */
double *GenerateLevyDistribution(int n, double beta){
    double *L = NULL, sigma_u, sigma_v = 1;
    double *u = NULL, *v = NULL;
    int i;
    
    if(n < 1){
        fprintf(stderr,"Invalid input paramater @GenerateLevyDistribution.\n");
        return NULL;
    }
    
    L = (double *)malloc(n*sizeof(double));
    
    sigma_u = pow((tgamma(1+beta)*sin(M_PI*beta/2))/(tgamma((1+beta)/2)*beta*pow(2,(beta-1)/2)), 1/beta); /* Equation 16 */
    
    u = (double *)malloc(n*sizeof(double));
    v = (double *)malloc(n*sizeof(double));
    sigma_u = pow(sigma_u, 2);
    for(i = 0; i < n; i++){ /* It computes Equation 15 */
        u[i] = GenerateGaussianRandomNumber(0, sigma_u);
        v[i] = GenerateGaussianRandomNumber(0, sigma_v);
    }
    
    for(i = 0; i < n; i++)
        L[i] = 0.01*(u[i]/pow(fabs(v[i]), 1/beta)); /* It computes Equation 14 (part of it) */
    
    free(u);
    free(v);
    
    return L;
}

/* It computes the Euclidean distance between two n-dimensional arrays
Parameters:
x: n-dimension array
y: n-dimension array */
double EuclideanDistance(double *x, double *y, int n){
    double sum = 0.0;
    int i;
    
    for(i = 0; i < n; i++)
        sum+=pow(x[i]-y[i],2);
    
    return sqrt(sum);
}

/* It computes the compare function by agent's fitness, which is used on Quick Sort (qsort) */
int CompareForQSort(const void *a, const void *b){
    const Agent *ap = *(Agent **)a, *bp = *(Agent **)b;
    if (ap->fit < bp->fit) {
        return -1;
    }
    if (ap->fit > bp->fit)
        return 1;
    return 0;
}

/* It waives a comment in a model file
Parameters:
fp = file pointer */
void WaiveComment(FILE *fp){
    char c;
    
    fscanf(fp, "%c", &c);
    while((c != '\n') && (!feof(fp))) fscanf(fp, "%c", &c);
    
}

/* It loads a search space with parameters specified in a file
Parameters:
fileName: path to the file that contains the parameters of the search space
opt_id: identifier of the optimization technique */
SearchSpace *ReadSearchSpaceFromFile(char *fileName, int opt_id){
    FILE *fp = NULL;
    SearchSpace *s = NULL;
    int j, m, n, iterations;
    
    fp = fopen(fileName, "r");
    if(!fp){
        fprintf(stderr,"\nUnable to open file %s @ReadSearchSpaceFromFile.\n", fileName);
        return NULL;
    }
    
    fscanf(fp, "%d %d %d", &m, &n, &iterations);
    WaiveComment(fp);
        
    switch (opt_id){
        case _PSO_:
            s = CreateSearchSpace(m, n, _PSO_);
            s->iterations = iterations;
            fscanf(fp, "%lf %lf", &(s->c1), &(s->c2));
            WaiveComment(fp);
            fscanf(fp, "%lf %lf %lf", &(s->w), &(s->w_min), &(s->w_max));
            WaiveComment(fp);
        break;
        case _BA_:
            s = CreateSearchSpace(m, n, _BA_);
            s->iterations = iterations;
            fscanf(fp, "%lf %lf", &(s->f_min), &(s->f_max));
            WaiveComment(fp);
            fscanf(fp, "%lf %lf", &(s->A), &(s->r));
            WaiveComment(fp);
        break;
        case _FPA_:
            s = CreateSearchSpace(m, n, _FPA_);
            s->iterations = iterations;
            fscanf(fp, "%lf %lf", &(s->beta), &(s->p));
            WaiveComment(fp);
        break;
        case _FA_:
            s = CreateSearchSpace(m, n, _FA_);
            s->iterations = iterations;
            fscanf(fp, "%lf %lf %lf", &(s->alpha), &(s->beta_0), &(s->gamma));
            WaiveComment(fp);
        break;
        case _GA_:
            s = CreateSearchSpace(m, n, _GA_);
            s->iterations = iterations;
            fscanf(fp, "%lf %lf", &(s->pCrossOver), &(s->pMutate));
            WaiveComment(fp);
        break;
        default:
            fprintf(stderr,"\nInvalid optimization identifier @ReadSearchSpaceFromFile.\n");
        break;
    }
    
    for(j = 0; j < s->n; j++){
        fscanf(fp, "%lf %lf", &(s->LB[j]), &(s->UB[j]));
        WaiveComment(fp);
    }
    fclose(fp);
    
    return s;
}

/* It returns the identifier of the function used as input
Parameters:
s: string with the function node description */
int getFUNCTIONid(char *s){
    if(!strcmp(s,"SUM")) return _SUM_;
    else if(!strcmp(s,"SUB")) return _SUB_;
        else if(!strcmp(s,"MUL")) return _MUL_;
            else if(!strcmp(s,"DIV")) return _DIV_;
                else if(!strcmp(s,"EXP")) return _EXP_;
                    else if(!strcmp(s,"SQRT")) return _SQRT_;
			else if(!strcmp(s,"LOG")) return _LOG_;
			    else if(!strcmp(s,"ABS")) return _ABS_;
			    else{
				fprintf(stderr,"\nUndefined function @getFUNCTIONid.");
				exit(-1);
			    }
}
/**************************/

/* Tree-related functions */
/* It creates a tree node
Parameters:
value: content of the node
node_id: identifier of the node id, i.e. its position in the array of terminals, functions or constants
status: TERMINAL|FUNCTION/CONSTANT */
Node *CreateNode(char *value, int node_id, char status){
    Node *tmp = NULL;
   tmp = (Node *)malloc(sizeof(Node));
    
    if(!value){
        fprintf(stderr,"\nInvalid input @CreateNode.\n");
        return NULL;
    }

    tmp->id = node_id;
    tmp->left = tmp->right = tmp->parent = NULL;
    tmp->status = status;
    tmp->left_son = 1; /* by default, every node is a left node */
    tmp->elem = (char *)malloc((strlen(value)+1)*sizeof(char));
    strcpy(tmp->elem, value);
    
    return tmp;
}

/* It creates a random tree based on the GROW algorithm described in "Two Fast Tree-Creation Algorithms for Genetic Programming", S. Lukw, IEEE Transactions on Evolutionary Computation, 2000.
Parameters:
s: search space
dmin: minimum depth
dmax: maximum depth */
Node *GROW(SearchSpace *s, int min_depth, int max_depth){
    int i, aux, const_id;
    Node *tmp = NULL, *node = NULL;
    
    if(!s){
        fprintf(stderr,"\nSearch space not allocated @GROW.\n");
        return NULL;
    }
    
    if(min_depth == max_depth){
        aux = round(GenerateUniformRandomNumber(0, s->n_terminals-1));
	if(!strcmp(s->terminal[aux], "CONST")){
	    const_id = round(GenerateUniformRandomNumber(0, s->n_constants-1));
	    return CreateNode(s->terminal[aux], const_id, CONSTANT);
	}
        return CreateNode(s->terminal[aux], aux, TERMINAL);
    }
    else{
        aux = round(GenerateUniformRandomNumber(0, s->n_functions+s->n_terminals-1));
        if(aux >= s->n_functions){ /* If aux is a terminal node */
            aux = aux-s->n_functions;
	    if(!strcmp(s->terminal[aux], "CONST")){
                const_id = round(GenerateUniformRandomNumber(0, s->n_constants-1));
		tmp = CreateNode(s->terminal[aux], const_id, CONSTANT);
	    }
            else tmp = CreateNode(s->terminal[aux], aux, TERMINAL);
            return tmp;
        }
        else{ /* The new node is function one */
            node = CreateNode(s->function[aux], aux, FUNCTION);
            for(i = 0; i < N_ARGS_FUNCTION[getFUNCTIONid(s->function[aux])]; i++){
                tmp = GROW(s, min_depth+1, max_depth);
                if(!i) node->left = tmp;
                else{
                    node->right = tmp;
                    tmp->left_son = 0;
                }
                tmp->parent = node;
            }
            return node;
        }
    }
}

/* It deallocates a tree
Parameters:
T: pointer to the tree */
void DestroyTree(Node **T){
    if(*T){
        DestroyTree(&(*T)->left);
        DestroyTree(&(*T)->right);
	free((*T)->elem);
        free(*T);
        *T = NULL;
    }
}

/* It stores a tree in a text file (prefix mode)
Parameters:
s: search space
T: pointer to the tree
fileName: output file name */
void PrintTree2File(SearchSpace *s, Node *T, char *fileName){
    FILE *fp = NULL;
    
    if(!T){
        fprintf(stderr,"\nTree not allocated @PrintTree2File.\n");
        exit(-1);
    }
    
    fp = fopen(fileName, "a");
    PreFixPrintTree4File(s, T, fp);
    fprintf(fp,"\n");
    fclose(fp);
}

/* It performs a prefix search in tree and saves the nodes in a text file
Parameters:
s: search space
T: pointer to the tree
fileName: output file name */
void PreFixPrintTree4File(SearchSpace *s, Node *T, FILE *fp){
    if(T){
        if(T->status != TERMINAL) fprintf(fp,"(");
        if(T->status == CONSTANT) fprintf(fp,"%lf ", s->constant[T->id]);
	else fprintf(fp,"%s ", T->elem);
        PreFixPrintTree4File(s, T->left, fp);
        PreFixPrintTree4File(s, T->right, fp);
        if(T->status != TERMINAL) fprintf(fp,")");
    }
}

/* It runs a given tree and outputs its solution array
Parameters:
s: search space
T: current tree */
double *RunTree(SearchSpace *s, Node *T){
    double *x = NULL, *y = NULL, *out = NULL;
    int i;
    
    if(T){
	x = RunTree(s, T->left); /* It runs over the subtree on the left */
	y = RunTree(s, T->right); /* It runs over the subtree on the right */
	
	if(T->status == TERMINAL || T->status == CONSTANT){
	    if(T->status == CONSTANT){
		out = (double *)calloc(s->n,sizeof(double));
                for(i = 0; i < s->n; i++)
                    out[i] = s->constant[T->id];
	    }else{
                for(i = 0; i < s->n; i++)
                    out[i] = s->a[T->id]->x[i];
	    }
	    return out;
	}else{
	    if(!strcmp(T->elem,"SUM")) out = f_SUM_(x, y, s->n);
            else if(!strcmp(T->elem,"SUB")) out = f_SUB_(x, y, s->n);
                else if(!strcmp(T->elem,"MUL")) out = f_MUL_(x, y, s->n);
                    else if(!strcmp(T->elem,"DIV")) out = f_DIV_(x, y, s->n);
                        else if(!strcmp(T->elem,"EXP")){
                            if(x) out = f_EXP_(x, s->n);
                            else out = f_EXP_(y, s->n);
                        }
                        else if(!strcmp(T->elem,"SQRT")){
                            if(x) out = f_SQRT_(x, s->n);
                            else out = f_SQRT_(y, s->n);
                        }else if(!strcmp(T->elem,"LOG")){
                            if(x) out = f_LOG_(x, s->n);
                            else out = f_LOG_(y, s->n);
                        }
	    /* it deallocates the sons of the current one, since they have been used already */
	    if (x) free(x); 
	    if (y) free(y);
	    return out;
	}
    }else return NULL;
}
/***********************/
