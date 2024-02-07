#include <bitset>
#include <iostream>
#include <cstring>
#include <utility>
#include <chrono>

#include "../../datastructures/includes/BinaryHeap.h"
#include "../../datastructures/includes/graph.h"
#include "../../datastructures/includes/GraphCompacter.h"
#include "../../datastructures/includes/MemoryPool.h"

#include "../../preprocessing/includes/Preprocessor.h"

#include "../includes/MultiPrim.h"

using namespace NEW_GENERATION;

inline static boost::dynamic_bitset<> addNode(const boost::dynamic_bitset<>& existing, Node newNode) {
    boost::dynamic_bitset<> n = existing;
    n[newNode] = true;
    return n;
}

IGMDA::IGMDA(const Graph &G):
        graph{G},
        implicitNodes((1UL<<(this->graph.nodesCount - 1))),
        dominanceBound(generate(MAX_COST)),
        targetNode{(1UL<<(this->graph.nodesCount - 1)) - 1},
        extractions{0},
        insertions{0},
        nqtIterations{0} {
        if (targetNode == 0 && graph.arcsCount > 0) {
            printf("Graph is to big. Leads to overflow computing target implicit node id. Abort\n");
            exit(1);
        }
        }

//Since we are representing subsets of n nodes using only 2^(n-1) subsets, we first need to multiply by 2 to get
//the correct index of the predSubset in the 2^n cardinality set. Then, we set the bit for the new node using "|1UL<<newNode".
//The bitset for the new subset of nodes is now finished. We just need to translate it back to our index-set with
//2^(n-1) subsets. This is the purpose of the division by two at the end.
static inline long unsigned computeNewTransitionNodeIndex(long unsigned predTransitionNodeIndex, Node newNode) {
    return ((predTransitionNodeIndex * 2) | 1UL << newNode) / 2;
}

IGMDA::TransitionNode& IGMDA::initTransitionNode(
        boost::dynamic_bitset<> bitRepresentation,
        long unsigned decimalRepresentation,
        const TransitionNode& predSubset,
        Node newNode) {
    //boost::dynamic_bitset<> nodeSet = addNode(predSubset.getNodes(), newNode);
    std::unique_ptr<TransitionNode> newTransitionNode =
            std::make_unique<TransitionNode>(this->graph, std::move(bitRepresentation), decimalRepresentation, predSubset.outgoingArcs(), newNode);
    //assert(this->implicitNodes[newTransitionNode->getIndex()] == nullptr);
    long unsigned index = newTransitionNode->getIndex();
    this->implicitNodes[index] = std::move(newTransitionNode);
    //auto insertionPair = this->transitionNodes.emplace(index, std::move(newTransitionNode));
    //assert(insertionPair.second);
//    return *insertionPair.first->second;
    return *this->implicitNodes[index];
}

IGMDA::TransitionNode& IGMDA::getTransitionNode(const TransitionNode& predSubset, Node newNode, long unsigned decimalRepresentation) {
//    if (this->transitionNodes.find(decimalRepresentation) == this->transitionNodes.end()) {
//        boost::dynamic_bitset<> bitRepresentation = addNode(predSubset.getNodes(), newNode);
//        ImplicitNode& result = this->initTransitionNode(std::move(bitRepresentation), decimalRepresentation, predSubset, newNode);
//        this->transitionArcs += result.outgoingArcs().size();
//        return result;
//    }
    if (this->implicitNodes[decimalRepresentation].get() == nullptr) {
        boost::dynamic_bitset<> bitRepresentation = addNode(predSubset.getNodes(), newNode);
        TransitionNode& result = this->initTransitionNode(std::move(bitRepresentation), decimalRepresentation, predSubset, newNode);
        this->transitionArcs += result.outgoingArcs().size();
        return result;
    }
    else {
        return *this->implicitNodes[decimalRepresentation];
    }
}

