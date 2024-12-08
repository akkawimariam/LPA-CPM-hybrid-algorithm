#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int dest;
    struct Node* next;
} Node;

typedef struct Graph {
    int V;
    Node** array;
} Graph;

// Function to create a new node
Node* createNode(int dest) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->dest = dest;
    newNode->next = NULL;
    return newNode;
}

// Function to create a graph
Graph* createGraph(int V) {
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    graph->V = V;
    graph->array = (Node**)malloc(V * sizeof(Node*));
    for (int i = 0; i < V; ++i) {
        graph->array[i] = NULL;
    }
    return graph;
}

// Function to add an edge to the graph
void addEdge(Graph* graph, int src, int dest) {
    Node* newNode = createNode(dest);
    newNode->next = graph->array[src];
    graph->array[src] = newNode;

    // Since the graph is undirected, add an edge from dest to src also
    newNode = createNode(src);
    newNode->next = graph->array[dest];
    graph->array[dest] = newNode;
}

// Function to read the data file and construct the graph
Graph* createGraphFromFile(const char* filename, int V) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file\n");
        return NULL;
    }

    Graph* graph = createGraph(V);
    int src, dest;
    while (fscanf(file, "%d %d", &src, &dest) != EOF) {
        addEdge(graph, src, dest);
    }

    fclose(file);
    return graph;
}

// Function to analyze the node degrees
void analyzeNodeDegrees(Graph* graph) {
    int* degrees = (int*)calloc(graph->V, sizeof(int));
    for (int i = 0; i < graph->V; ++i) {
        Node* node = graph->array[i];
        while (node) {
            degrees[i]++;
            node = node->next;
        }
    }

    int isolated_nodes = 0;
    for (int i = 0; i < graph->V; ++i) {
        if (degrees[i] == 0) {
            isolated_nodes++;
        }
    }

    printf("Total nodes: %d\n", graph->V);
    printf("Isolated nodes: %d\n", isolated_nodes);
    printf("Percentage of isolated nodes: %.2f%%\n", (isolated_nodes / (double)graph->V) * 100);

    free(degrees);
}

int main() {
    int V = 4039; // Number of vertices
    const char* filename = "C:datasets\\facebook_combined.txt"; // Replace with your data file name

    Graph* graph = createGraphFromFile(filename, V);
    if (graph) {
        analyzeNodeDegrees(graph);
    }

    return 0;
}