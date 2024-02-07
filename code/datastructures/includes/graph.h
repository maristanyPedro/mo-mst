//Implementation of an undirected graph.
#ifndef GRAPH_H_
#define GRAPH_H_

#include <cstdio>
#include <memory> //for unique_ptr
#include <set>
#include <list>
#include <vector>

#include "boost/dynamic_bitset.hpp"

#include "../../datastructures/includes/typedefs.h"

struct Arc {
    Arc() = default;
    Arc(Node n, const CostArray& c, ArcId id);

    Arc& operator=(const Arc& other) = default;

    void print() const;

    //Node tail{INVALID_NODE};
    CostArray c;
    CostType cSum{MAX_COST};
    Node n{INVALID_NODE};
    ArcId idInArcVector{0};
    bool redArc{false};
    bool blueArc{false};
};

struct ArcSorterBN {
    inline bool operator() (const Arc& lhs, const Arc& rhs) const {
        return lhs.n < rhs.n;
    }
};


struct Edge {
    Edge() = delete;
    Edge(ArcId id, Node tail, Node head, const CostArray& c);

    Edge& operator=(const Edge& other) {
        this->tail = other.tail;
        this->head = other.head;
        this->c = other.c;
        this->isRed = other.isRed;
        this->isBlue = other.isBlue;
        return *this;
    }

    const ArcId id;
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
    std::set<ArcId> edgeIds;
};

typedef std::vector<ConnectedComponent> ConnectedComponents;

struct OutgoingArcInfo {
    OutgoingArcInfo(ArcId id, bool chenStatus, bool cutExitStatus):
        edgeId{id}, incomingArcId{INVALID_ARC}, chenPruned{chenStatus}, cutExitPruned{cutExitStatus} {}

    OutgoingArcInfo(const OutgoingArcInfo& other):
        edgeId{other.edgeId}, incomingArcId{INVALID_ARC}, chenPruned{false}, cutExitPruned{other.cutExitPruned} {}

    ArcId edgeId;
    mutable ArcId incomingArcId; ///< Position of the current arc in the vector of incoming edges of the arc's head node.
    bool chenPruned;
    bool cutExitPruned;
};

class Graph {
    public:
        Graph() = default;

        Graph(Node nodesCount, ArcId arcsCount);

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

    std::vector<Edge> edges;
    Node nodesCount{0};
    ArcId arcsCount{0};

    private: //Members
        std::vector<NodeAdjacency> nodes;
};

inline const NodeAdjacency& Graph::node(const Node nodeId) const {
    return this->nodes[nodeId];
}

inline NodeAdjacency& Graph::node(const Node nodeId) {
    return this->nodes[nodeId];
}

std::unique_ptr<Graph> setupGraph(const std::string& filename);


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


void split(const std::string& s, char delim, std::vector<std::string>& elems);

std::vector<std::string> split(const std::string& s, char delim);

#endif