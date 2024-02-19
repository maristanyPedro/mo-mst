//Implementation of an undirected graph.
#ifndef GRAPH_H_
#define GRAPH_H_

#include <cstdio>
#include <memory> //for unique_ptr
#include <set>
#include <list>
#include <vector>
#include <fstream>

#include "boost/dynamic_bitset.hpp"

#include "../../datastructures/includes/typedefs.h"

struct Arc {
    Arc() = default;
    Arc(Node n, const CostArray& c, EdgeId edgeId);

    Arc& operator=(const Arc& other) = default;

    void print() const;

    CostArray c;
    Node n{INVALID_NODE};
    EdgeId idInEdgesVector{0};
    bool redArc{false};
    bool blueArc{false};
};


struct Edge {
    Edge() = delete;
    Edge(EdgeId id, Node tail, Node head, const CostArray& c);

    Edge& operator=(const Edge& other) = default;

    void print() const;

    EdgeId id;
    Node tail{INVALID_NODE};
    Node head{INVALID_NODE};

    CostArray c;
    bool isRed{false};
    bool isBlue{false};
};

struct EdgeSorter {
    explicit EdgeSorter(const DimensionsVector& dimO):
        dimensionsVector{dimO} {};

    inline bool operator() (const Edge& lhs, const Edge& rhs) const {
        return lexSmaller(lhs.c, rhs.c, this->dimensionsVector);
    }
    DimensionsVector dimensionsVector;
};

struct EdgeSorterBN {
    inline bool operator() (const Edge& lhs, const Edge& rhs) const {
        return lhs.tail < rhs.tail || (lhs.tail == rhs.tail && lhs.head < rhs.head);
    }
};

typedef std::vector<Arc> Neighborhood;

template<class Comparator>
static void sortNeighborhood(Neighborhood& arcs, const Comparator& comp) {
    std::sort(arcs.begin(), arcs.end(), comp);
}

class NodeAdjacency {
    public:
        NodeAdjacency();
        NodeAdjacency(Node nid, NeighborhoodSize degree);
    //private:
        Neighborhood adjacentArcs;
        Node id;
};

struct ConnectedComponent {
    CostArray cost{generate(0)};
    std::set<Node> component;
    std::set<EdgeId> edgeIds;
};

typedef std::vector<ConnectedComponent> ConnectedComponents;

struct OutgoingArcInfo {
    OutgoingArcInfo(EdgeId id, bool chenStatus, bool cutExitStatus):
        edgeId{id}, incomingArcId{INVALID_ARC}, chenPruned{chenStatus}, cutExitPruned{cutExitStatus} {}

    OutgoingArcInfo(const OutgoingArcInfo& other):
        edgeId{other.edgeId}, incomingArcId{INVALID_ARC}, chenPruned{false}, cutExitPruned{other.cutExitPruned} {}

    EdgeId edgeId;
    mutable EdgeId incomingArcId; ///< Position of the current arc in the vector of incoming edges of the arc's head node.
    bool chenPruned;
    bool cutExitPruned;
};

class Graph {
    public:
        Graph() = default;

        Graph(Node nodesCount, EdgeId arcsCount);

        inline const Neighborhood& adjacentArcs(const Node nodeId) const {
            return this->nodes[nodeId].adjacentArcs;
        }

        inline Neighborhood& adjacentArcs(const Node nodeId) {
            return this->nodes[nodeId].adjacentArcs;
        }

        const NodeAdjacency& node(Node nodeId) const;
        NodeAdjacency& node(Node nodeId);

        void printNodeInfo(Node nodeId) const;
        void printArcs(const Neighborhood & arcs) const;

        void setNodeInfo(Node n);

        void DFS_blue(const Node startNodes, ConnectedComponent& reachedNodes) const;

        bool reachable(Node start, Node target, const boost::dynamic_bitset<>& forbiddenNodes) const;

        NodeAdjacency& addNode(Node id);

        const Edge& edgeRepresentation(const Arc& a) const;

    std::vector<Edge> edges;
    Node nodesCount{0};
    EdgeId arcsCount{0};

    private: //Members
        std::vector<NodeAdjacency> nodes;
};

