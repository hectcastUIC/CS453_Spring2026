#include <mpi.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using WeightedGraph = std::map<int, std::vector<std::pair<int, int>>>;
using PartitionMap = std::map<int, std::vector<int>>;
using OwnerMap = std::map<int, int>;
using IntMap = std::map<int, int>;

struct Metrics {
    int iterations = 0;
    long long total_msgs = 0;
    long long total_bytes = 0;
    double runtime_ms = 0.0;
};

struct AlgoResult {
    IntMap values;
    Metrics metrics;
};

WeightedGraph load_graph(const std::string& filename) {
    std::ifstream file(filename);
    WeightedGraph graph;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        int node = -1;
        char colon = '\0';
        ss >> node >> colon;

        std::string token;
        while (ss >> token) {
            int neighbor = -1;
            int weight = -1;
            if (std::sscanf(token.c_str(), "%d(%d)", &neighbor, &weight) == 2) {
                graph[node].push_back({neighbor, weight});
            }
        }
    }

    return graph;
}

PartitionMap load_partition(const std::string& filename) {
    std::ifstream file(filename);
    PartitionMap partition;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        int rank = -1;
        char colon = '\0';
        ss >> rank >> colon;

        int node = -1;
        while (ss >> node) {
            partition[rank].push_back(node);
        }
    }

    return partition;
}

OwnerMap build_owner_map(const PartitionMap& partition) {
    OwnerMap owner;
    for (const auto& entry : partition) {
        int rank = entry.first;
        const std::vector<int>& nodes = entry.second;
        for (int node : nodes) {
            owner[node] = rank;
        }
    }
    return owner;
}

AlgoResult run_leader(
    int rank,
    int world_size,
    int rounds,
    const WeightedGraph& graph,
    const std::vector<int>& local_nodes,
    const OwnerMap& owner_of
) {
    IntMap candidate;
    for (int node : local_nodes) {
        candidate[node] = node;
    }

    Metrics metrics;
    double start = MPI_Wtime();

    for (int round = 0; round < rounds; ++round) {
        metrics.iterations++;

        IntMap next = candidate;
        std::map<int, std::vector<int>> outgoing;

        for (int node : local_nodes) {
            int current = candidate[node];

            auto it = graph.find(node);
            if (it == graph.end()) {
                continue;
            }

            for (const auto& edge : it->second) {
                int neighbor = edge.first;
                int owner = owner_of.at(neighbor);

                if (owner == rank) {
                    next[neighbor] = std::max(next[neighbor], current);
                } else {
                    outgoing[owner].push_back(neighbor);
                    outgoing[owner].push_back(current);
                }
            }
        }

        for (int other = 0; other < world_size; ++other) {
            if (other == rank) {
                continue;
            }

            int send_count = 0;
            if (outgoing.count(other)) {
                send_count = static_cast<int>(outgoing[other].size());
            }

            int recv_count = 0;
            MPI_Sendrecv(
                &send_count, 1, MPI_INT, other, 0,
                &recv_count, 1, MPI_INT, other, 0,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE
            );

            metrics.total_msgs += 2;
            metrics.total_bytes += 2 * static_cast<long long>(sizeof(int));

            std::vector<int> recv(recv_count, -1);

            const int* send_ptr = nullptr;
            if (send_count > 0) {
                send_ptr = outgoing[other].data();
            }

            MPI_Sendrecv(
                send_ptr, send_count, MPI_INT, other, 1,
                recv.data(), recv_count, MPI_INT, other, 1,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE
            );

            metrics.total_msgs += 2;
            metrics.total_bytes += static_cast<long long>(send_count + recv_count) * sizeof(int);

            for (int i = 0; i + 1 < recv_count; i += 2) {
                int node = recv[i];
                int val = recv[i + 1];
                if (next.count(node)) {
                    next[node] = std::max(next[node], val);
                }
            }
        }

        candidate = next;
    }

    double end = MPI_Wtime();
    metrics.runtime_ms = (end - start) * 1000.0;

    return {candidate, metrics};
}

