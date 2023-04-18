#include <cassert>
#include "../includes/GraphCompacter.h"

GraphCompacter::GraphCompacter(const Graph &G, std::unique_ptr<ConnectedComponents>& components, size_t redArcs, size_t blueArcs):
    originalGraph{G},
    connectedComponents{std::move(components)},
    node2connectedComponent(G.nodesCount, INVALID_NODE),
    redArcs{redArcs},
    blueArcs{blueArcs}
{
    this->generateNodes2ComponentsLinks();
    this->buildCompactGraph();
}

void GraphCompacter::generateNodes2ComponentsLinks() {
    std::vector<bool> assignedNodes(this->originalGraph.nodesCount, false);
    for (size_t componentId = 0; componentId != this->connectedComponents->size(); ++componentId) {
        const ConnectedComponent& component = (*this->connectedComponents)[componentId];
        for (const Node n : component.component) {
            assert(!assignedNodes[n]);
            assignedNodes[n] = true;
            this->node2connectedComponent[n] = componentId;
        }
    }
    for (Node n = 0; n < this->originalGraph.nodesCount; ++n) {
        assert(assignedNodes[n]);
    }
}

void GraphCompacter::buildCompactGraph() {
    //Create one node per connected component.
    for (size_t componentId = 0; componentId < this->connectedComponents->size(); ++componentId) {
        this->compactGraph.addNode(componentId);
    }
    //Now, we iterate over all edges in the original graph. Every edge that connects two distinct
    //connected components is added to the compact graph. The new copy links the two connected components
    //that contain the original nodes connected by the original edge.
    for (const Edge& edge: this->originalGraph.arcs) {
        if (edge.isRed) {
            continue;
        }
        Node tail = edge.tail;
        Node head = edge.head;
        Node tailInNewGraph = node2connectedComponent[tail];
        Node headInNewGraph = node2connectedComponent[head];
        if (tailInNewGraph == headInNewGraph) {
            continue;
        }
        this->compactGraph.arcs.emplace_back(tailInNewGraph, headInNewGraph, edge.c);
        //TODO: add the new edge to the adjacency lists of the connectedComponents!
        this->compactGraph.node(tailInNewGraph).adjacentArcs.emplace_back(headInNewGraph, edge.c, this->compactGraph.arcs.size()-1);
        this->compactGraph.node(headInNewGraph).adjacentArcs.emplace_back(tailInNewGraph, edge.c, this->compactGraph.arcs.size()-1);
        this->compactGraph.arcsCount++;
    }
//    printf("Finish building contracted graph! n = %u m = %u\n",
//           this->compactGraph.nodesCount, this->compactGraph.arcsCount);
}

//CostArray GraphCompacter::getBlueComponentsCosts() const {
//    for (const std::set<>)
//}
