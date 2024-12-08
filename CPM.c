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
void findTriangles(Graph* graph, int* labels, int* clique_count, Clique*** cliques, int* cliques_size) {
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
                        printf("Triangle found: %d, %d, %d\n", u, v, w);

                        // Check if we need to resize the cliques array
                        if (*clique_count >= *cliques_size) {
                            *cliques_size *= 2;
                            *cliques = (Clique**)realloc(*cliques, (*cliques_size) * sizeof(Clique*));
                            if (!*cliques) {
                                printf("Memory allocation failed for cliques array\n");
                                exit(1);
                            }
                        }

                        // Allocate memory for the new clique
                        (*cliques)[*clique_count] = (Clique*)malloc(sizeof(Clique));
                        if (!(*cliques)[*clique_count]) {
                            printf("Memory allocation failed for cliques[%d]\n", *clique_count);
                            exit(1);
                        }
                        (*cliques)[*clique_count]->vertices = (int*)malloc(3 * sizeof(int));
                        if (!(*cliques)[*clique_count]->vertices) {
                            printf("Memory allocation failed for cliques[%d]->vertices\n", *clique_count);
                            exit(1);
                        }

                        // Assign vertices to the clique
                        (*cliques)[*clique_count]->vertices[0] = u;
                        (*cliques)[*clique_count]->vertices[1] = v;
                        (*cliques)[*clique_count]->vertices[2] = w;
                        (*cliques)[*clique_count]->size = 3;

                        // Increment the clique count
                        (*clique_count)++;
                    }
                    neighbor2 = neighbor2->next;
                }
            }
            neighbor = neighbor->next;
        }
    }
}

void BronKerboschPivot(Graph* graph, int* R, int* P, int* X, int k, Clique*** cliques, int* clique_count, int* cliques_size) {
    if (k == 0) {
        // Found a clique
        if (*clique_count >= *cliques_size) {
            *cliques_size *= 2;
            *cliques = (Clique**)realloc(*cliques, (*cliques_size) * sizeof(Clique*));
            if (!*cliques) {
                printf("Memory allocation failed for cliques array\n");
                exit(1);
            }
        }

        (*cliques)[*clique_count] = (Clique*)malloc(sizeof(Clique));
        if (!(*cliques)[*clique_count]) {
            printf("Memory allocation failed for cliques[%d]\n", *clique_count);
            exit(1);
        }
        (*cliques)[*clique_count]->vertices = (int*)malloc(k * sizeof(int));
        if (!(*cliques)[*clique_count]->vertices) {
            printf("Memory allocation failed for cliques[%d]->vertices\n", *clique_count);
            exit(1);
        }

        for (int i = 0; i < k; i++) {
            (*cliques)[*clique_count]->vertices[i] = R[i];
        }
        (*cliques)[*clique_count]->size = k;
        (*clique_count)++;
        return;
    }

    // Choose pivot
    int pivot = -1;
    int max_degree = -1;
    for (int i = 0; i < graph->V; i++) {
        if (P[i]) {
            // Calculate the degree of node i by traversing its adjacency list
            int degree = 0;
            Node* node = graph->array[i].head;
            while (node) {
                degree++;
                node = node->next;
            }

            // If this node has a higher degree, select it as the pivot
            if (degree > max_degree) {
                max_degree = degree;
                pivot = i;
            }
        }
    }

    int* newP = (int*)malloc(graph->V * sizeof(int));
    int* newX = (int*)malloc(graph->V * sizeof(int));

    for (int u = 0; u < graph->V; u++) {
        if (P[u] && !isNeighbor(graph, u, pivot)) {
            // Add to R
            R[k] = u;

            // Recalculate newP and newX
            int newP_size = 0;
            int newX_size = 0;
            for (int i = 0; i < graph->V; i++) {
                if (P[i] && isNeighbor(graph, u, i)) {
                    newP[newP_size++] = i;
                }
                if (X[i] && isNeighbor(graph, u, i)) {
                    newX[newX_size++] = i;
                }
            }

            // Recurse
            BronKerboschPivot(graph, R, newP, newX, k + 1, cliques, clique_count, cliques_size);

            // Remove u from P and add to X
            P[u] = 0;
            X[u] = 1;
        }
    }

    free(newP);
    free(newX);
}


void findCliques(Graph* graph, int k, int* labels, Clique*** cliques, int* clique_count, int* cliques_size) {
    printf("Finding cliques of size %d...\n", k);

    // if (k == 3) {
    //     findTriangles(graph, labels, clique_count, cliques, cliques_size);
    // } else {
        int* R = (int*)malloc(graph->V * sizeof(int));
        int* P = (int*)malloc(graph->V * sizeof(int));
        int* X = (int*)malloc(graph->V * sizeof(int));

        for (int i = 0; i < graph->V; i++) {
            P[i] = 1;
            X[i] = 0;
        }

        BronKerboschPivot(graph, R, P, X, 0, cliques, clique_count, cliques_size);

        free(R);
        free(P);
        free(X);

    printf("Cliques found: %d\n", *clique_count);
    for (int i = 0; i < *clique_count; ++i) {
        if ((*cliques)[i] != NULL) {
            // //printf("Clique %d: ", i);
            // for (int j = 0; j < (*cliques)[i]->size; ++j) {
            //     printf("%d ", (*cliques)[i]->vertices[j]);
            // }
            // printf("\n");
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
    int cliques_size = graph->V;
    Clique** cliques = (Clique**)malloc(cliques_size * sizeof(Clique*));
    int clique_count = 0;
    findCliques(graph, k, labels, &cliques, &clique_count, &cliques_size);

    Graph* clique_graph = buildCliqueGraph(cliques, clique_count, k, directed);

    int* component = (int*)malloc(clique_count * sizeof(int));
    int component_count = 0;
    decomposeGraph(clique_graph, component, &component_count);

    // Print the component array to verify component labels
    // printf("Component labels:\n");
    // for (int i = 0; i < clique_count; ++i) {
    //     printf("Clique %d -> Component %d\n", i, component[i]);
    // }

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
    for (int i = 0; i <= max_label; ++i) {
        if (community_count[i] > 0) {
            printf("Community %d: %d nodes\n", counter++, community_count[i]);
        }
    }

    printf("\n");

    // Step 5: Free allocated memory
    free(community_count);
    printf("Communities printed.\n");
}

int main() {
    int directed = 0;
    int V = 2890; // Number of vertices

    // // SNAP dataset facebook V=4039 directed=0
    // const char* filename = "C:datasets\\facebook_combined.txt";
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
    
    cliqueCommunity(graph, k, labels, directed);
    printf("CPM completed.\n");

    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    // Print the number of communities and the communities
    printCommunities(labels, V);

    // Calculate performance measures
     double modularity = calculateModularity(graph, labels, V, graph->E, directed);
    double conductance = calculateConductance(graph, labels, V, directed);
    double coverage = calculateCoverage(graph, labels, V, graph->E, directed);

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