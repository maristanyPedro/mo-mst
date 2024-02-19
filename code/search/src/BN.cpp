#include <bitset>
#include <iostream>
#include <cstring>
#include <utility>
#include <chrono>

#include "../../datastructures/includes/BinaryHeap.h"
#include "../../datastructures/includes/graph.h"
#include "../../datastructures/includes/GraphCompacter.h"

#include "../../preprocessing/includes/Dfs.h"
#include "../../preprocessing/includes/Preprocessor.h"

#include "../includes/BN.h"

using namespace BN;

inline static boost::dynamic_bitset<> addNode(const boost::dynamic_bitset<>& existing, Node newNode) {
    boost::dynamic_bitset<> n = existing;
    n[newNode] = true;
    return n;
}

MultiobjectiveSearch::MultiobjectiveSearch(const Graph &G):
    G{G},
    //permanentTrees((1<<(this->graph.nodesCount-1))),
    //queueTrees((1<<(this->graph.nodesCount-1))),
    //lastD2((1<<(this->graph.nodesCount-1)), MAX_COST),
    implicitNodes((1UL<<(this->G.nodesCount - 1))),
    dominanceBound(generate(MAX_COST)),
    targetNode{(1UL<<(this->G.nodesCount-1))-1},
    extractions{0},
    insertions{0},
    nqtIterations{0} {

    if (targetNode == 0 && G.arcsCount > 0) {
        printf("Graph is to big. Leads to overflow computing target implicit node id. Abort\n");
        exit(1);
    }

    }

MultiobjectiveSearch::TransitionNode& MultiobjectiveSearch::intializeSubset(
        boost::dynamic_bitset<> bitRepresentation,
        long unsigned decimalRepresentation) {
    //boost::dynamic_bitset<> nodeSet = addNode(predSubset.getNodes(), newNode);
    std::unique_ptr<TransitionNode> newSubset =
            std::make_unique<TransitionNode>(std::move(bitRepresentation), decimalRepresentation);
    //assert(this->implicitNodes[newSubset->getIndex()] == nullptr);
    long unsigned index = newSubset->getIndex();
    this->implicitNodes[index] = std::move(newSubset);
    return *this->implicitNodes[index];
}

MultiobjectiveSearch::TransitionNode& MultiobjectiveSearch::getSubset(const TransitionNode& predSubset, Node newNode) {
    //Since we are representing subsets of n nodes using only 2^(n-1) subsets, we first need to multiply by 2 to get
    //the correct index of the predSubset in the 2^n cardinality set. Then, we set the bit for the new node using "|1UL<<newNode".
    //The bitset for the new subset of nodes is now finished. We just need to translate it back to our index-set with
    //2^(n-1) subsets. This is the purpose of the division by two at the end.
    long unsigned idea = ((predSubset.getIndex() * 2) | 1UL << newNode)/2;
    if (this->implicitNodes[idea].get() == nullptr) {
        boost::dynamic_bitset<> bitRepresentation = addNode(predSubset.getNodes(), newNode);
        return this->intializeSubset(std::move(bitRepresentation), idea);
    }
    else {
        //printf("---------> NodeSet exists! Just take it\n");
        return *this->implicitNodes[idea];
//        return *this->test[idea];
    }
}

inline void addNode2Sequence(const QueueTree* predQueueTree, QueueTree& newQueueTree, Node newNode) {
    newQueueTree.addedNodesInOrder = predQueueTree->addedNodesInOrder;
    newQueueTree.addedNode2Index = predQueueTree->addedNode2Index;
    newQueueTree.addedNode2Index.emplace(newNode, newQueueTree.addedNodesInOrder.size());
    newQueueTree.addedNodesInOrder.push_back(newNode);
}

bool truncatedInsertionLazy(TruncatedFront& front, const CostArray& c) {
    const TruncatedCosts tc = truncate(c);
    if (front.empty()) {
        front.push_back(tc);
        return true;
    }
    auto it = front.begin();
//    while (it != front.end() && (*it)[0] <= c[1]) {
    while (it != front.end() && lexSmallerOrEquiv(*it, c)) {
//        if ((*it)[1] <= c[2]) {
        if (tc_dominates(*it, c)) {
            return false;
        }
        ++it;
    }
    it  = front.insert(it, tc);
    ++it;
    while (it != front.end()) {
//        if (c[2] <= (*it)[1]) {
        if (tc_dominates(tc, *it)) {
            it = front.erase(it);
        }
        else {
//                ++it;
            break;
        }
    }
    return true;
}

