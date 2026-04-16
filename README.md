# CS 453 MPI Graph Project

## Overview

This project implements distributed graph algorithms using MPI.

Each process (rank) owns a subset of nodes in the graph. Local computation is done within the rank, and communication across partitions is handled using MPI.

Algorithms:

* Leader election
* Shortest path (Dijkstra-style)


## Requirements

Run on Linux / WSL (Ubuntu recommended).

Install dependencies:

```bash
sudo apt update
sudo apt install -y build-essential cmake openmpi-bin libopenmpi-dev openjdk-17-jdk scala
```

Install sbt if not installed (required for NetGameSim).


## Build

```bash
cmake -S mpi_runtime -B build
cmake --build build
```


## Generate Graph (NetGameSim)

```bash
./tools/graph_export/run.sh configs/netgamesim.conf outputs/graph_ngs.txt
```

This step:

* runs NetGameSim to generate a graph
* exports the graph in DOT format
* converts it into a weighted graph
* saves seed and metadata for reproducibility

Note:

* The script may print warnings after graph generation, but the output graph file will still be created.


## Partition Graph

```bash
./tools/partition/run.sh outputs/graph_ngs.txt --ranks 4 --out outputs/part.txt
```

Each rank is assigned a subset of nodes.


## Run Leader Election

```bash
mpirun -n 4 ./build/ngs_mpi \
  --algo leader \
  --graph outputs/graph_ngs.txt \
  --part outputs/part.txt \
  --rounds 20
```


## Run Shortest Path

```bash
mpirun -n 4 ./build/ngs_mpi \
  --algo dijkstra \
  --graph outputs/graph_ngs.txt \
  --part outputs/part.txt \
  --rounds 20 \
  --source 0
```


## Running with More Ranks

For 8 ranks on a local machine:

```bash
mpirun --oversubscribe -n 8 ./build/ngs_mpi ...
```


## Notes

* Graphs are generated using NetGameSim with a fixed seed
* DOT output is imported and converted into a weighted graph
* The seed is saved to ensure reproducibility
* Metrics include runtime, iterations, messages, and bytes
* Tested with 2, 4, and 8 ranks

