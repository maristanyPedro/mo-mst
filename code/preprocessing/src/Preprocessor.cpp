//
// Created by bzfmaris on 03.05.22.
//
#include <algorithm>
#include <chrono>

#include "../../datastructures/includes/graph.h"
#include "../../datastructures/includes/Label.h"
#include "../../datastructures/includes/BinaryHeap.h"
#include "../../datastructures/includes/GraphCompacter.h"

#include "../includes/Dfs.h"
#include "../includes/Preprocessor.h"

Preprocessor::Preprocessor()
//    lb((1<<(nodesCount-1))) {}
    {}

std::vector<ConnectedComponent> contract(const Graph& G) {
    std::vector<bool> reached(G.nodesCount, false);
    std::vector<ConnectedComponent> connectedComponents;
    for (Node u = 0; u < G.nodesCount; u++) {
        if (reached[u]) {
            continue;
        }
        ConnectedComponent connectedComponent;
        G.DFS_blue(u, connectedComponent);
        for (Node n : connectedComponent.component) {
            reached[n] = true;
        }
        connectedComponents.push_back(connectedComponent);
    }
    return connectedComponents;
}

GraphCompacter Preprocessor::run(Graph& G) {
    auto start = std::chrono::high_resolution_clock::now();
    size_t blueArcs = findBlueArcs(G);
    size_t redArcs = findRedArcs(G);

    std::unique_ptr<ConnectedComponents> connectedComponents =
            std::make_unique<ConnectedComponents>(contract(G));
    GraphCompacter gc(G, connectedComponents, redArcs, blueArcs);
    //Now, compute the lower bounds.
    this->computeLowerBounds(gc.compactGraph);
//    this->calculateHeuristic(gc.compactGraph);
    //Finally, compute a dominance bound.
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> t = end - start;
    this->duration = t.count();
    return gc;
}

//--------------------------------------------------------------------------------------------------------------------------

CostArray Preprocessor::Prim(const Graph& G, Node root, const DimensionsVector& dimOrdering) {

    std::vector<Label> labels(G.nodesCount);
    std::vector<bool> nodeSet(G.nodesCount, false);
    BinaryHeap<Label, LexComparison> heap;
    labels[root].update(generate(0), root);
    heap.push(&labels[root]);

    CostArray treeCosts;
    initialize(treeCosts, 0);
    Node finalNode = INVALID_NODE;
    while (heap.size() != 0) {
        Label* min = heap.pop();
        finalNode = min->n;
        nodeSet[finalNode] = true;
        addInPlace(treeCosts, min->c);
        const auto& adjacency{G.adjacentArcs(finalNode)};
        for (const Arc& a: adjacency) {
            Node nodeCandidate = a.n;
            if (!nodeSet[nodeCandidate]) {
                CostArray cNew = sorted(a.c, dimOrdering);
                Label& newNodeLabel{labels[nodeCandidate]};
                if (lexSmaller(cNew, newNodeLabel.c)) {
                    newNodeLabel.update(cNew, nodeCandidate);
                    if (!newNodeLabel.inQueue) {
                        heap.push(&newNodeLabel);
                    }
                    else {
                        heap.decreaseKey(&newNodeLabel);
                    }
                }
            }
        }
    }
//    assert(labels[finalNode].c1 == treeCosts[0] && labels[finalNode].c2 == treeCosts[1]);
    return treeCosts;
}

