# CS 453 MPI Graph Project

## Overview

This project implements distributed graph algorithms using MPI.

Each process (rank) owns a subset of nodes. Local computation is done within the rank, and communication across partitions is handled using MPI.

Algorithms:

* Leader election
* Shortest path (distributed Dijkstra-style)

---

## Clone

Clone the repository with submodules:

```bash
git clone --recurse-submodules https://github.com/hectcastUIC/CS453_Spring2026.git
cd CS453_Spring2026
```

If already cloned:

```bash
git submodule update --init --recursive
```

---

## Requirements

Run on Linux / WSL (Ubuntu recommended).

Install dependencies:

```bash
sudo apt update
sudo apt install -y build-essential cmake openmpi-bin libopenmpi-dev openjdk-17-jdk scala
```

Install `sbt` if not already installed (required for NetGameSim).

---

## Fix Scripts (WSL/Linux)

If scripts fail with `bash\r` errors, run:

```bash
find tools -type f -name "*.sh" -exec sed -i 's/\r$//' {} +
```

---

## Build

```bash
cmake -S mpi_runtime -B build
cmake --build build
```

---

## Generate Graph (NetGameSim)

```bash
./tools/graph_export/run.sh configs/netgamesim.conf outputs/graph_ngs.txt
```

This step:

* runs NetGameSim
* generates a graph with a fixed seed
* exports DOT format
* converts it to a weighted graph
* saves seed and metadata for reproducibility

Note:

* NetGameSim may print Graphviz or perturbation warnings
* The script still imports the DOT output and generates the graph file

---

## Partition Graph

```bash
./tools/partition/run.sh outputs/graph_ngs.txt --ranks 4 --out outputs/part.txt
```

---

## Run Leader Election

```bash
mpirun -n 4 ./build/ngs_mpi \
  --algo leader \
  --graph outputs/graph_ngs.txt \
  --part outputs/part.txt \
  --rounds 20
```

---

## Run Shortest Path

```bash
mpirun -n 4 ./build/ngs_mpi \
  --algo dijkstra \
  --graph outputs/graph_ngs.txt \
  --part outputs/part.txt \
  --rounds 20 \
  --source 0
```

---

## Running with More Ranks

For 8 ranks on a local machine:

```bash
mpirun --oversubscribe -n 8 ./build/ngs_mpi ...
```

---

## Notes

* Graphs are generated using NetGameSim with a fixed seed
* DOT output is imported and converted into a weighted graph
* The seed is saved for reproducibility
* Metrics include runtime, iterations, messages, and bytes
* Tested with 2, 4, and 8 ranks

---
