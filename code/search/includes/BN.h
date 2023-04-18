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
        TransitionNode &getSubset(const TransitionNode &predSubset, Node newNode);

        TransitionNode &intializeSubset(
                boost::dynamic_bitset<> bitRepresentation,
                long unsigned decimalRepresentation);

        bool buildAndAnalyze(
                QueueTree* predLabel,
                const TransitionNode& searchNode,
                BinaryHeap<QueueTree, BN::CandidateLexComp>& H,
                Node lastTail,
                const Arc& edge,
                ArcId aId,
                //std::pair<Node, Node>& orientedArc,
                Node newTreeNode);

        bool propagate(QueueTree* predLabel, const TransitionNode &searchNode, BinaryHeap<QueueTree, BN::CandidateLexComp> &H);

        void printSpanningTrees(const std::list<QueueTree*>& targetFront) const;

        inline bool pruned(const CostArray &c) const;

        bool merge(OpenCosts& open, QueueTree* newLabel);

        void clean(OpenCosts& open, QueueTree* newLabel);

        void storeStatistics(Solution &sol, size_t solutionsCount) const;

        size_t countTransitionNodes() const;

        size_t countTransitionArcs() const;

    private:
        const Graph &G;
        Pool<QueueTree> treePool;
        std::vector<std::unique_ptr<TransitionNode>> implicitNodes;
        std::unordered_map<long unsigned, TruncatedFront> truncated;
        std::vector<Front> permanentTrees;
        //std::unordered_map<long unsigned, std::unique_ptr<ImplicitNode>> test;
        const CostArray dominanceBound;
        const size_t targetNode;
        size_t extractions;
        size_t insertions;
        size_t nqtIterations;
    };

    bool MultiobjectiveSearch::pruned(const CostArray& c) const {
        return weakDominates(this->dominanceBound, c);// || isDominated(this->permanentTrees.at(targetNode), c);
    }
}
#endif
