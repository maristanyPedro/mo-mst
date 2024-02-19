#include <cassert>

#include <iterator>
#include <limits>
#include <sstream>
#include <string>
#include <algorithm>

#include "../includes/graph.h"

using namespace std;

void Graph::setNodeInfo(Node n) {
    NodeAdjacency& currentNode = this->nodes[n];
    currentNode.id = n;
//    currentNode.adjacentArcs.resize(degree);
}

void Graph::printNodeInfo(const Node nodeId) const {
    printf("Analyzing node: %u\n", nodeId);
    printf("Adjacent ARCS\n");
    printArcs(this->adjacentArcs(nodeId));
}

void Graph::printArcs(const Neighborhood & arcs) const {
    for (const Arc& arc : arcs) {
        arc.print();
    }
}

void Graph::DFS_blue(const Node startNode, ConnectedComponent& reachedNodes) const {
    auto& component = reachedNodes.component;
    reachedNodes.component.insert(startNode);
    const Neighborhood& arcs{this->adjacentArcs(startNode)};
    for (const Arc& a : arcs) {
        const Edge& edge{this->edges[a.idInEdgesVector]};
        if (component.find(a.n) != component.end() || !edge.isBlue) {
            continue;
        }
        reachedNodes.edgeIds.emplace(a.idInEdgesVector);
        addInPlace(reachedNodes.cost, a.c);
        DFS_blue(a.n, reachedNodes);
    }
}

Arc::Arc(Node n, const CostArray& c, EdgeId edgeId):
        c{c}, n{n}, idInEdgesVector{edgeId} {}

void Arc::print() const {
    printf("Arc costs: (%d, %d)\n", c[0], c[1]);
}

Edge::Edge(EdgeId id, Node tail, Node head, const CostArray& c):
    id{id}, tail{tail}, head{head}, c{c} {}

Graph::Graph(Node nodesCount, EdgeId arcsCount):
        nodesCount{nodesCount},
        arcsCount{arcsCount},
        nodes(nodesCount) {
    //In case this assertion fails, just change the EdgeId typedef in typedefs.h
    assert(INVALID_ARC >= arcsCount);
}

NodeAdjacency& Graph::addNode(Node id) {
    this->nodes.emplace_back(NodeAdjacency());
    this->nodes.back().id = id;
    ++nodesCount;
    return nodes.back();
}

const Edge& Graph::edgeRepresentation(const Arc& a) const {
    return this->edges[a.idInEdgesVector];
}

bool Graph::reachable(Node start, Node target, const boost::dynamic_bitset<>& forbiddenNodes) const {
    std::list<Node> queue;
    boost::dynamic_bitset<> reached = forbiddenNodes;
    queue.push_back(start);
    assert(!reached[start]);
    reached[start] = true;
    while (!queue.empty()) {
        Node currentNode = queue.front();
        queue.pop_front();
        const Neighborhood& adjacentArcs{this->adjacentArcs(currentNode)};
        for (const Arc& a: adjacentArcs) {
            if (reached[a.n]) {
                continue;
            }
            if (a.n == target) {
                return true;
            }
            reached[a.n] = true;
            queue.push_back(a.n);
        }
    }
    return false;
}

void Edge::print() const {
    printf("\t\tEdge [%u, %u] with id: %u.\t", this->tail, this->head, this->id);
    printf("Costs: ");
    printCosts(this->c);
    printf("\n");
}


void split(const string& s, char delim, vector<string>& elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

vector<string> split(const string& s, char delim) {
    vector<string> elems;
    if (delim == ' ') {
        istringstream iss(s);
        elems = {istream_iterator<string>{iss}, istream_iterator<string>{}};
    } else {
        split(s, delim, elems);
    }

    for (auto& elem : elems) {
        if (elem.back() == '\n') {
            elem = elem.substr(0, elem.size() - 1);
        }
    }
    return elems;
}

NodeAdjacency::NodeAdjacency(): id{INVALID_NODE} {}

NodeAdjacency::NodeAdjacency(const Node nid, NeighborhoodSize degree):
        adjacentArcs(degree),
        id{nid} {
    adjacentArcs.shrink_to_fit();
}