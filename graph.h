#ifndef GRAPH_H
#define GRAPH_H

typedef struct Node {
    int dest;
    struct Node* next;
} Node;

typedef struct AdjList {
    Node* head;
} AdjList;

typedef struct Graph {
    int V;
    int E;
    int directed;
    AdjList* array;
} Graph;

Graph* createGraph(int V, int directed);
void addEdge(Graph* graph, int src, int dest);
Graph* createGraphFromFile(const char* filename, int V, int directed);
Graph* createGraphFromFileWithMapping(const char* filename, int V, int directed);
void freeGraph(Graph* graph);

#endif // GRAPH_H