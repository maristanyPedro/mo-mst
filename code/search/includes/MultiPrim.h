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

template <typename Data = SubTree>
        struct Pool;

template<typename LabelType=SubTree, typename Comparator=CandidateLexComp>
        class BinaryHeap;
class Preprocessor;

namespace NEW_GENERATION {

    class IGMDA {
    typedef ImplicitNode<SubTree> TransitionNode;
    public:
        explicit IGMDA(const Graph& G);
        Solution run(const GraphCompacter& compactGraph);
        void printParetoFront(const ConnectedComponents& blueArcsComponents) const;

    private:
        TransitionNode& getTransitionNode(const TransitionNode& predSubset, Node newNode, long unsigned decimalRepresentatio);

        TransitionNode& initTransitionNode(
                boost::dynamic_bitset<> bitRepresentation,
                long unsigned decimalRepresentation,
                const TransitionNode& predSubset,
                Node newNode);

        void nextQueueTree(const SubTree* recentlyExtracted, BinaryHeap<SubTree, CandidateLexComp>& heap, Pool<SubTree>& treesPool);

        bool propagate(const SubTree* predLabel, const TransitionNode& searchNode, BinaryHeap<SubTree, CandidateLexComp>& H, Pool<SubTree>& treesPool);

        static void printSpanningTrees(const std::list<SubTree*>& solutions, const Permanents& permanents, const Graph& G, const GraphCompacter& compactGraph);

        inline bool pruned(const CostArray& c);

        void storeStatistics(Solution& sol, std::list<SubTree*>& solutions) const;

        size_t countTransitionNodes() const;

        size_t countTransitionArcs() const;

    private:
        const Graph& graph;
        Permanents permanentTrees;
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

    bool IGMDA::pruned(const CostArray& c)  {
        //return weakDominates(this->dominanceBound, c) || truncatedDominance(this->truncated[targetNode], c);
        return truncatedDominance(this->truncated[targetNode], c);
        //return false;
    }
}

#endif
