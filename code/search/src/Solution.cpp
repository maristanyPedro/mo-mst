#include "../includes/Solution.h"

#include "../../datastructures/includes/GraphCompacter.h"

void Solution::printSpanningTrees(const GraphCompacter& compactGraph) {

    size_t treeCount{0};
    for (size_t treeIndex: this->spanningTreeIndices) {
        printf("Solution tree number: %lu\n", treeCount++);
        CostArray treeCosts{generate(0)};
        PermanentTree const *tree = &this->permanents->getElement(treeIndex);
        Edge const *preimageOfPredArc{&compactGraph.compactGraph.edges[tree->lastArc]};
        addInPlace(treeCosts, preimageOfPredArc->c);
        size_t printedEdges{0};

        const Edge &edgeInOriginalGraph = compactGraph.originalGraph.edges[compactGraph.getOriginalId(
                *preimageOfPredArc)];
        edgeInOriginalGraph.print();
        ++printedEdges;
        while (printedEdges < compactGraph.compactGraph.nodesCount - 1) {
            tree = &this->permanents->getElement(tree->predLabelPosition);
            preimageOfPredArc = &compactGraph.compactGraph.edges[tree->lastArc];
            addInPlace(treeCosts, preimageOfPredArc->c);
            const Edge &edgeInOriginalGraph = compactGraph.originalGraph.edges[compactGraph.getOriginalId(
                    *preimageOfPredArc)];
            edgeInOriginalGraph.print();
//printf("\t\t\tEdge: [%u, %u] with c = (%u, %u, %u)\n", preimageOfPredArc->tail, preimageOfPredArc->head, preimageOfPredArc->c[0],  preimageOfPredArc->c[1],  preimageOfPredArc->c[2]);
            ++printedEdges;
        }
        for (const auto &connectedComponent: *compactGraph.connectedComponents) {
            for (EdgeId edgeId: connectedComponent.edgeIds) {
                const Edge &edgeInOriginalGraph{compactGraph.originalGraph.edges[edgeId]};
                edgeInOriginalGraph.print();
                ++printedEdges;
            }
        }
        addInPlace(treeCosts, compactGraph.connectedComponentsCost);
        printf("\t\tTree with costs: ");
        printCosts(treeCosts);
        printf("\n");
        assert(printedEdges == compactGraph.originalGraph.nodesCount - 1);
    }
}