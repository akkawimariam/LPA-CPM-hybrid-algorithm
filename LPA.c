#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "graph.h"
#include "performanceMeasure.h"

#define MAX_ITER 1000

void initializeLabels(int* labels, int V) {
    for (int i = 0; i < V; ++i) {
        labels[i] = i;
    }
}

void shuffle(int *array, int n) {
    if (n > 1) {
        for (int i = 0; i < n - 1; i++) {
            int j = i + rand() / (RAND_MAX / (n - i) + 1);
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

void labelPropagation(Graph* graph, int* labels) {
    int V = graph->V;
    int* node_order = (int*)malloc(V * sizeof(int));
    int loop_count = 0;
    int changed;

    for (int i = 0; i < V; ++i) {
        node_order[i] = i;
        labels[i] = i; // Initialize each node with its own label
    }
    printf("Initialized nodes with their own labels successfully.\n");
    srand(3000); // Fix the random seed for consistent results

    int* label_count = (int*)calloc(V, sizeof(int));
    if (label_count == NULL) {
        fprintf(stderr, "Memory allocation failed for label_count\n");
        exit(1);
    }

    while (1) {
        loop_count++;
        printf("Looping: %d\n", loop_count);
        changed = 0;

        shuffle(node_order, V);

        for (int k = 0; k < V; ++k) {
            if (k % 1000 == 0) {
                printf("Processed %d nodes...\n", k);
            }
            memset(label_count, 0, V * sizeof(int)); // Reset the array
            int i = node_order[k];
            Node* node = graph->array[i].head;

            // Count the labels of neighbors
            while (node) {
                label_count[labels[node->dest]]++;
                node = node->next;
            }

            // Find the label with the highest count
            int max_label = labels[i];
            int max_count = label_count[max_label];
            for (int j = 0; j < V; ++j) {
                if (label_count[j] > max_count) {
                    max_count = label_count[j];
                    max_label = j;
                }
            }

            // Update the label if needed
            if (labels[i] != max_label) {
                labels[i] = max_label;
                changed = 1;
            }
        }

        printf("Loop count: %d\n", loop_count);
        if (!changed || loop_count >= MAX_ITER) {
            printf("Max iterations reached or no changes made. Terminating.\n");
            break;
        }
    }

    free(label_count);
    free(node_order);
}


void printCommunities(int* labels, int V) {
    int* community_count = (int*)calloc(V, sizeof(int));
    for (int i = 0; i < V; ++i) {
        community_count[labels[i]]++;
    }

    int num_communities = 0;
    for (int i = 0; i < V; ++i) {
        if (community_count[i] > 0) {
            num_communities++;
        }
    }

    printf("\nNumber of Communities: %d\n", num_communities);

    int counter=1;
    for (int i = 0; i < V; ++i) {
        if (community_count[i] > 0) {
            printf("Community %d: %d nodes\n", counter, community_count[i]);
            counter++;
        }
    }
    printf("\n");

    free(community_count);
}

int main() {
    int directed = 0;
    int V = 2888; // Number of vertices

    // twitter lists V=23370 directed=1 seed=2000
    const char* filename = "C:datasets\\outego-twitter.txt";
    Graph* graph = createGraphFromFile(filename, V, directed);


//    //SNAP facebook V=4039 directed=0 seed=3000
//     const char* filename = "C:datasets\\facebook_combined.txt";
//     Graph* graph = createGraphFromFile(filename, V, directed);

//    //facebook lists V=2888 directed=0 seed=3000
//     const char* filename = "C:datasets\\outego-facebook.txt";
//     Graph* graph = createGraphFromFile(filename, V, directed);

    if (!graph) {
        fprintf(stderr, "Memory allocation failed for graph\n");
        exit(1);
    }
    printf("Graph successfully created with %d vertices.\n", V);

    // Initialize labels for LPA
    int* labels = (int*)malloc(V * sizeof(int));
    if (!labels) {
        fprintf(stderr, "Memory allocation failed for labels\n");
        exit(1);
    }

    printf("Memory allocation for labels successful.\n");

    // Initialize labels to -1
    for (int i = 0; i < V; ++i) {
        labels[i] = -1;
    }

    clock_t start, end;
    double cpu_time_used;
    start = clock();

    // Run the LPA algorithm
    printf("Running Label Propagation Algorithm (LPA)...\n");
    labelPropagation(graph, labels);
    printf("LPA completed.\n");

    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    // Print the number of communities and the communities
    printCommunities(labels, V);

    // Example usage of performance measures
    double modularity = calculateModularity(graph, labels, V, graph->E, directed);
    printf("Modularity: %f\n", modularity);

    double conductance = calculateConductance(graph, labels, V, directed);
    printf("Conductance: %f\n", conductance);

    double coverage = calculateCoverage(graph, labels, V, graph->E, directed);
    printf("Coverage: %f\n", coverage);

    printf("Execution Time: %f seconds\n", cpu_time_used);

    free(labels);
    freeGraph(graph);

    return 0;
}