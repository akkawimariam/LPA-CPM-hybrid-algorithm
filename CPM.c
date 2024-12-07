#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "graph.h"
#include "performanceMeasure.h"
#include <stdbool.h>

typedef struct {
    int* vertices;
    int size;
} Clique;


bool isNeighbor(Graph* graph, int u, int v) {
    Node* neighbor = graph->array[u].head;
    while (neighbor) {
        if (neighbor->dest == v) return true;
        neighbor = neighbor->next;
    }
    return false;
}

void findTriangles(Graph* graph, int* labels, int* clique_count, Clique** cliques) {
    printf("Running find triangles...\n");
    int V = graph->V;

    for (int u = 0; u < V; u++) {
        Node* neighbor = graph->array[u].head;
        while (neighbor) {
            int v = neighbor->dest;
            if (v > u) { // Avoid duplicate triangle counting
                Node* neighbor2 = graph->array[v].head;
                while (neighbor2) {
                    int w = neighbor2->dest;
                    if (w > v && isNeighbor(graph, u, w)) {
                        //printf("Triangle found: %d, %d, %d\n", u, v, w);
                        labels[u] = *clique_count;
                        labels[v] = *clique_count;
                        labels[w] = *clique_count;

                        cliques[*clique_count] = (Clique*)malloc(sizeof(Clique));
                        cliques[*clique_count]->vertices = (int*)malloc(3 * sizeof(int));
                        cliques[*clique_count]->vertices[0] = u;
                        cliques[*clique_count]->vertices[1] = v;
                        cliques[*clique_count]->vertices[2] = w;
                        cliques[*clique_count]->size = 3;

                        (*clique_count)++;
                    }
                    neighbor2 = neighbor2->next;
                }
            }
            neighbor = neighbor->next;
        }
    }
}

void findCliques(Graph* graph, int k, int* labels, Clique** cliques, int* clique_count) {
    printf("Finding cliques of size %d...\n", k);

    if (k == 3) {
        findTriangles(graph, labels, clique_count, cliques);
    } else {
        //findKCliques(graph, k, cliques, clique_count);
    }

    printf("Cliques found: %d\n", *clique_count);
    for (int i = 0; i < *clique_count; ++i) {
        if (cliques[i] != NULL) {
            //printf("Clique %d: ", i);
            // for (int j = 0; j < cliques[i]->size; ++j) {
            //     printf("%d ", cliques[i]->vertices[j]);
            // }
            //printf("\n");
        } else {
            printf("Clique %d is NULL\n", i);
        }
    }
}

void addCliqueEdge(Graph* clique_graph, int i, int j) {
    addEdge(clique_graph, i, j);
}
bool hasValidClique(Clique* clique, int k) {
    return clique != NULL && clique->vertices != NULL && clique->size == k;
}

Graph* buildCliqueGraph(Clique** cliques, int clique_count, int k, int directed) {
    printf("Building clique graph...\n");

    // Step 1: Create the clique graph
    Graph* clique_graph = createGraph(clique_count, directed);

    // Step 2: Loop through cliques and check each for valid pointer and vertices
    for (int i = 0; i < clique_count; ++i) {
        if (!hasValidClique(cliques[i], k)) {
            continue;  // Skip invalid clique
        }

        for (int j = i + 1; j < clique_count; ++j) {
            if (!hasValidClique(cliques[j], k)) {
                continue;  // Skip invalid clique
            }

            int shared_vertices = 0;

            // Step 3: Use hash sets for fast lookup to find shared vertices
            for (int vi = 0; vi < k; ++vi) {
                for (int vj = 0; vj < k; ++vj) {
                    if (cliques[i]->vertices[vi] == cliques[j]->vertices[vj]) {
                        shared_vertices++;
                    }
                }
            }

            // Step 4: Add edge if necessary (based on shared vertices)
            if (shared_vertices >= k - 1) {
                addCliqueEdge(clique_graph, i, j);
            }
        }
    }

    printf("Clique graph built\n");
    return clique_graph;
}

void DFSUtil(Graph* graph, int v, int* visited, int* component, int component_id) {
    visited[v] = 1;
    component[v] = component_id;
    Node* node = graph->array[v].head;
    while (node) {
        if (!visited[node->dest]) {
            DFSUtil(graph, node->dest, visited, component, component_id);
        }
        node = node->next;
    }
}

void decomposeGraph(Graph* graph, int* component, int* component_count) {
    printf("Decomposing graph into connected components...\n");
    int V = graph->V;
    int* visited = (int*)calloc(V, sizeof(int));
    *component_count = 0;

    for (int v = 0; v < V; ++v) {
        if (!visited[v]) {
            DFSUtil(graph, v, visited, component, *component_count);
            (*component_count)++;
        }
    }
    printf("Graph decomposed.\n");
    free(visited);
}

void mapCliquesToNodes(int* labels, int V, Clique** cliques, int num_cliques, int* component_labels) {
    printf("Mapping cliques to original graph...\n");

    // Map each node in the cliques to its component
    for (int i = 0; i < num_cliques; ++i) {
        int component = component_labels[i];  // The component of the current clique
        Clique* clique = cliques[i];
        for (int j = 0; j < clique->size; ++j) {
            labels[clique->vertices[j]] = component;
        }
    }

    printf("Cliques mapped.\n");
}

