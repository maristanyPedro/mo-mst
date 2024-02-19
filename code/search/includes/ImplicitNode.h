//
// Created by bzfmaris on 16.03.22.
//

#ifndef BI_MST_IMPLICITNODE_H
#define BI_MST_IMPLICITNODE_H

#include <memory>
#include <vector>

#include "../../datastructures/includes/typedefs.h"
#include "../../datastructures/includes/Label.h"

class Graph;
typedef std::vector<OutgoingArcInfo> OutgoingArcs;

struct PredArc {
    PredArc() = default;

    mutable MultiPrim::List nextQueueTrees;
};

template <typename LabelType>
class ImplicitNode {
    typedef std::vector<PredArc> IncomingArcs;
//    typedef std::set<PredArc> IncomingArcs;
//    typedef std::vector<PredArc> IncomingArcs;
public:
    /**
     *
     * @param originalGraph
     * @param nodes Passing by value is ok since we are moving the bitset into the 'containedNodes' member.
     */
    ImplicitNode(const Graph& originalGraph, boost::dynamic_bitset<> nodes, long unsigned index, const OutgoingArcs& existingCut, Node newNode);
    //ImplicitNode(const Graph& graph, const ImplicitNode& predSubset, Node newNode);

    ImplicitNode(const Graph &originalGraph, Node initialNode);

    void print() const {
        std::cout << "\t\t\tImplicit Node with Index: " << this->getIndex() <<  " and cardinality: " << cardinality << std::endl;
        std::cout << "\t\t\tBinary representation: " << containedNodes << std::endl;
    }

    inline size_t getCardinality() const;

    inline bool initialized() const;

    inline const OutgoingArcs& outgoingArcs() const;

    inline size_t getIndex() const;

    inline const NodesSubset& getNodes() const;

    inline IncomingArcs& getIncomingArcs() {
        return incomingArcs;
    }

    inline const PredArc& addIncomingArc();

    inline const PredArc& getIncomingArc(EdgeId id);

    inline LabelType* getQueueTree() {
        return this->queueTree;
    }

    inline const LabelType* getQueueTree() const {
        return this->queueTree;
    }

    inline void setQueueTree(LabelType* t) {
        this->queueTree = t;
    }

    inline size_t nextIncomingArcIndex() const {
        return this->incomingArcs.size();
    }

private:
    const NodesSubset containedNodes;
    const unsigned long index;
    const std::unique_ptr<OutgoingArcs> outgoing;
    IncomingArcs incomingArcs;
    LabelType* queueTree;

private:
    const size_t cardinality;

private:
    void chenPruning(const Graph& G, OutgoingArcs& outgoingArcs);

    std::unique_ptr<OutgoingArcs> computeOutgoingArcsNew(const Graph& G, const OutgoingArcs& existingCut, Node newNode);

    std::unique_ptr<OutgoingArcs> computeOutgoingArcs(const Graph& G, Node initialNode);

    inline unsigned long computeIndex();

};

template <typename LabelType>
ImplicitNode<LabelType>::ImplicitNode(const Graph& originalGraph, boost::dynamic_bitset<> nodes, long unsigned index, const OutgoingArcs& existingCut, Node newNode):
        containedNodes{std::move(nodes)},
        index{index},
        outgoing(computeOutgoingArcsNew(originalGraph, existingCut, newNode)),
        queueTree{nullptr},
        cardinality{this->containedNodes.count()} {
    //std::cout << "Constructor for growing tree: " << containedNodes << std::endl;
//    for (EdgeId aId : this->outgoingArcs()) {
//        const Edge& arc = graph.edges[aId];
//        arc.print();
//    }
//    std::cout << "Index: " << this->getIndex() << std::endl;
}

template <typename LabelType>
ImplicitNode<LabelType>::ImplicitNode(const Graph &originalGraph, Node initialNode):
        containedNodes(boost::dynamic_bitset<>(originalGraph.nodesCount, initialNode + 1)),
        index{this->computeIndex()},
        outgoing(computeOutgoingArcs(originalGraph, initialNode)),
        queueTree{nullptr},
        cardinality{1} {
//    std::cout << "Constructor for initial node: " << containedNodes << std::endl;
//    for (EdgeId aId : this->outgoingArcs()) {
//        const Edge& arc = graph.edges[aId];
//        arc.print();
//    }
//    std::cout << "Index: " << this->getIndex() << std::endl;
}

