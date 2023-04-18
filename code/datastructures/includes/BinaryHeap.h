//
// Created by bzfmaris on 25.01.22.
//

#ifndef BI_MST_BINARYHEAP_H
#define BI_MST_BINARYHEAP_H

#include <cassert>
#include <vector>

#include "typedefs.h"

template <typename LabelType, typename Comparator>
class BinaryHeap {
public:

    explicit BinaryHeap()
            : lastElementIndex(0), heapElements(0) {
        this->heapElements.resize(1);
    }

    void decreaseKey(LabelType* lOld, LabelType* lNew) {
        lNew->priority = lOld->priority;
        heapElements[lNew->priority] = lNew;
        up(lNew->priority);
        lNew->inQueue = true;
        lOld->inQueue = false;
    }

    void decreaseKey(LabelType *val) {
        assert(val->priority < lastElementIndex);
        up(val->priority);
    }

    void decreaseKey(size_t prio, LabelType *newVal) {
        (void)prio;
        newVal->inQueue = true;
        this->heapElements[newVal->priority] = newVal;
        assert(newVal->priority < lastElementIndex);
        up(newVal->priority);
    }

    void push(LabelType *val) {
        if (lastElementIndex + 1 > heapElements.size()) {
            heapElements.resize(heapElements.size() * 2);
        }
        heapElements[lastElementIndex] = val;

        val->priority = lastElementIndex;
        size_t temp = lastElementIndex;
        lastElementIndex++;
        up(temp);
        val->inQueue = true;
        size_t nodeElements = 0;
        for (auto& element : heapElements) {
            if (element && element->inQueue) {
                if (element->n == val->n) {
                    ++nodeElements;
                }
            }
        }
        //assert (nodeElements <= 1);
    }

    LabelType *pop() {
        assert(this->size() != 0);

        LabelType *ans = heapElements[0];
        lastElementIndex--;

        if (lastElementIndex > 0) {
            heapElements[0] = heapElements[lastElementIndex];
            heapElements[0]->priority = 0;
            down(0);
        }
        ans->inQueue = false;
        return ans;
    }

    inline bool contains(LabelType *n) {
        size_t priority = n->priority;
        if (priority < lastElementIndex && &*n == &*heapElements[priority]) {
            return true;
        }
        return false;
    }

    inline bool contains(Node n) const {
        for (size_t i = 0; i < lastElementIndex; ++i) {
            if (heapElements[i]->n == n) {
                return true;
            }
        }
        return false;
    }

    inline size_t
    size() const {
        return lastElementIndex;
    }

private:
    size_t lastElementIndex;
    std::vector<LabelType *> heapElements;
    Comparator comparator;

    void up(size_t index) {
        assert(index < lastElementIndex);
        while (index > 0) {
            size_t parent = (index - 1) >> 1; //Shift right dividing by 2

            if (comparator(heapElements[index], heapElements[parent])) {
                swap(parent, index);
                index = parent;
            } else { break; }
        }
    }

    void down(size_t index) {
        size_t first_leaf_index = lastElementIndex >> 1;
        while (index < first_leaf_index) {
            // find smallest (or largest, depending on heap type) child
            size_t child1 = (index << 1) + 1;
            size_t child2 = (index << 1) + 2;
            size_t which = child1;
            if ((child2 < lastElementIndex) &&
                comparator(heapElements[child2], heapElements[child1])) { which = child2; }

            // swap child with parent if necessary
            if (comparator(heapElements[which], heapElements[index])) {
                swap(index, which);
                index = which;
            } else { break; }
        }
    }

    inline void swap(size_t index1, size_t index2) {
        LabelType *tmp1 = heapElements[index1];
        heapElements[index1] = heapElements[index2];
        heapElements[index2] = tmp1;
        heapElements[index1]->priority = index1;
        heapElements[index2]->priority = index2;

    }

//    std::vector<size_t> countElements(Node n) const {
//        std::vector<size_t> indices;
//        for (size_t i = 0; i < this->lastElementIndex; ++i) {
//            if (this->heapElements[i]->n == n) {
//                indices.push_back(i);
//            }
//        }
//        return indices;
//    }
};

#endif //BI_MST_BINARYHEAP_H
