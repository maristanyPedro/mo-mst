#ifndef GRAPH_COMPACTER_H_
#define GRAPH_COMPACTER_H_

#include <memory>
#include <set>
#include <vector>

#include "graph.h"

class GraphCompacter {
public:
    GraphCompacter(const Graph& G, std::unique_ptr<ConnectedComponents>& components, size_t redArcs, size_t blueArcs);

    CostArray getBlueComponentsCosts() const;

    ArcId getOriginalId(const Edge& edgeinCompactGraph) const;

public:
    Graph compactGraph;
    const Graph& originalGraph;
    std::unique_ptr<ConnectedComponents> connectedComponents;
    CostArray connectedComponentsCost;
    std::vector<Node> node2connectedComponent;
    const size_t redArcs;
    const size_t blueArcs;
    std::vector<bool> edgeProcessed;
    std::map<ArcId, ArcId> compactEdgeIds2OriginalEdgeIds;

private:
    void generateNodes2ComponentsLinks();

    void buildCompactGraph();
};

#endif
