#include <cassert>
#include <fstream>
#include <iterator>
#include <limits>
#include <sstream>
#include <string>
#include <algorithm>

#include "../includes/graph.h"

using namespace std;

unique_ptr<Graph> setupGraph(const string& filename) {
    ifstream infile(filename);
    string line;
    size_t nodesCount =0, arcsCount = 0;
    Dimension dimension{0};
    //Parse file until information about number of nodes and number of arcs is reached.
    while (getline(infile, line)) {
        vector<string> splittedLine{split(line, ' ')};
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
    std::vector<Edge> arcs;
    size_t addedArcs = 0;
    while (getline(infile, line)) {
        //printf("%s\n", line.c_str());
        vector<string> splittedLine{split(line, ' ')};
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
            arcs.emplace_back(tailId, headId, arcCosts);
            ++addedArcs;
        }
    }
    assert(addedArcs == arcsCount);
    unique_ptr<Graph> G = make_unique<Graph>(nodesCount, arcsCount);
    std::sort(arcs.begin(), arcs.end(), EdgeSorter(standardSorting()));
    G->arcs = std::move(arcs);
    for (Node i = 0; i < nodesCount; ++i) {
        G->setNodeInfo(i);
    }
    vector<NeighborhoodSize> degreePerNode(nodesCount, 0);
    //printf("Added %lu arcs. Want to build graph now!\n", arcs.size());
    for (size_t aId = 0; aId < G->arcs.size(); ++aId) {
        Edge& doubleEndedArc{G->arcs[aId]};

        NodeAdjacency& tail = G->node(doubleEndedArc.tail);
        NodeAdjacency& head = G->node(doubleEndedArc.head);
        tail.id = doubleEndedArc.tail;
        head.id = doubleEndedArc.head;

        tail.adjacentArcs.emplace_back(head.id, doubleEndedArc.c, aId);
        head.adjacentArcs.emplace_back(tail.id, doubleEndedArc.c, aId);
    }
    //printf("Graph built!\n");
    return G;
}

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
        const Edge& edge{this->arcs[a.idInArcVector]};
        if (component.find(a.n) != component.end() || !edge.isBlue) {
            continue;
        }
        addInPlace(reachedNodes.cost, a.c);
        DFS_blue(a.n, reachedNodes);
    }
}

Arc::Arc(Node n, const CostArray& c, ArcId id):
    c{c}, n{n}, idInArcVector{id} {
    cSum = 0;
    for (size_t i = 0; i < DIM; ++i) {
        cSum += c[i];
    }
}

void Arc::print() const {
    printf("Arc costs: (%d, %d)\n", c[0], c[1]);
}

Edge::Edge(Node tail, Node head, const CostArray& c):
    tail{tail}, head{head}, c{c} {}

Graph::Graph(Node nodesCount, ArcId arcsCount):
        nodesCount{nodesCount},
        arcsCount{arcsCount},
        nodes(nodesCount) {
    //In case this assertion fails, just change the ArcId typedef in typedefs.h
    assert(INVALID_ARC >= arcsCount);
}

NodeAdjacency& Graph::addNode(Node id) {
    this->nodes.emplace_back(NodeAdjacency());
    this->nodes.back().id = id;
    ++nodesCount;
    return nodes.back();
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