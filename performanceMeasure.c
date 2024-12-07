#include <stdio.h>
#include <stdlib.h>
#include "performanceMeasure.h"
#include "graph.h"

double calculateModularity(Graph* graph, int* community, int V, int E, int directed) {
    double modularity = 0.0;
    int* communityEdges = (int*)calloc(V, sizeof(int));  // Tracks internal edges for each community
    int* totalDegree = (int*)calloc(V, sizeof(int));     // Tracks total degree for each community

    // Calculate communityEdges and totalDegree
    for (int v = 0; v < V; ++v) {
        Node* node = graph->array[v].head;
        while (node) {
            // If node v and node->dest are in the same community, update community edges
            if (community[v] == community[node->dest]) {
                communityEdges[community[v]]++;  // Community edge count
            }
            totalDegree[v]++;  // Degree of node v (not community)
            node = node->next;
        }
    }
    // Check if there is only one community
    int unique_communities = 0;
    for (int i = 0; i < V; ++i) {
        if (totalDegree[i] > 0) {
            unique_communities++;
        }
    }
    if (unique_communities <= 1) {
        free(communityEdges);
        free(totalDegree);
        return 0.0;  // Modularity is zero when there is only one community
    }
    // Calculate modularity for each community
    for (int i = 0; i < V; ++i) {
        if (totalDegree[i] > 0) {
            double eii = (double)communityEdges[i] / (directed ? E : (2 * E)); // Proportion of edges within the community
            double ai = (double)totalDegree[i] / (directed ? E : (2 * E)); // Proportion of total degree
            modularity += eii - ai * ai; // Modularity contribution from community i
        }
    }
    // Clean up memory
    free(communityEdges);
    free(totalDegree);

    return modularity;
}


double calculateConductance(Graph* graph, int* labels, int V, int directed) {
    double conductance = 0.0;
    int* communityEdges = (int*)calloc(V, sizeof(int));  // Internal edges
    int* boundaryEdges = (int*)calloc(V, sizeof(int));  // Boundary edges
    int* totalDegree = (int*)calloc(V, sizeof(int));     // Node degree

    if (!communityEdges || !boundaryEdges || !totalDegree) {
        fprintf(stderr, "Memory allocation failed for conductance calculation\n");
        free(communityEdges);
        free(boundaryEdges);
        free(totalDegree);
        return 0.0;
    }

    printf("Calculating community and boundary edges...\n");
    for (int v = 0; v < V; ++v) {
        Node* node = graph->array[v].head;
        while (node) {
            totalDegree[v]++;  // Count degree of node v

            if (labels[v] == labels[node->dest]) {
                communityEdges[labels[v]]++;  // Internal edge
            } else {
                boundaryEdges[labels[v]]++;  // Boundary edge
            }

            // For undirected graphs, count the boundary edge for the destination node as well
            if (!directed && labels[v] != labels[node->dest]) {
                boundaryEdges[labels[node->dest]]++;  // Add boundary edge for the destination node
            }

            node = node->next;
        }
    }

    printf("Calculating conductance for each community...\n");
    int num_communities = 0;
    for (int i = 0; i < V; ++i) {
        if (totalDegree[i] > 0) {
            num_communities++;
            double internal = communityEdges[i];
            double boundary = boundaryEdges[i];
            double degree = totalDegree[i];
            printf("Community %d: Internal edges = %f, Boundary edges = %f, Degree = %f\n", i, internal, boundary, degree);
            if (boundary > 0) {
                conductance += boundary / (internal + boundary);
            }
        }
    }

    // Average conductance over all communities
    if (num_communities > 0) {
        conductance /= num_communities;
    }

    // Clean up memory
    free(communityEdges);
    free(boundaryEdges);
    free(totalDegree);

    printf("Conductance calculated: %f\n", conductance);
    return conductance;
}

double calculateCoverage(Graph* graph, int* community, int V, int E, int directed) { 
    int intraCommunityEdges = 0;

    for (int v = 0; v < V; ++v) {
        Node* node = graph->array[v].head;
        while (node) {
            // For directed graphs, count the edge from v to node->dest
            // For undirected graphs, count the edge from v to node->dest only if v < node->dest
            if (community[v] == community[node->dest]) {
                if (directed || v < node->dest) {
                    intraCommunityEdges++;
                }
            }
            node = node->next;
        }
    }

    double coverage = (double)intraCommunityEdges / (directed ? E : (2 * E));
    return coverage;
}