Solution MultiobjectiveSearch::run() {
    if (this->G.arcsCount == 0) {
        return Solution();
    }
    QueueTree* initialTree = this->treePool.newItem();
    initialTree->n = 0;
    initialTree->c = generate(0);
    initialTree->addedNode2Index.emplace(0,0);
    initialTree->addedNodesInOrder.push_back(0);
    std::unique_ptr<TransitionNode> initialImplicitNode = std::make_unique<TransitionNode>(this->G, 0);
    this->implicitNodes[initialImplicitNode->getIndex()] = std::move(initialImplicitNode);
    this->truncated.emplace(targetNode, TruncatedFront());
    QueueTree* efficientTree;
    std::list<QueueTree*> targetFront;

    BinaryHeap<QueueTree, CandidateLexComp> heap;
//    BinaryHeap<QueueTree, LS_sum> heap;
    heap.push(initialTree);
    auto start = std::chrono::high_resolution_clock::now();
    while (heap.size() != 0) {
        efficientTree = heap.pop();
        long unsigned currentTransitionNodeId = efficientTree->n;
        TransitionNode& currentTransitionNode{*this->implicitNodes[currentTransitionNodeId]};
        assert(currentTransitionNode.getIndex() == currentTransitionNodeId);
        TruncatedFront& currentFront{this->truncated[currentTransitionNode.getIndex()]};
        bool inserted = truncatedInsertionLazy(currentFront, efficientTree->c);
        if (!inserted) {
            this->treePool.free(efficientTree);
            continue;
        }

        if (currentTransitionNodeId == targetNode) {
            targetFront.push_back(efficientTree);
            //printf("%u %u %u\n", efficientTree->c[0], efficientTree->c[1], efficientTree->c[2]);
            continue;
        }
        extractions++;
        assert(currentTransitionNode.getIndex() == currentTransitionNodeId);

        bool success = propagate(efficientTree, currentTransitionNode, heap);
        if (success) {
            EdgeId lastEdgeId = std::numeric_limits<EdgeId>::max();
            if (efficientTree->n != 0) {
                const Arc& lastArc{this->G.node(efficientTree->lastTail).adjacentArcs[efficientTree->lastEdgeId]};
                const Edge& edgeRepresentation = this->G.edgeRepresentation(lastArc);
                lastEdgeId = edgeRepresentation.id;
            }
            permanentTrees.addElement(efficientTree->predLabelPosition, lastEdgeId);
        }
        this->treePool.free(efficientTree);
    }
    auto end = std::chrono::high_resolution_clock::now();
    //for(k=0;k<graph->nodos;k++)
    Solution solution;
    storeStatistics(solution, targetFront.size());
    std::chrono::duration<double> duration = end - start;
    solution.time = duration.count();
//    printf("The search initialized %lu out of %lu implicit nodes!\n", this->transitionNodes.size(), this->targetNode+1);
    //this->printSpanningTrees();
    //PrintTree(targetNode, solution.trees, permanentTrees);
    return solution;
}

bool MultiobjectiveSearch::buildAndAnalyze(
        QueueTree* efficientSubtree,
        const TransitionNode& transitionNodeForEfficientSubtree,
        BinaryHeap<QueueTree, BN::CandidateLexComp>& H,
        Node lastTail,
        const Arc& cutArc,
        NeighborhoodSize cutArcPosition,
        Node newTreeNode) {
    Node currentNode = efficientSubtree->n;
    CostArray costCandidate = generate(MAX_COST);
    TransitionNode& successorNode = this->getSubset(transitionNodeForEfficientSubtree, newTreeNode);
    assert(successorNode.getCardinality() == transitionNodeForEfficientSubtree.getCardinality() + 1);
    costCandidate = add(efficientSubtree->c, cutArc.c);

    if (truncatedDominance(truncated[successorNode.getIndex()], costCandidate)) {
//    if (isDominated(this->permanentTrees[successorNode.getIndex()], costCandidate)) {
        return false;
    }
    QueueTree* newOpenTree = this->treePool.newItem();
    newOpenTree->c = costCandidate;
    newOpenTree->lastEdgeId = cutArcPosition;
    newOpenTree->predSubset = currentNode;
    newOpenTree->n = successorNode.getIndex();
    newOpenTree->predLabelPosition = this->permanentTrees.getCurrentIndex();
    newOpenTree->lastTail = lastTail;
    newOpenTree->lastHead = cutArc.n;
    addNode2Sequence(efficientSubtree, *newOpenTree, newTreeNode);
    H.push(newOpenTree);
    return true;
//        printf("\t\tNew open tree: %lu, c=(%u, %u, %u)\n", newOpenTree->n, newOpenTree->c[0],  newOpenTree->c[1],  newOpenTree->c[2]);
    //newOpenTree.lastArcOriented = orientedArc;
}

