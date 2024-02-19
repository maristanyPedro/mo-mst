#ifndef BIOBJECTIVE_H_
#define BIOBJECTIVE_H_

#include <algorithm>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include "../../datastructures/includes/Label.h"
#include "Permanents.h"
#include "ImplicitNode.h"
#include "Solution.h"

class Graph;


template <typename Data = MultiPrim::SubTree>
        struct Pool;

template<typename LabelType=MultiPrim::SubTree, typename Comparator=MultiPrim::CandidateLexComp>
        class BinaryHeap;
class Preprocessor;

class IGMDA {
    typedef ImplicitNode<MultiPrim::SubTree> TransitionNode;
    public:
        explicit IGMDA(const Graph& G);
        Solution run();

    private:
        TransitionNode& getTransitionNode(const TransitionNode& predSubset, Node newNode, long unsigned decimalRepresentatio);

        TransitionNode& initTransitionNode(
                boost::dynamic_bitset<> bitRepresentation,
                long unsigned decimalRepresentation,
                const TransitionNode& predSubset,
                Node newNode);

        void nextQueueTree(const MultiPrim::SubTree* recentlyExtracted, BinaryHeap<MultiPrim::SubTree, MultiPrim::CandidateLexComp>& heap, Pool<MultiPrim::SubTree>& treesPool);

        bool propagate(const MultiPrim::SubTree* predLabel, const TransitionNode& searchNode, BinaryHeap<MultiPrim::SubTree, MultiPrim::CandidateLexComp>& H, Pool<MultiPrim::SubTree>& treesPool);

        inline bool pruned(const CostArray& c);

        void storeStatistics(Solution& sol);

        size_t countTransitionNodes() const;

        size_t countTransitionArcs() const;

    private:
        const Graph& graph;
        std::unique_ptr<Permanents> permanentTrees;
        std::unordered_map<long unsigned, TruncatedFront> truncated;
        std::vector<std::unique_ptr<TransitionNode>> implicitNodes;
        const CostArray dominanceBound;
        const size_t targetNode;
        size_t solutionsCount{0};
        size_t extractions;
        size_t insertions;
        size_t nqtIterations;
        size_t transitionArcs{0};
        size_t skipCounter{0};
    };

#endif
