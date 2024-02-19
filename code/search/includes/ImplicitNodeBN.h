#ifndef BN_IMPLICITNODE_H
#define BN_IMPLICITNODE_H

#include <memory>
#include <vector>
#include <iostream>

#include "../../datastructures/includes/typedefs.h"
#include "../../datastructures/includes/Label.h"
#include "../../datastructures/includes/graph.h"

template <typename LabelType>
class ImplicitNodeBN {
public:
    /**
     *
     * @param originalGraph
     * @param nodes Passing by value is ok since we are moving the bitset into the 'containedNodes' member.
     */
    ImplicitNodeBN(boost::dynamic_bitset<> nodes, long unsigned index);
    //ImplicitNode(const Graph& graph, const ImplicitNode& predSubset, Node newNode);

    ImplicitNodeBN(const Graph &originalGraph, Node initialNode);

    void print() const {
        std::cout << "\t\t\tImplicit Node with Index: " << this->getIndex() <<  " and cardinality: " << cardinality << std::endl;
        std::cout << "\t\t\tBinary representation: " << containedNodes << std::endl;
    }

    inline size_t getCardinality() const;

    inline bool initialized() const;

    inline size_t getIndex() const;

    inline const NodesSubset& getNodes() const;

private:
    const NodesSubset containedNodes;
    const unsigned long index;

private:
    const size_t cardinality;

private:

    inline unsigned long computeIndex();

};

template <typename LabelType>
ImplicitNodeBN<LabelType>::ImplicitNodeBN(boost::dynamic_bitset<> nodes, long unsigned index):
        containedNodes{std::move(nodes)},
        index{index},
        cardinality{this->containedNodes.count()} {
    //std::cout << "Constructor for growing tree: " << containedNodes << std::endl;
//    for (EdgeId aId : this->outgoingArcs()) {
//        const Edge& arc = graph.edges[aId];
//        arc.print();
//    }
//    std::cout << "Index: " << this->getIndex() << std::endl;
}

template <typename LabelType>
ImplicitNodeBN<LabelType>::ImplicitNodeBN(const Graph &originalGraph, Node initialNode):
        containedNodes(boost::dynamic_bitset<>(originalGraph.nodesCount, initialNode + 1)),
        index{this->computeIndex()},
        cardinality{1} {
//    std::cout << "Constructor for initial node: " << containedNodes << std::endl;
//    for (EdgeId aId : this->outgoingArcs()) {
//        const Edge& arc = graph.edges[aId];
//        arc.print();
//    }
//    std::cout << "Index: " << this->getIndex() << std::endl;
}

template <typename LabelType>
bool ImplicitNodeBN<LabelType>::initialized() const {
    return this->outgoing != nullptr;
}

template <typename LabelType>
size_t ImplicitNodeBN<LabelType>::getCardinality() const {
    return this->cardinality;
}

template <typename LabelType>
unsigned long ImplicitNodeBN<LabelType>::computeIndex() {
    return toDecimal(this->containedNodes);
}

template <typename LabelType>
unsigned long ImplicitNodeBN<LabelType>::getIndex() const {
    return this->index;
}

template <typename LabelType>
const NodesSubset& ImplicitNodeBN<LabelType>::getNodes() const {
    return this->containedNodes;
}

#endif