bool MultiobjectiveSearch::propagate(QueueTree* efficientTree, const TransitionNode& transitionNode, BinaryHeap<QueueTree, BN::CandidateLexComp>& H) {
    NeighborhoodSize lastCutArcPosition = efficientTree->lastEdgeId;
    const Neighborhood& lastTailNeighborhood{this->G.node(efficientTree->lastTail).adjacentArcs};
    bool success = false;

    for (NeighborhoodSize aId = lastCutArcPosition + 1; aId < lastTailNeighborhood.size(); ++aId) {
        const Arc& arc = lastTailNeighborhood[aId];
        if (arc.redArc || transitionNode.getNodes()[arc.n]) {
            continue;
        }
        if (efficientTree->lastHead >= arc.n) {
            continue;
        }
        Node newTreeNode = arc.n;
        //std::pair<Node, Node> orientedArc{efficientTree.lastTail, arc.n};
        bool added = buildAndAnalyze(efficientTree, transitionNode, H, efficientTree->lastTail, arc, aId, newTreeNode);
        if (added) {
            success = true;
        }
    }
    //Skip the next propagation block if there is only one node in the tree. Reason: no arc in the tree.
    if (efficientTree->addedNodesInOrder.size() == 1) {
        return success;
    }
    size_t indexOfTail = efficientTree->addedNode2Index.at(efficientTree->lastTail);
    for (size_t i = indexOfTail + 1; i < efficientTree->addedNode2Index.size(); ++i) {
        Node tail = efficientTree->addedNodesInOrder[i];
        const Neighborhood& neighborhood{this->G.node(tail).adjacentArcs};
        for (NeighborhoodSize aId = 0; aId < neighborhood.size(); ++aId) {
            const Arc& cutArc = neighborhood[aId];
            if (cutArc.redArc || transitionNode.getNodes()[cutArc.n]) {
                continue;
            }
            Node newTreeNode{cutArc.n};
            //std::pair<Node, Node> orientedArc{tail, cutArc.n};
            bool added = buildAndAnalyze(efficientTree, transitionNode, H, tail, cutArc, aId, newTreeNode);
            if (added) {
                success = true;
            }
        }
    }
    return success;
}

void MultiobjectiveSearch::storeStatistics(Solution &sol, size_t solutionsCount) const {
    sol.trees = solutionsCount;
    sol.insertions = insertions;
    sol.extractions = extractions;
    sol.nqtIt = nqtIterations;
    sol.transitionArcsCount = 0;
    sol.transitionNodes = countTransitionNodes();
}

size_t MultiobjectiveSearch::countTransitionNodes() const {
    return std::count_if(this->implicitNodes.begin(), this->implicitNodes.end(), [](const std::unique_ptr<TransitionNode>& i){return i.get() !=
                                                                                                                                     nullptr;});
}

size_t MultiobjectiveSearch::countTransitionArcs() const {
    size_t counter{0};
//    for (auto& transitionNode : this->implicitNodes) {
//        counter += transitionNode->outgoingArcs().size();
//    }
    return counter;
}

void MultiobjectiveSearch::printParetoFront(const std::list<QueueTree*>& targetFront, const ConnectedComponents& blueArcsComponents) const {
    CostArray fixedCosts = generate(0);
    for (const auto& component : blueArcsComponents) {
        addInPlace(fixedCosts, component.cost);
    }
    for (const QueueTree* tree : targetFront) {
        for (ushort dim = 0; dim < DIM; ++dim) {
            printf("%u ", tree->c[dim] + fixedCosts [dim]);
        }
    }
}