template<class Comparator>
static void sortArcs(Graph& G, const Comparator& comp) {
    for (size_t i = 0; i < G.nodesCount; ++i) {
        auto& arcs = G.node(i).adjacentArcs;
        sortNeighborhood(arcs, comp);
    }
}

inline const NodeAdjacency& Graph::node(const Node nodeId) const {
    return this->nodes[nodeId];
}

inline NodeAdjacency& Graph::node(const Node nodeId) {
    return this->nodes[nodeId];
}

void split(const std::string& s, char delim, std::vector<std::string>& elems);

std::vector<std::string> split(const std::string& s, char delim);

template<class Comparator>
std::unique_ptr<Graph> setupGraph(const std::string& filename, const Comparator& edgeComparator) {
    std::ifstream infile(filename);
    std::string line;
    size_t nodesCount =0, arcsCount = 0;
    Dimension dimension{0};
    //Parse file until information about number of nodes and number of edges is reached.
    while (getline(infile, line)) {
        std::vector<std::string> splittedLine{split(line, ' ')};
        if (splittedLine[0] == "mmst") {
            assert(splittedLine.size() >= 4);
            nodesCount = stoi(splittedLine[1]);
            arcsCount = stoi(splittedLine[2]);
            dimension = stoi(splittedLine[3]);
            if (dimension != DIM) {
                std::printf("ERROR Program compiled for %u dimensions but input file contains %u dimensional costs!\n",
                            DIM, dimension);
                exit(1);
            }
            break;
        }
    }
    if (nodesCount == 0 || arcsCount == 0) {
        printf("Could not determine the size of the graph %s. Abort.\n", filename.c_str());
        exit(1);
    }
    std::vector<NeighborhoodSize> degree(nodesCount, 0);
    std::vector<bool> foundNodes(nodesCount, false);
    std::vector<Edge> edges;
    size_t addedEdges = 0;
    while (getline(infile, line)) {
        //printf("%s\n", line.c_str());
        std::vector<std::string> splittedLine{split(line, ' ')};
        if (splittedLine[0] == "e") {
            assert(splittedLine.size() == 3 + DIM);
            Node tailId;
            std::stringstream(splittedLine[1]) >> tailId;
            Node headId;
            std::stringstream(splittedLine[2]) >> headId;
            foundNodes[tailId] = true;
            foundNodes[headId] = true;
            ++degree[tailId];
            ++degree[headId];
            assert(degree[tailId] != MAX_DEGREE);
            assert(degree[headId] != MAX_DEGREE);
            CostArray arcCosts;
            for (size_t i = 0; i < dimension; ++i) {
                arcCosts[i] = stoi(splittedLine[3+i]);
            }
            edges.emplace_back(INVALID_ARC, tailId, headId, arcCosts);
            ++addedEdges;
        }
    }
    assert(addedEdges == arcsCount);
    std::unique_ptr<Graph> G = std::make_unique<Graph>(nodesCount, arcsCount);
    std::sort(edges.begin(), edges.end(), edgeComparator);
    G->edges = std::move(edges);
    for (Node i = 0; i < nodesCount; ++i) {
        G->setNodeInfo(i);
    }
    std::vector<NeighborhoodSize> degreePerNode(nodesCount, 0);
    //printf("Added %lu edges. Want to build graph now!\n", edges.size());
    for (size_t edgeId = 0; edgeId < G->edges.size(); ++edgeId) {
        Edge& edge{G->edges[edgeId]};
        assert(edge.id == INVALID_ARC);
        edge.id = edgeId;

        NodeAdjacency& tail = G->node(edge.tail);
        NodeAdjacency& head = G->node(edge.head);
        tail.id = edge.tail;
        head.id = edge.head;

        tail.adjacentArcs.emplace_back(head.id, edge.c, edgeId);
        head.adjacentArcs.emplace_back(tail.id, edge.c, edgeId);
    }
    //printf("Graph built!\n");
    return G;
}

struct OutgoingArcSorter {
    explicit OutgoingArcSorter(const Graph& G):
            G{G} {}

    inline bool operator()(const OutgoingArcInfo& lhs, const OutgoingArcInfo& rhs) const {
        const Edge& a_lhs = G.edges[lhs.edgeId];
        const Edge& a_rhs = G.edges[rhs.edgeId];
        return lexSmaller(a_lhs.c, a_rhs.c);
    }
private:
    const Graph& G;
};

#endif