void Preprocessor::calculateHeuristic(const Graph& G) {
    size_t subsets = (1<<(G.nodesCount-1));
    //std::vector<CostArray> lb(subsets);
    for (size_t i = 0; i < subsets; ++ i) {
        size_t fullGraphRepresentation = i*2;
        boost::dynamic_bitset<> containedNodes(G.nodesCount, fullGraphRepresentation);
        containedNodes[0] = true;
        DimensionsVector dimOrdering = standardSorting();
        //printf("Processing %lu\n", i);
        //fill dimOrderings vector from 0 to dimOrdering.size()-1.
        Info<bool> processedMainDimensions;
        std::fill(processedMainDimensions.begin(), processedMainDimensions.end(), false);
        do {
            //Avoid multiple lex. searches with the same first optimization criterion.
            if (!processedMainDimensions[dimOrdering[0]]) {
                processedMainDimensions[dimOrdering[0]] = true;
                CostArray lexResult = Prim(G, containedNodes, dimOrdering);
                lb[i][dimOrdering[0]] = lexResult[0];
                //printf("\tDimOrdering = (%u, %u, %u)\n", dimOrdering[0], dimOrdering[1], dimOrdering[2]);
                //printf("\t\tLexSolution = (%u, %u, %u)\n", lexResult[0], lexResult[1], lexResult[2]);
            }
        } while (std::next_permutation(dimOrdering.begin(), dimOrdering.end()));
    }
}

CostArray Preprocessor::Prim(const Graph& G, boost::dynamic_bitset<> containedNodes, const DimensionsVector& dimOrdering) {

    std::vector<Label> labels(G.nodesCount);
    std::vector<bool> nodeSet(G.nodesCount, false);
    BinaryHeap<Label, LexComparison> heap;
    for (Node n = 0; n < G.nodesCount; ++ n) {
        if (containedNodes[n]) {
            //printf("\t\t\tAdding node %u to super source!\n", n);
            labels[n].update(generate(0), n);
            heap.push(&labels[n]);
        }
    }

    CostArray treeCosts;
    initialize(treeCosts, 0);
    Node finalNode = INVALID_NODE;
    while (heap.size() != 0) {
        Label* min = heap.pop();
        finalNode = min->n;
        nodeSet[finalNode] = true;
        addInPlace(treeCosts, min->c);
        const auto& adjacency{G.adjacentArcs(finalNode)};
        for (const Arc& a: adjacency) {
            Node nodeCandidate = a.n;
            if (!nodeSet[nodeCandidate]) {
                CostArray cNew = sorted(a.c, dimOrdering);
                Label& newNodeLabel{labels[nodeCandidate]};
                if (lexSmaller(cNew, newNodeLabel.c)) {
                    newNodeLabel.update(cNew, nodeCandidate);
                    if (!newNodeLabel.inQueue) {
                        heap.push(&newNodeLabel);
                    }
                    else {
                        heap.decreaseKey(&newNodeLabel);
                    }
                }
            }
        }
    }
//    assert(labels[finalNode].c1 == treeCosts[0] && labels[finalNode].c2 == treeCosts[1]);
    return treeCosts;
}

void Preprocessor::computeLowerBounds(const Graph& G) {
    CostArray INF_array;
    initialize(INF_array, MAX_COST);
    this->lb = std::vector<CostArray>(G.nodesCount, INF_array);
    DimensionsVector dimOrdering = standardSorting();
    //fill dimOrderings vector from 0 to dimOrdering.size()-1.
    Info<bool> processedMainDimensions;
    std::fill(processedMainDimensions.begin(), processedMainDimensions.end(), false);
    std::vector<Edge> arcsCopy = G.edges;
    do {
        //Avoid multiple lex. searches with the same first optimization criterion.
        if (!processedMainDimensions[dimOrdering[0]]) {
            std::sort(arcsCopy.begin(), arcsCopy.end(), EdgeSorter(dimOrdering));
            lb[0][dimOrdering[0]] = 0;
            for (Node n = 0; n < G.nodesCount -1; ++n) {
                lb[n+1][dimOrdering[0]] = lb[n][dimOrdering[0]] + arcsCopy[n].c[dimOrdering[0]];
            }
            processedMainDimensions[dimOrdering[0]] = true;
            dominanceBound = max(dominanceBound, Prim(G, 0, dimOrdering), dimOrdering);
        }
    } while (std::next_permutation(dimOrdering.begin(), dimOrdering.end()));
    std::reverse(lb.begin(), lb.end());
}