Solution IGMDA::run(const GraphCompacter& compactGraph) {
    if (this->graph.arcsCount == 0) {
        return Solution();
    }
    Pool<SubTree> treesPool;
    SubTree* initialTree = treesPool.newItem();
    initialTree->n = 0; initialTree->c = generate(0);
    std::unique_ptr<TransitionNode> initialImplicitNode = std::make_unique<TransitionNode>(this->graph, 0);
    initialImplicitNode->setQueueTree(initialTree);
    this->implicitNodes[initialImplicitNode->getIndex()] = std::move(initialImplicitNode);
    std::list<SubTree*> solutions;
    BinaryHeap<SubTree, CandidateLexComp> heap;
    heap.push(initialTree);
    this->truncated.emplace(targetNode, TruncatedFront());
    auto start = std::chrono::high_resolution_clock::now();
    while (heap.size() != 0) {
        SubTree* minTree = heap.pop();
        extractions++;
        long unsigned currentNode = minTree->n;
        const TransitionNode& searchNode{*this->implicitNodes[currentNode]};
        assert(searchNode.getQueueTree() == minTree);
//        printf("Extract minTree with costs %u %u %u. Last arc is %u, predPosition is %lu and node is: \n",
//               minTree->c[0], minTree->c[1], minTree->c[2], minTree->lastTransitionArc, minTree->predLabelPosition);
//        searchNode.print();

        assert(searchNode.getIndex() == currentNode);

        truncatedInsertion(truncated[currentNode], minTree->c);
        nextQueueTree(minTree, heap, treesPool);
        if (currentNode == targetNode) {
//            printf("SOL: %u %u %u\n", minTree->c[0], minTree->c[1], minTree->c[2]);
            solutions.push_back(minTree);
            ++solutionsCount;
            continue;
        }

        bool success = propagate(minTree, searchNode, heap, treesPool);
        if (success) {
            permanentTrees.addElement(minTree->predLabelPosition, minTree->lastTransitionArc);
        }
        treesPool.free(minTree);
    }
    auto end = std::chrono::high_resolution_clock::now();
    Solution solution;
    std::chrono::duration<double> duration = end - start;
    solution.time = duration.count();
    storeStatistics(solution, solutions);
    //this->printSpanningTrees(solutions, this->permanentTrees, this->graph, compactGraph);
//    for (const SubTree* tree : solutions) {
//        printf("SOL;%u;%u;%u\n", tree->c[0], tree->c[1], tree->c[2]);
//    }
    return solution;
}

void IGMDA::nextQueueTree(const SubTree* minTree, BinaryHeap<SubTree, CandidateLexComp>& heap, Pool<SubTree>& treesPool) {
    long unsigned n{minTree->n};
    TransitionNode& searchNode{*this->implicitNodes[n]};
    SubTree* newQueueTree = nullptr;
    std::vector<PredArc>& predArcs = searchNode.getIncomingArcs();
    //const Front& currentFront{this->permanentTrees[searchNode.getIndex()]};
    const TruncatedFront& currentTruncatedFront{this->truncated[searchNode.getIndex()]};
    //const TruncatedFront& targetFront{this->truncated[this->targetNode]};
    List* minCandidates{nullptr};
    bool success = false;
    for (const PredArc& predInfo : predArcs) {
        List &predSubTrees = predInfo.nextQueueTrees;
        if (predSubTrees.empty()) {
            continue;
        }
        SubTree *candidateTree = predSubTrees.first;
        while (candidateTree != nullptr) {
            if (newQueueTree != nullptr && !lexSmaller(candidateTree->c, newQueueTree->c)) {
                if (dominates(newQueueTree->c, candidateTree->c) || dominates(minTree->c, candidateTree->c)) {
                    predSubTrees.pop_front();
                    treesPool.free(candidateTree);
                }
                break;
            }
            if (!dominates(minTree->c, candidateTree->c)) {
//                    if (this->solutionsCount == minTree->knownTargetElements || !truncatedDominance(targetFront, minTree->c)) {
                if (candidateTree->nclChecked || !truncatedDominance(currentTruncatedFront, candidateTree->c)) {
                    candidateTree->nclChecked = true;
                    success = true;
                    newQueueTree = candidateTree;
                    minCandidates = &predSubTrees;
                    break;
                }
//                    }
            }
            predSubTrees.pop_front();
            treesPool.free(candidateTree);
            candidateTree = predSubTrees.first;
        }
    }
    if (success) {
        heap.push(newQueueTree);
        searchNode.setQueueTree(newQueueTree);
        ++insertions;
        minCandidates->pop_front();
    }
    else{
        //treesPool.free(searchNode.getQueueTree());
        searchNode.setQueueTree(nullptr);
    }
}

