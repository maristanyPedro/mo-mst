#ifndef BN_H_
#define BN_H_

#include <algorithm>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include "../../datastructures/includes/Label.h"
#include "../../datastructures/includes/BinaryHeap.h"
#include "../../datastructures/includes/MemoryPool.h"
#include "../../search/includes/Permanents.h"
#include "ImplicitNodeBN.h"
#include "Solution.h"

class Graph;
class Preprocessor;

namespace BN {
    typedef std::vector<PermanentQueueTree> Front;
    typedef std::list<QueueTree*> OpenCosts;
    class MultiobjectiveSearch {
        typedef ImplicitNodeBN<QueueTree> TransitionNode;
    public:
        explicit MultiobjectiveSearch(const Graph &G);

        Solution run();

        void printParetoFront(const std::list<QueueTree*>& targetFront, const ConnectedComponents &blueArcsComponents) const;

    private:
        EdgeId retrieveEdgeId(const QueueTree* efficientTree) const;

        TransitionNode &getSubset(const TransitionNode &predSubset, Node newNode);

        TransitionNode &intializeSubset(
                boost::dynamic_bitset<> bitRepresentation,
                long unsigned decimalRepresentation);

        bool buildAndAnalyze(
                QueueTree* efficientSubtree,
                const TransitionNode& transitionNodeForEfficientSubtree,
                BinaryHeap<QueueTree, BN::CandidateLexComp>& H,
                Node lastTail,
                const Arc& cutArc,
                NeighborhoodSize cutArcPosition,
                Node newTreeNode);

        bool propagate(QueueTree* efficientTree, const TransitionNode &transitionNode, BinaryHeap<QueueTree, BN::CandidateLexComp> &H);

        inline bool pruned(const CostArray &c) const;

        bool merge(OpenCosts& open, QueueTree* newLabel);

        void clean(OpenCosts& open, QueueTree* newLabel);

        void storeStatistics(Solution &sol);

        size_t countTransitionNodes() const;

        size_t countTransitionArcs() const;

    private:
        const Graph &G;
        Pool<QueueTree> treePool;
        std::vector<std::unique_ptr<TransitionNode>> implicitNodes;
        std::unordered_map<long unsigned, TruncatedFront> truncated;
        std::unique_ptr<Permanents> permanentTrees;
        const CostArray dominanceBound;
        const size_t targetNode;
        size_t extractions;
        size_t insertions;
        size_t nqtIterations;
    };

    bool MultiobjectiveSearch::pruned(const CostArray& c) const {
        return weakDominates(this->dominanceBound, c);// || isDominated(this->permanentTrees.at(targetNode), c);
    }

    struct ArcSorter {
        inline bool operator() (const Arc& lhs, const Arc& rhs) const {
            return lhs.n < rhs.n;
        }
    };
}
#endif
