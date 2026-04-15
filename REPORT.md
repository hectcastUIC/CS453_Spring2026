# REPORT

## Overview

This project implements distributed graph algorithms using MPI on a partitioned graph.

Each MPI rank owns a subset of nodes. Local edges are processed within the same rank, while edges that cross partitions require MPI communication.

Implemented algorithms:

* Leader election
* Shortest path (distributed Dijkstra-style iterative relaxation)


## Approach

The main idea is to split the graph across multiple MPI ranks and simulate distributed computation.

Each rank:

* stores a subset of nodes
* updates local values
* exchanges information with other ranks when needed

The goal is to mimic how large graphs are processed across multiple machines.


## Graph Generation

Graphs are generated using NetGameSim with a fixed seed.

Command:

```
./tools/graph_export/run.sh configs/netgamesim.conf outputs/graph_ngs.txt
```

This process:

* generates a graph using NetGameSim
* exports it as a DOT file
* converts it into a weighted graph
* saves the seed for reproducibility


## Implementation Details

### Partitioning

The graph is partitioned by assigning nodes evenly across ranks. Each rank only updates its own nodes.

### Leader Election

Each node starts with its own ID as a candidate. During each round, nodes exchange values with neighbors and update to the maximum value seen. After enough rounds, all nodes agree on the same leader.

### Shortest Path

Each node keeps a distance value. Nodes repeatedly exchange distance updates with neighbors and apply relaxation. Over time, distances converge to the correct shortest paths from the source.


## Experimental Setup

Experiments were run using:

* 4 ranks and 8 ranks
* graphs generated with a fixed seed
* multiple rounds to allow convergence


## Hypothesis

* Leader election should converge to the node with the highest ID
* Shortest path should produce correct distances from the source
* Increasing the number of ranks will increase communication cost


## Expected Results

* All nodes agree on the same leader
* Distances match expected shortest path values
* More ranks lead to more messages and communication overhead


## Actual Results

Leader election:

* All nodes agreed on the same leader (highest node ID)
* Convergence occurred within the given number of rounds

Shortest path:

* All nodes produced correct shortest path distances
* Results were consistent across different runs

Metrics:

* Runtime and message count increased with more ranks
* Communication overhead was noticeable when using 8 ranks


## Insights

* Partitioning affects communication cost significantly
* More ranks do not always improve performance due to overhead
* Using a fixed seed makes experiments reproducible and easier to debug
* MPI communication is the main bottleneck for distributed graph algorithms


## Notes

* Graphs are generated using NetGameSim with a fixed seed
* The system was tested with 2, 4, and 8 ranks
* `--oversubscribe` is used for higher ranks on a local machine