void cliqueCommunity(Graph* graph, int k, int* labels, int directed) {
    printf("Running clique community detection...\n");
    Clique** cliques = (Clique**)malloc(graph->V * sizeof(Clique*));
    int clique_count = 0;
    findCliques(graph, k, labels, cliques, &clique_count);

    Graph* clique_graph = buildCliqueGraph(cliques, clique_count, k, directed);

    int* component = (int*)malloc(clique_count * sizeof(int));
    int component_count = 0;
    decomposeGraph(clique_graph, component, &component_count);

    // Print the component array to verify component labels
    printf("Component labels:\n");
    for (int i = 0; i < clique_count; ++i) {
        printf("Clique %d -> Component %d\n", i, component[i]);
    }

   // Call the mapCliquesToNodes function
    mapCliquesToNodes(labels, graph->V, cliques, clique_count, component);

    // Free allocated memory
    for (int i = 0; i < clique_count; ++i) {
        free(cliques[i]->vertices);
        free(cliques[i]);
    }
    free(cliques);
    free(component);
    freeGraph(clique_graph);
    printf("Clique community detection completed.\n");
}

void printCommunities(int* labels, int V) {
    printf("Printing communities...\n");

    // Step 1: Determine the maximum label value
    int max_label = -1;
    for (int i = 0; i < V; ++i) {
        if (labels[i] >= 0 && labels[i] > max_label) {
            max_label = labels[i];
        }
    }

    if (max_label == -1) {
        printf("No communities found.\n");
        return;
    }

    // Step 2: Allocate memory for community counts
    int* community_count = (int*)calloc(max_label + 1, sizeof(int));
    if (!community_count) {
        fprintf(stderr, "Memory allocation failed for community_count\n");
        return;
    }

    // Step 3: Count nodes in each community
    for (int i = 0; i < V; ++i) {
        if (labels[i] >= 0) {
            community_count[labels[i]]++;
        }
    }

    // Step 4: Count number of communities and print results
    int num_communities = 0;
    for (int i = 0; i <= max_label; ++i) {
        if (community_count[i] > 0) {
            num_communities++;
        }
    }

    printf("\nNumber of Communities: %d\n", num_communities);

    int counter = 1;
    // for (int i = 0; i <= max_label; ++i) {
    //     if (community_count[i] > 0) {
    //         printf("Community %d: %d nodes\n", counter++, community_count[i]);
    //     }
    // }

    // printf("\n");

    // Step 5: Free allocated memory
    free(community_count);
    printf("Communities printed.\n");
}

int main() {
    int directed = 0;
    int V = 2888; // Number of vertices

    // // SNAP dataset facebook V=4039 directed=0
    // const char* filename = "C:\\lau\\design of algorithms\\projects\\datasets\\facebook_combined.txt";
    // printf("Reading graph from file: %s\n", filename);
    // Graph* graph = createGraphFromFile(filename, V, directed);

    //facebook lists V=2888 directed=0 seed=3000
    const char* filename = "C:datasets\\outego-facebook.txt";
    printf("Reading graph from file: %s\n", filename);
    Graph* graph = createGraphFromFile(filename, V, directed);

    if (!graph) {
        fprintf(stderr, "Memory allocation failed for graph\n");
        exit(1);
    }
    printf("Graph successfully created with %d vertices.\n", V);

    int k = 3; // Size of cliques

    // Measure the execution time of the CPM algorithm
    clock_t start, end;
    double cpu_time_used;
    start = clock();

    // Run the CPM algorithm
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

    printf("Labels initialized to -1.\n");

    // Run the CPM algorithm
    printf("Running Clique Percolation Method (CPM) with k=%d...\n", k);
    cliqueCommunity(graph, k, labels, directed);
    printf("CPM completed.\n");

    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    // Print the number of communities and the communities
    printCommunities(labels, V);

    printf("done printing.........\n");

    // // Print the labels array
    // for (int i = 0; i < V; ++i) {
    //     printf("Node %d -> Label %d\n", i, labels[i]);
    // }

    // Calculate performance measures
    printf("Calculating modularity...\n");
    double modularity = calculateModularity(graph, labels, V, graph->E, directed);
    printf("Modularity calculated: %f\n", modularity);

    printf("Calculating conductance...\n");
    double conductance = calculateConductance(graph, labels, V, directed);
    printf("Conductance calculated: %f\n", conductance);

    printf("Calculating coverage...\n");
    double coverage = calculateCoverage(graph, labels, V, graph->E, directed);
    printf("Coverage calculated: %f\n", coverage);

    // Print the results
    printf("Modularity: %f\n", modularity);
    printf("Conductance: %f\n", conductance);
    printf("Coverage: %f\n", coverage);

    // Print the execution time
    printf("Execution Time: %f seconds\n", cpu_time_used);

    free(labels);
    freeGraph(graph);

    return 0;
}