static SubTree* getQueueTree(ImplicitNode<SubTree>& n, Pool<SubTree>& treesPool) {
    SubTree* existingQueueTree = n.getQueueTree();
    if (existingQueueTree) {
        return existingQueueTree;
    }
    else {
        SubTree* newTree = treesPool.newItem();
        newTree->initialize(n.getIndex());
        n.setQueueTree(newTree);
        return newTree;
    }
}

bool IGMDA::propagate(const SubTree* predLabel, const TransitionNode& searchNode,
                      BinaryHeap<SubTree, CandidateLexComp>& H, Pool<SubTree>& treesPool) {
    //printf("Propagate from node\n");
    //searchNode.print();
    const auto& outgoingArcs = searchNode.outgoingArcs();
    CostArray costCandidate;
    bool success = false;
    //If the propagation yields some non-dominated subtrees, predLabel will be made permanent. predIndex indicates
    //the position in which it will be stored.
    const size_t predIndex = this->permanentTrees.getCurrentIndex();
    //CostArray treeCosts = substract(predLabel->c, lb[searchNode.getCardinality()]);

    for (const OutgoingArcInfo& outgoingArcInfo : outgoingArcs) {
        //Pruning by Chen when constructing set of outgoing edges for searchNode determined that this arc is not active.
        if (outgoingArcInfo.chenPruned || outgoingArcInfo.cutExitPruned) {
            continue;
        }
        ArcId aId = outgoingArcInfo.edgeId;
        const Edge& edge{this->graph.edges[aId]};
        //printf("\t\tAnalyzing outgoing edge %u --> %u c = (%u,%u,%u) id: %u\n", edge.tail, edge.head, edge.c[0], edge.c[1], edge.c[2], aId);
        assert(!searchNode.getNodes()[edge.tail] || !searchNode.getNodes()[edge.head]);
        Node newTreeNode = searchNode.getNodes()[edge.tail] ? edge.head : edge.tail;
        long unsigned successorNodeIndex = computeNewTransitionNodeIndex(searchNode.getIndex(), newTreeNode);
        costCandidate = add(predLabel->c, edge.c);

        TransitionNode& successorNode = this->getTransitionNode(searchNode, newTreeNode, successorNodeIndex);
//        printf("\t\t\tPropagating tree with costs %u %u %u to node %lu\n",
//               costCandidate[0], costCandidate[1], costCandidate[2], successorNode.getIndex());
        if (outgoingArcInfo.incomingArcId == INVALID_ARC) {
            outgoingArcInfo.incomingArcId = successorNode.nextIncomingArcIndex();
            successorNode.addIncomingArc();
        }
        //successorNode.print();
        assert(successorNode.getCardinality() == searchNode.getCardinality() + 1);

        //constructing them.
        //CostArray reducedCosts = add(costCandidate, lb[successorNode.getCardinality()-1]);
        SubTree* queueTree = getQueueTree(successorNode, treesPool);
        if (queueTree->inQueue) {
            SubTree* newLabel = treesPool.newItem();
            newLabel->update(successorNode.getIndex(), costCandidate, outgoingArcInfo.incomingArcId, predIndex);
            if (lexSmaller(costCandidate, queueTree->c)) {
                if (truncatedDominance(this->truncated[successorNode.getIndex()], costCandidate)) {
                    continue;
                }
//                if (this->pruned(reducedCosts)) {
//                    continue;
//                }
//                printf("\t\t\t\tThe queue tree is in queue and is %u %u %u, replacement!\n",
//                       queueTree->c[0], queueTree->c[1], queueTree->c[2]);
                success = true;
                const PredArc& oldQueueTreePred = successorNode.getIncomingArc(queueTree->lastTransitionArc);
                //printf("\n\nSubstitute (%u, %u) with (%u, %u) for index %lu\n", queueTree.c[0], queueTree.c[1], cr1, cr2, queueTree.n);
                H.decreaseKey(queueTree, newLabel);
                successorNode.setQueueTree(newLabel);
                oldQueueTreePred.nextQueueTrees.push_front(queueTree);
            } else {
                if (dominates(queueTree->c, costCandidate) ) { //|| pruned(reducedCosts)) {
                    continue;
                }
//                printf("\t\t\t\tThe queue tree is in queue and is %u %u %u, stays!\n",
//                       queueTree->c[0], queueTree->c[1], queueTree->c[2]);
                success = true;
                const PredArc& predArc = successorNode.getIncomingArc(newLabel->lastTransitionArc);
                //THE NEW CANDIDATE NEEDS TO BE APPENDED TO THE LIST CORRESPONDING TO ITS LAST ARC!
                predArc.nextQueueTrees.push_back(newLabel);
            }
        }
        else {
            if (truncatedDominance(this->truncated[successorNode.getIndex()], costCandidate)) {
                continue;
            }
//            if (this->pruned(reducedCosts)) {
//                continue;
//            }
            success = true;
            queueTree->update(successorNode.getIndex(), costCandidate, outgoingArcInfo.incomingArcId, predIndex);
//            printf("\t\t\t\tNo queue tree and is %u %u %u, I'm queue!\n",
//                   queueTree->c[0], queueTree->c[1], queueTree->c[2]);
            assert(queueTree->n == successorNode.getIndex());
            //printf("\n\nPushing (%u, %u) for index %lu\n", cr1, cr2, queueTree.n);
            H.push(queueTree);
        }
        //assert(successorNode.getIncomingArcs().size() == successorNode.nextIncomingArcIndex());
    }
    return success;
}

