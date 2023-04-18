//
// Created by bzfmaris on 03.05.22.
//

#ifndef BI_MST_PREPROCESSOR_H
#define BI_MST_PREPROCESSOR_H

#include <vector>
#include "boost/dynamic_bitset.hpp"

#include "../../datastructures/includes/typedefs.h"

class Graph;
class GraphCompacter;

struct Label {

    inline void update(const CostArray& cNew, Node n) {
        //assert(pathId <= std::numeric_limits<u_int16_t>::max());
        this->c = cNew;
        this->n = n;
    }

    uint32_t priority{std::numeric_limits<uint32_t>::max()}; ///< for heap operations.
    CostArray c{generate(MAX_COST)};
    Node n = INVALID_NODE;
    bool inQueue = false;
};

struct LexComparison {
    inline bool operator() (const Label* lhs, const Label* rhs) const {
        return lexSmaller(lhs->c, rhs->c);
    }
};

class Preprocessor {
public:
    explicit Preprocessor();

    GraphCompacter run(Graph& G);

    void calculateHeuristic(const Graph& G);

    //Graph& originalGraph;
    //Entries of these vector are indexed from 0 to number of nodes - 1. Entry k stands for spanning trees
    //of every subset of nodes containing n nodes and contains the cheapest possible way of connecting n-k
    //nodes using a spanning tree of cardinality n-k.
    std::vector<CostArray> lb;
    //Initializing to MAX_COST implies that it does not prune away any solution.
    CostArray dominanceBound{generate(0)};
    double duration{0};
private:
    static CostArray Prim(const Graph& G, Node root, const DimensionsVector& dimOrdering);
    static CostArray Prim(const Graph& G, boost::dynamic_bitset<> containedNodes, const DimensionsVector& dimOrdering);
    void computeLowerBounds(const Graph& G);
};

#endif //BI_MST_PREPROCESSOR_H
