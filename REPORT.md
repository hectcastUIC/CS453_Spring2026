# REPORT

## Overview

This project implements distributed graph algorithms using MPI on a partitioned graph.

Each MPI rank owns a subset of graph nodes. Local edges are handled inside the same rank, and cross-partition edges require MPI communication.

Implemented algorithms:
- Leader election
- Shortest path (Dijkstra-style iterative relaxation)

## Graph Generation

The project includes a graph generation script:

```bash
./tools/graph_export/run.sh configs/small.conf outputs/graph8.txt