void IGMDA::storeStatistics(Solution &sol, std::list<SubTree*>& solutions) const {
    sol.trees = solutions.size();
    sol.insertions = insertions;
    sol.extractions = extractions;
    sol.nqtIt = nqtIterations;
    sol.transitionArcsCount = this->transitionArcs;
    sol.transitionNodes = countTransitionNodes();
    sol.transitionArcs = countTransitionArcs();
}

void IGMDA::printSpanningTrees(const std::list<SubTree*>& solutions, const Permanents& permanents, const Graph& G, const GraphCompacter& compactGraph) {
    for (const SubTree* tree : solutions) {
        size_t printedEdges{0};
        CostArray treeCosts{generate(0)};
        addInPlace(treeCosts, tree->c);
        addInPlace(treeCosts, compactGraph.connectedComponentsCost);
        printf("New tree with costs %u %u %u\n", treeCosts[0], treeCosts[1], treeCosts[2]);
        PermanentTree const * pred = &permanents.getElement(tree->predLabelPosition);
        Edge const * preimageOfPredArc{&G.edges[tree->lastTransitionArc]};
        const Edge& edgeInOriginalGraph = compactGraph.originalGraph.edges[compactGraph.getOriginalId(*preimageOfPredArc)];
        printf("Edge: [%u, %u] with c = (%u, %u, %u)\n", edgeInOriginalGraph.tail, edgeInOriginalGraph.head, edgeInOriginalGraph.c[0],  edgeInOriginalGraph.c[1],  edgeInOriginalGraph.c[2]);
        ++printedEdges;
        CostArray currentCosts = substract(tree->c, preimageOfPredArc->c);
        while (printedEdges < G.nodesCount-1) {
//            printf("\t\t%u %u %u after edge [%u,%u].\n",
//                   currentCosts[0], currentCosts[1], currentCosts[2],
//                   preimageOfPredArc->tail, preimageOfPredArc->head);
//            std::cout << "\t\tNodes: " << this->transitionNodes.at(pred->predSubset)->getNodes() << std::endl;
            //currentCosts = substract(currentCosts, preimageOfPredArc->c);
            preimageOfPredArc = &G.edges[pred->lastArc];
            const Edge& edgeInOriginalGraph = compactGraph.originalGraph.edges[compactGraph.getOriginalId(*preimageOfPredArc)];
            printf("Edge: [%u, %u] with c = (%u, %u, %u)\n", edgeInOriginalGraph.tail, edgeInOriginalGraph.head, edgeInOriginalGraph.c[0],  edgeInOriginalGraph.c[1],  edgeInOriginalGraph.c[2]);
            ++printedEdges;
            currentCosts = substract(currentCosts, preimageOfPredArc->c);
            pred = &permanents.getElement(pred->predLabelPosition);
            if (pred->predLabelPosition == std::numeric_limits<size_t>::max()) {
//                printf("\t\t%u %u %u after edge [%u,%u]\n", currentCosts[0], currentCosts[1], currentCosts[2], preimageOfPredArc->tail, preimageOfPredArc->head);
                break;
            }
        }
        for (const auto& connectedComponent: *compactGraph.connectedComponents) {
            for (ArcId edgeId : connectedComponent.edgeIds) {
                const Edge& edgeInOriginalGraph{compactGraph.originalGraph.edges[edgeId]};
                printf("Edge: [%u, %u] with c = (%u, %u, %u)\n", edgeInOriginalGraph.tail, edgeInOriginalGraph.head, edgeInOriginalGraph.c[0],  edgeInOriginalGraph.c[1],  edgeInOriginalGraph.c[2]);
                ++printedEdges;
            }
        }
        assert(printedEdges == compactGraph.originalGraph.nodesCount-1);
    }
}

