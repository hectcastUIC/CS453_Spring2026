#!/usr/bin/env python3
import re
import sys
import random
from collections import defaultdict

if len(sys.argv) != 4:
    print("Usage: convert_dot_to_weighted.py <input.dot> <output_graph.txt> <seed>")
    sys.exit(1)

dot_file = sys.argv[1]
out_file = sys.argv[2]
seed = int(sys.argv[3])

random.seed(seed)

# Supports both undirected and directed DOT edge syntax:
#   1 -- 2
#   1 -> 2
edge_re = re.compile(r'^\s*"?(\d+)"?\s*(--|->)\s*"?(\d+)"?')

adj = defaultdict(set)
nodes = set()

with open(dot_file, "r", encoding="utf-8", errors="ignore") as f:
    for line in f:
        m = edge_re.search(line)
        if not m:
            continue
        a = int(m.group(1))
        b = int(m.group(3))
        if a == b:
            continue
        nodes.add(a)
        nodes.add(b)
        adj[a].add(b)
        adj[b].add(a)

# Ensure stable ordering and deterministic weights
all_nodes = sorted(nodes)

with open(out_file, "w", encoding="utf-8") as out:
    for node in all_nodes:
        neighbors = sorted(adj[node])
        parts = []
        for nb in neighbors:
            # deterministic positive weight from seed + edge endpoints
            edge_seed = seed + min(node, nb) * 10007 + max(node, nb) * 10009
            rng = random.Random(edge_seed)
            w = rng.randint(1, 20)
            parts.append(f"{nb}({w})")
        out.write(f"{node}: {' '.join(parts)}\n")
