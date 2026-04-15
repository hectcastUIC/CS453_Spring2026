# CS 453 MPI Graph Project

## Overview

This project runs distributed graph algorithms using MPI.

Each process (rank) owns part of the graph and communicates with others when needed.

Algorithms:

* Leader election
* Shortest path (Dijkstra-style)

---

## Build

```bash
cmake -S mpi_runtime -B build
cmake --build build
```

---

## Generate graph (NetGameSim)

```bash
./tools/graph_export/run.sh configs/netgamesim.conf outputs/graph_ngs.txt
```

This:

* runs NetGameSim
* generates a graph with a fixed seed
* converts it to weighted format
* saves metadata for reproducibility

---

## Partition graph

```bash
./tools/partition/run.sh outputs/graph_ngs.txt --ranks 4 --out outputs/part.txt
```

---

## Run (leader election)

```bash
mpirun -n 4 ./build/ngs_mpi \
  --algo leader \
  --graph outputs/graph_ngs.txt \
  --part outputs/part.txt \
  --rounds 20
```

---

## Run (shortest path)

```bash
mpirun -n 4 ./build/ngs_mpi \
  --algo dijkstra \
  --graph outputs/graph_ngs.txt \
  --part outputs/part.txt \
  --rounds 20 \
  --source 0
```

---

## Notes

* Graphs are generated using NetGameSim with a fixed seed
* DOT output is imported and converted to weighted graph format
* Results include runtime, iterations, and message statistics
* Works with multiple ranks (tested with 2, 4, 8)

---
