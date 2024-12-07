#ifndef PERFORMANCE_MEASURE_H
#define PERFORMANCE_MEASURE_H

#include "graph.h"

double calculateModularity(Graph* graph, int* community, int V, int E, int directed);
double calculateConductance(Graph* graph, int* labels, int V, int directed);
double calculateCoverage(Graph* graph, int* community, int V, int E, int directed);

#endif // PERFORMANCE_MEASURE_H