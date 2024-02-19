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

using namespace MultiPrim;

inline static boost::dynamic_bitset<> addNode(const boost::dynamic_bitset<>& existing, Node newNode) {
    boost::dynamic_bitset<> n = existing;
    n[newNode] = true;
    return n;
}

IGMDA::IGMDA(const Graph &G):
        graph{G},
        permanentTrees(std::make_unique<Permanents>()),
        implicitNodes((1UL<<(this->graph.nodesCount - 1))),
        dominanceBound(generate(MAX_COST)),
        targetNode{(1UL<<(this->graph.nodesCount - 1)) - 1},
        extractions{0},
        insertions{0},
        nqtIterations{0} {
            if (targetNode == 0 && graph.arcsCount > 0) {
                printf("Graph is too big. Leads to overflow computing target implicit node id. Abort\n");
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
    std::unique_ptr<TransitionNode> newTransitionNode =
            std::make_unique<TransitionNode>(this->graph, std::move(bitRepresentation), decimalRepresentation, predSubset.outgoingArcs(), newNode);
    long unsigned index = newTransitionNode->getIndex();
    this->implicitNodes[index] = std::move(newTransitionNode);
    return *this->implicitNodes[index];
}

IGMDA::TransitionNode& IGMDA::getTransitionNode(const TransitionNode& predSubset, Node newNode, long unsigned decimalRepresentation) {
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

Solution IGMDA::run() {
    if (this->graph.arcsCount == 0) {
        return Solution();
    }
    Pool<SubTree> treesPool;
    SubTree* initialTree = treesPool.newItem();
    initialTree->n = 0; initialTree->c = generate(0);
    std::unique_ptr<TransitionNode> initialImplicitNode = std::make_unique<TransitionNode>(this->graph, 0);
    initialImplicitNode->setQueueTree(initialTree);
    this->implicitNodes[initialImplicitNode->getIndex()] = std::move(initialImplicitNode);
    Solution solution;
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
        assert(searchNode.getIndex() == currentNode);

        truncatedInsertion(truncated[currentNode], minTree->c);
        nextQueueTree(minTree, heap, treesPool);
        if (currentNode == targetNode) {
            size_t solutionIndex = this->permanentTrees->getCurrentIndex();
            permanentTrees->addElement(minTree->predLabelPosition, minTree->lastEdgeId);
            assert(permanentTrees->getElement(solutionIndex).lastArc == minTree->lastEdgeId && permanentTrees->getElement(solutionIndex).predLabelPosition == minTree->predLabelPosition);
            solution.spanningTreeIndices.push_back(solutionIndex);
            ++solutionsCount;
            continue;
        }

        bool success = propagate(minTree, searchNode, heap, treesPool);
        if (success) {
            permanentTrees->addElement(minTree->predLabelPosition, minTree->lastEdgeId);
        }
        treesPool.free(minTree);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    solution.time = duration.count();
    storeStatistics(solution);

    return solution;
}

void IGMDA::nextQueueTree(const SubTree* minTree, BinaryHeap<SubTree, CandidateLexComp>& heap, Pool<SubTree>& treesPool) {
    long unsigned n{minTree->n};
    TransitionNode& searchNode{*this->implicitNodes[n]};
    SubTree* newQueueTree = nullptr;
    std::vector<PredArc>& predArcs = searchNode.getIncomingArcs();
    const TruncatedFront& currentTruncatedFront{this->truncated[searchNode.getIndex()]};
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
                if (candidateTree->nclChecked || !truncatedDominance(currentTruncatedFront, candidateTree->c)) {
                    candidateTree->nclChecked = true;
                    success = true;
                    newQueueTree = candidateTree;
                    minCandidates = &predSubTrees;
                    break;
                }
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
    const auto& outgoingArcs = searchNode.outgoingArcs();
    CostArray costCandidate;
    bool success = false;
    //If the propagation yields some non-dominated subtrees, predLabel will be made permanent. predIndex indicates
    //the position in which it will be stored.
    const size_t predIndex = this->permanentTrees->getCurrentIndex();
    for (const OutgoingArcInfo& outgoingArcInfo : outgoingArcs) {
        //Pruning by Chen when constructing set of outgoing edges for searchNode determined that this arc is not active.
        if (outgoingArcInfo.chenPruned || outgoingArcInfo.cutExitPruned) {
            continue;
        }
        EdgeId aId = outgoingArcInfo.edgeId;
        const Edge& edge{this->graph.edges[aId]};
        assert(aId == edge.id);
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

        SubTree* queueTree = getQueueTree(successorNode, treesPool);
        if (queueTree->inQueue) {
            SubTree* newLabel = treesPool.newItem();
            newLabel->update(successorNode.getIndex(), costCandidate, outgoingArcInfo.incomingArcId, edge.id, predIndex);
            if (lexSmaller(costCandidate, queueTree->c)) {
                if (truncatedDominance(this->truncated[successorNode.getIndex()], costCandidate)) {
                    continue;
                }
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
                predArc.nextQueueTrees.push_back(newLabel);
            }
        }
        else {
            if (truncatedDominance(this->truncated[successorNode.getIndex()], costCandidate)) {
                continue;
            }
            success = true;
            queueTree->update(successorNode.getIndex(), costCandidate, outgoingArcInfo.incomingArcId, edge.id, predIndex);
//            printf("\t\t\t\tNo queue tree and is %u %u %u, I'm queue!\n",
//                   queueTree->c[0], queueTree->c[1], queueTree->c[2]);
            assert(queueTree->n == successorNode.getIndex());
            //printf("\n\nPushing (%u, %u) for index %lu\n", cr1, cr2, queueTree.n);
            H.push(queueTree);
        }
    }
    return success;
}

void IGMDA::storeStatistics(Solution &sol) {
    sol.trees = sol.spanningTreeIndices.size();
    sol.insertions = insertions;
    sol.extractions = extractions;
    sol.nqtIt = nqtIterations;
    sol.transitionArcsCount = this->transitionArcs;
    sol.transitionNodes = countTransitionNodes();
    sol.transitionArcs = countTransitionArcs();
    sol.permanents = std::move(this->permanentTrees);
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