AlgoResult run_dijkstra(
    int rank,
    int world_size,
    int rounds,
    int source,
    const WeightedGraph& graph,
    const std::vector<int>& local_nodes,
    const OwnerMap& owner_of
) {
    const int INF = std::numeric_limits<int>::max() / 4;

    IntMap dist;
    for (int node : local_nodes) {
        dist[node] = (node == source) ? 0 : INF;
    }

    Metrics metrics;
    double start = MPI_Wtime();

    for (int round = 0; round < rounds; ++round) {
        metrics.iterations++;

        IntMap next = dist;
        std::map<int, std::vector<int>> outgoing;

        for (int node : local_nodes) {
            int d = dist[node];
            if (d >= INF) {
                continue;
            }

            auto it = graph.find(node);
            if (it == graph.end()) {
                continue;
            }

            for (const auto& edge : it->second) {
                int neighbor = edge.first;
                int weight = edge.second;
                int newd = d + weight;
                int owner = owner_of.at(neighbor);

                if (owner == rank) {
                    next[neighbor] = std::min(next[neighbor], newd);
                } else {
                    outgoing[owner].push_back(neighbor);
                    outgoing[owner].push_back(newd);
                }
            }
        }

        for (int other = 0; other < world_size; ++other) {
            if (other == rank) {
                continue;
            }

            int send_count = 0;
            if (outgoing.count(other)) {
                send_count = static_cast<int>(outgoing[other].size());
            }

            int recv_count = 0;
            MPI_Sendrecv(
                &send_count, 1, MPI_INT, other, 0,
                &recv_count, 1, MPI_INT, other, 0,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE
            );

            metrics.total_msgs += 2;
            metrics.total_bytes += 2 * static_cast<long long>(sizeof(int));

            std::vector<int> recv(recv_count, -1);

            const int* send_ptr = nullptr;
            if (send_count > 0) {
                send_ptr = outgoing[other].data();
            }

            MPI_Sendrecv(
                send_ptr, send_count, MPI_INT, other, 1,
                recv.data(), recv_count, MPI_INT, other, 1,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE
            );

            metrics.total_msgs += 2;
            metrics.total_bytes += static_cast<long long>(send_count + recv_count) * sizeof(int);

            for (int i = 0; i + 1 < recv_count; i += 2) {
                int node = recv[i];
                int val = recv[i + 1];
                if (next.count(node)) {
                    next[node] = std::min(next[node], val);
                }
            }
        }

        dist = next;
    }

    double end = MPI_Wtime();
    metrics.runtime_ms = (end - start) * 1000.0;

    return {dist, metrics};
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank = -1;
    int world_size = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    std::string algo = "leader";
    std::string graph_file;
    std::string part_file;
    int rounds = 4;
    int source = 0;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--algo") == 0 && i + 1 < argc) {
            algo = argv[++i];
        } else if (std::strcmp(argv[i], "--graph") == 0 && i + 1 < argc) {
            graph_file = argv[++i];
        } else if (std::strcmp(argv[i], "--part") == 0 && i + 1 < argc) {
            part_file = argv[++i];
        } else if (std::strcmp(argv[i], "--rounds") == 0 && i + 1 < argc) {
            rounds = std::stoi(argv[++i]);
        } else if (std::strcmp(argv[i], "--source") == 0 && i + 1 < argc) {
            source = std::stoi(argv[++i]);
        }
    }

    if (graph_file.empty() || part_file.empty()) {
        if (rank == 0) {
            std::cerr << "Usage: ./ngs_mpi --algo <leader|dijkstra> --graph <file> --part <file> [--rounds N] [--source N]" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    WeightedGraph graph = load_graph(graph_file);
    PartitionMap part = load_partition(part_file);
    OwnerMap owner = build_owner_map(part);

    if (!part.count(rank)) {
        std::cerr << "Rank " << rank << " missing in partition file" << std::endl;
        MPI_Finalize();
        return 1;
    }

    std::vector<int> local_nodes = part[rank];

    if (algo == "leader") {
        AlgoResult result = run_leader(rank, world_size, rounds, graph, local_nodes, owner);

        for (int node : local_nodes) {
            std::cout << "Rank " << rank
                      << " node " << node
                      << " leader = " << result.values[node]
                      << std::endl;
        }

        if (rank == 0) {
            std::cout << "Leader metrics: iterations=" << result.metrics.iterations
                      << " runtime_ms=" << result.metrics.runtime_ms
                      << " total_msgs=" << result.metrics.total_msgs
                      << " total_bytes=" << result.metrics.total_bytes
                      << std::endl;
        }
    } else if (algo == "dijkstra") {
        AlgoResult result = run_dijkstra(rank, world_size, rounds, source, graph, local_nodes, owner);

        for (int node : local_nodes) {
            std::cout << "Rank " << rank
                      << " node " << node
                      << " distance = " << result.values[node]
                      << std::endl;
        }

        if (rank == 0) {
            std::cout << "Dijkstra metrics: iterations=" << result.metrics.iterations
                      << " runtime_ms=" << result.metrics.runtime_ms
                      << " total_msgs=" << result.metrics.total_msgs
                      << " total_bytes=" << result.metrics.total_bytes
                      << std::endl;
        }
    } else {
        if (rank == 0) {
            std::cerr << "Unknown algorithm: " << algo << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    MPI_Finalize();
    return 0;
}