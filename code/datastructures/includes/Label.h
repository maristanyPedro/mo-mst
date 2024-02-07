//
// Created by bzfmaris on 25.01.22.
//

#ifndef BI_MST_LABEL_H
#define BI_MST_LABEL_H

#include <cassert>
#include <unordered_map>

#include "typedefs.h"

struct SubTree{
    SubTree() = default;

//    SubTree(long unsigned n, const CostArray& c):
//        c{c}, n{n} {}

    inline void initialize(Node newNode) {
        predLabelPosition = std::numeric_limits<size_t>::max();
        lastTransitionArc = std::numeric_limits<uint32_t>::max();
        n = newNode;
//        this->knownTargetElements = 0;
        this->nclChecked = false;
        inQueue = false;
        //c = generate(MAX_COST);
        assert(!inQueue);
    }

    inline void update(Node nNew, const CostArray& cNew, ArcId la, size_t plp) {
        this->n = nNew; this->c = cNew; this->lastTransitionArc = la;
        this->predLabelPosition = plp;
//        this->knownTargetElements = kTe;
        this->nclChecked = false;
    }

    void print() const {
        printf("%lu c=(%u, %u) lastTransitionArc=%d\n", n, c[0], c[1], lastTransitionArc);
    }

    CostArray c{generate(MAX_COST)};
    size_t predLabelPosition{std::numeric_limits<size_t>::max()};
    ArcId lastTransitionArc{std::numeric_limits<uint32_t>::max()};
    long unsigned n{INVALID_NODE};
    uint32_t priority{std::numeric_limits<uint32_t>::max()}; ///< for heap operations.
    SubTree* next{nullptr};
//    uint16_t knownTargetElements{0};
    bool nclChecked{false};
    bool inQueue{false};
};

class List {
public:
    SubTree* first{nullptr};
    SubTree* last{nullptr};
    size_t size{0};

    inline void push_back(SubTree* ec) {
        if (this->empty()) {
            this->first = ec;
            this->last = ec;
            ec->next = nullptr;
        }
        else {
            this->last->next = ec;
            this->last = ec;
            ec->next = nullptr;
        }
        ++size;
    }

    inline void push_front(SubTree* l) {
        if (this->empty()) {
            this->first = l;
            this->last = l;
            l->next = nullptr;
        }
        else {
            l->next = this->first;
            this->first = l;
        }
        ++size;
    }

    inline SubTree* pop_front() {
        if (this->empty()) {
            return nullptr;
        }
        else if (this->size == 1) {
            this->first = nullptr;
            this->last = nullptr;
            --size;
            return nullptr;
        }
        else {
            SubTree* front = this->first;
            this->first = front->next;
            --size;
            return this->first;
        }
    }

    inline bool empty() const {
        return this->size == 0;
    }
};

//bool equal(const List& l1, const List& l2) {
//    SubTree* it1 = l1.first;
//    SubTree* it2 = l2.first;
//    while (it1 != nullptr && it2 != nullptr) {
//        if (it1 != it2) {
//            return false;
//        }
//        it1 = it
//    }
//}

struct CandidateLexComp {
    inline bool operator() (const SubTree* lhs, const SubTree* rhs) const {
        return lexSmaller(lhs->c, rhs->c);
    }
};

namespace BN {
    struct QueueTree {
        QueueTree() = default;

        QueueTree(long unsigned n, const CostArray &c) :
                c{c}, n{n} {}

        void print() const {
            printf("%lu c=(%u, %u) lastTransitionArc=%d\n", n, c[0], c[1], lastArc);
        }

        CostArray c{generate(MAX_COST)};
        std::vector<Node> addedNodesInOrder;
        std::unordered_map<Node, size_t> addedNode2Index;
        long unsigned predSubset{std::numeric_limits<long unsigned>::max()};
        Node lastTail{0};
        Node lastHead{0};
        ArcId lastArc{std::numeric_limits<ArcId>::max()};
        size_t predLabelPosition{std::numeric_limits<size_t>::max()};
        long unsigned n{INVALID_NODE};
        uint32_t priority{std::numeric_limits<uint32_t>::max()}; ///< for heap operations.
        QueueTree* next{nullptr};
        bool inQueue{false};
    };

    struct PermanentQueueTree {
        PermanentQueueTree() = default;

        PermanentQueueTree(const QueueTree* qt) :
        predSubset{qt->predSubset}, lastArc{qt->lastArc}, predLabelPosition{qt->predLabelPosition} {}

        long unsigned predSubset{std::numeric_limits<long unsigned>::max()};
        ArcId lastArc{std::numeric_limits<ArcId>::max()};
        size_t predLabelPosition{std::numeric_limits<size_t>::max()};
    };

    struct CandidateLexComp {
        inline bool operator()(const QueueTree *lhs, const QueueTree *rhs) const {
            return lexSmaller(lhs->c, rhs->c);
        }
    };
}

#endif