size_t IGMDA::countTransitionNodes() const {
    return std::count_if(this->implicitNodes.begin(), this->implicitNodes.end(), [](const std::unique_ptr<TransitionNode>& i){return i.get() !=
            nullptr;});
}

size_t IGMDA::countTransitionArcs() const {
    size_t counter{0};
    for (auto& transitionNode : this->implicitNodes) {
        if (transitionNode == nullptr) {
            continue;
        }
        for (const auto& arcInfo : transitionNode->outgoingArcs()) {
            if (!arcInfo.chenPruned && !arcInfo.cutExitPruned) {
                ++counter;
            }
        }
    }
    return counter;
}

//void IGMDA::printParetoFront(const ConnectedComponents& blueArcsComponents) const {
//    CostArray fixedCosts = generate(0);
//    for (const auto& component : blueArcsComponents) {
//        addInPlace(fixedCosts, component.cost);
//    }
//    const std::vector<SubTree*>& spanningTrees = this->permanentTrees.at(targetNode);
//    for (const SubTree* tree : spanningTrees) {
//        for (ushort dim = 0; dim < DIM; ++dim) {
//            printf("%u ", tree->c[dim] + fixedCosts [dim]);
//        }
//        printf("\n");
//    }
//}
//
//void IGMDA::printSpanningTrees() const {
//    const std::vector<SubTree*>& spanningTrees = this->permanentTrees.at(targetNode);
//    for (const SubTree* tree : spanningTrees) {
//        printf("Tree with costs %u %u %u\n", tree->c[0], tree->c[1], tree->c[2]);
//        CostArray solutionCosts = {tree->c[0], tree->c[1]};
//        unsigned long predNodeSet = tree->predSubset;
//        size_t predLabelPos = tree->predLabelPosition;
//        const Edge& lastAddedArc{graph.edges[tree->lastTransitionArc]};
//        printf("\t%u -- %u, c = (%u, %u, %u), n = %lu\n",
//               lastAddedArc.tail, lastAddedArc.head,
//               tree->c[0], tree->c[1], tree->c[2], tree->n);
//        solutionCosts[0] -= lastAddedArc.c[0];
//        solutionCosts[1] -= lastAddedArc.c[1];
////        while (predLabelPos != std::numeric_limits<size_t>::max()) {
//        while (predNodeSet != 0) {
//            const SubTree* subTree{permanentTrees.at(predNodeSet)[predLabelPos]};
//            const Edge& lastAddedArc{graph.edges[subTree->lastTransitionArc]};
//            printf("\t%u -- %u, c = (%u, %u, %u), n = %lu\n",
//                   lastAddedArc.tail, lastAddedArc.head,
//                   subTree->c[0], subTree->c[1], subTree->c[2], subTree->n);
//            solutionCosts[0] -= lastAddedArc.c[0];
//            solutionCosts[1] -= lastAddedArc.c[1];
//            predNodeSet = subTree->predSubset;
//            predLabelPos = subTree->predLabelPosition;
//        }
//        assert(solutionCosts[0] == 0 && solutionCosts[1] == 0);
//    }
//}
