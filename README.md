# NetGameSim to MPI Distributed Algorithms Project

CS 453: Introduction to Parallel and Distributed Processing  
Spring 2026

## Overview

This project builds a distributed MPI application that operates on synthetic network graphs generated with NetGameSim. The system supports:

- connected weighted graph generation
- graph partitioning across MPI ranks
- distributed leader election
- distributed shortest paths using Dijkstra
- metrics collection and reproducible experiments

The goal is to simulate a distributed graph-processing workflow where each MPI rank owns a subset of graph nodes rather than mapping one node directly to one process.

## Project Goals

This repository implements the following pipeline:

1. Generate a connected graph using NetGameSim
2. Assign positive edge weights
3. Partition graph nodes across MPI ranks
4. Run distributed leader election
5. Run distributed Dijkstra shortest path
6. Record logs, runtime, message counts, and experiment summaries

## Repository Structure

```text
.
├── netgamesim/              # Upstream or forked NetGameSim code
├── tools/
│   ├── graph_export/        # Graph generation / export utilities
│   └── partition/           # Graph partitioning utilities
├── mpi_runtime/
│   ├── include/             # Shared headers
│   ├── src/                 # MPI runtime and algorithm implementations
│   └── CMakeLists.txt       # Build configuration
├── configs/                 # Example configurations
├── experiments/             # Scripts for reproducible runs
├── outputs/                 # Generated graphs, partitions, logs
├── REPORT.md                # Experiment report
└── README.md