template <typename LabelType>
bool ImplicitNode<LabelType>::initialized() const {
    return this->outgoing != nullptr;
}

template <typename LabelType>
inline const PredArc& ImplicitNode<LabelType>::addIncomingArc() {
    this->incomingArcs.emplace_back();
    return this->incomingArcs.back();
}

template <typename LabelType>
inline const PredArc& ImplicitNode<LabelType>::getIncomingArc(EdgeId id) {
    assert(id < this->incomingArcs.size());
    return this->incomingArcs[id];
}

template <typename LabelType>
const OutgoingArcs& ImplicitNode<LabelType>::outgoingArcs() const {
    assert(this->initialized());
    return *(this->outgoing);
}

template <typename LabelType>
size_t ImplicitNode<LabelType>::getCardinality() const {
    return this->cardinality;
}

template <typename LabelType>
unsigned long ImplicitNode<LabelType>::computeIndex() {
    return toDecimal(this->containedNodes);
}

template <typename LabelType>
unsigned long ImplicitNode<LabelType>::getIndex() const {
    return this->index;
}

template <typename LabelType>
const NodesSubset& ImplicitNode<LabelType>::getNodes() const {
    return this->containedNodes;
}

template <typename LabelType>
void ImplicitNode<LabelType>::chenPruning(const Graph& G, OutgoingArcs& outgoingArcs) {
    for (size_t i = 0; i < outgoingArcs.size(); ++i) {
        const EdgeId edgeId = outgoingArcs[i].edgeId;
        const Edge& edge = G.edges[edgeId];
        for (size_t j = 0; j < i; ++j) {
            if (dominates(G.edges[outgoingArcs[j].edgeId].c, edge.c)) {
                //printf("For node %lu, eliminating arc %lu\n", this->index, i);
                outgoingArcs[i].chenPruned = true;
                Node newNodeInDominated = this->containedNodes[edge.head] ? edge.tail : edge.head;
                assert(!this->containedNodes[newNodeInDominated]);
                Node newNodeInDominating = this->containedNodes[G.edges[outgoingArcs[j].edgeId].head] ? G.edges[outgoingArcs[j].edgeId].tail : G.edges[outgoingArcs[j].edgeId].head;
                assert(!this->containedNodes[newNodeInDominating]);
                if (newNodeInDominated == newNodeInDominating) {
                    outgoingArcs[i].cutExitPruned = true;
                }
                break;
            }
        }
    }
}

template <typename LabelType>
std::unique_ptr<OutgoingArcs> ImplicitNode<LabelType>::computeOutgoingArcsNew(
        const Graph& G, const OutgoingArcs& existingCut, Node newNode) {
    std::unique_ptr<OutgoingArcs> result = std::make_unique<OutgoingArcs>();
    //First, add all outgoing edges from the old tree that do not end at the new node.
    for (const OutgoingArcInfo& info : existingCut) {
        const Edge& edge = G.edges[info.edgeId];
        if (info.cutExitPruned || edge.tail == newNode || edge.head == newNode) {
//        if (edge.tail == newNode || edge.head == newNode) {
            continue;
        }
        result->push_back(OutgoingArcInfo(info));
    }
    //Now, add the adjacent edges from the newNode that do not end at a node contained in the old tree.
    const Neighborhood& neighborhood{G.node(newNode).adjacentArcs};
    for (const Arc& arc : neighborhood) {
        const Edge& edge = G.edges[arc.idInEdgesVector];
        if (edge.isRed) {
            continue;
        }
        if (!this->containedNodes[arc.n]) {
            result->push_back(OutgoingArcInfo(arc.idInEdgesVector, false, false));
        }
    }
    std::sort(result->begin(), result->end(), OutgoingArcSorter(G));
    chenPruning(G, *result);
    return result;
}

template <typename LabelType>
std::unique_ptr<OutgoingArcs> ImplicitNode<LabelType>::computeOutgoingArcs(const Graph& G, Node initialNode) {
    std::unique_ptr<OutgoingArcs> result = std::make_unique<OutgoingArcs>();
    const Neighborhood& neighborhood{G.node(initialNode).adjacentArcs};
    for (const Arc& arc : neighborhood) {
        if (arc.redArc) {
            continue;
        }
        result->push_back(OutgoingArcInfo(arc.idInEdgesVector, false, false));
    }
    std::sort(result->begin(), result->end(), OutgoingArcSorter(G));
    chenPruning(G, *result);
    return result;
}

#endif //BI_MST_IMPLICITNODE_H
