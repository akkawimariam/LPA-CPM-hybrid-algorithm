// graph.c
#include <stdio.h>
#include <stdlib.h>
#include "graph.h"

Graph* createGraph(int V, int directed) {
    Graph* graph = (Graph*) malloc(sizeof(Graph));
    graph->V = V;
    graph->E = 0;
    graph->directed = directed;
    graph->array = (AdjList*) malloc(V * sizeof(AdjList));
    for (int i = 0; i < V; ++i) {
        graph->array[i].head = NULL;
    }
    return graph;
}
void addEdge(Graph* graph, int src, int dest) {
    Node* newNode = (Node*) malloc(sizeof(Node));
    if (!newNode) {
        fprintf(stderr, "Memory allocation failed for new node\n");
        exit(1);
    }
    newNode->dest = dest;
    newNode->next = graph->array[src].head;
    graph->array[src].head = newNode;

    if (!graph->directed) {
        newNode = (Node*) malloc(sizeof(Node));
        if (!newNode) {
            fprintf(stderr, "Memory allocation failed for new node\n");
            exit(1);
        }
        newNode->dest = src;
        newNode->next = graph->array[dest].head;
        graph->array[dest].head = newNode;
    }

    graph->E++;
}

Graph* createGraphFromFile(const char* filename, int V, int directed) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Unable to open file %s\n", filename);
        exit(1);
    }
    printf("File opened successfully.\n");

    Graph* graph = createGraph(V, directed);
    if (!graph) {
        fprintf(stderr, "Memory allocation failed for graph structure\n");
        fclose(file);
        exit(1);
    }
    printf("Graph structure created successfully.\n");

    int src, dest;
    int edge_count = 0;
    while (fscanf(file, "%d %d", &src, &dest) != EOF) {
        addEdge(graph, src, dest);
        edge_count++;
        // if (edge_count % 100000 == 0) {
        //     printf("%d edges added...\n", edge_count);
        // }
    }
    printf("Total %d edges added successfully.\n", edge_count);

    fclose(file);
    printf("File closed successfully.\n");
    return graph;
}

void freeGraph(Graph* graph) {
    for (int v = 0; v < graph->V; ++v) {
        Node* node = graph->array[v].head;
        while (node) {
            Node* temp = node;
            node = node->next;
            free(temp);
        }
    }
    free(graph->array);
    free(graph);
}


int compare(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

Graph* createGraphFromFileWithMapping(const char* filename, int V, int directed) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Unable to open file %s\n", filename);
        exit(1);
    }
    printf("File opened successfully.\n");

    Graph* graph = createGraph(V, directed);
    if (!graph) {
        fprintf(stderr, "Memory allocation failed for graph structure\n");
        fclose(file);
        exit(1);
    }
    printf("Graph structure created successfully.\n");

    int* node_ids = (int*)malloc(2 * V * sizeof(int));
    int node_count = 0;

    int src, dest;
    while (fscanf(file, "%d %d", &src, &dest) != EOF) {
        node_ids[node_count++] = src;
        node_ids[node_count++] = dest;
    }

    qsort(node_ids, node_count, sizeof(int), compare);

    int unique_count = 0;
    for (int i = 1; i < node_count; ++i) {
        if (node_ids[i] != node_ids[unique_count]) {
            node_ids[++unique_count] = node_ids[i];
        }
    }
    unique_count++;

    rewind(file);

    int edge_count = 0;
    while (fscanf(file, "%d %d", &src, &dest) != EOF) {
        int* src_ptr = (int*)bsearch(&src, node_ids, unique_count, sizeof(int), compare);
        int* dest_ptr = (int*)bsearch(&dest, node_ids, unique_count, sizeof(int), compare);

        int src_index = src_ptr - node_ids;
        int dest_index = dest_ptr - node_ids;

        printf("Read edge: %d -> %d (mapped to %d -> %d)\n", src, dest, src_index, dest_index);
        addEdge(graph, src_index, dest_index);
        edge_count++;
        // if (edge_count % 100000 == 0) {
        //     printf("%d edges added...\n", edge_count);
        // }
    }
    printf("Total %d edges added successfully.\n", edge_count);

    fclose(file);
    printf("File closed successfully.\n");
    free(node_ids);
    return graph;
}


