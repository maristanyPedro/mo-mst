#ifndef PERMANENTS_H_
#define PERMANENTS_H_

#include <cassert>
#include <vector>
#include "../../datastructures/includes/typedefs.h"

typedef std::pair<size_t , uint16_t> PredLabel;
constexpr uint16_t labelsPerRow = 1000;

struct PermanentTree {
    PermanentTree() = default;

    PermanentTree(size_t pLp, EdgeId la):
        predLabelPosition{pLp}, lastArc{la} {}

    //long unsigned predSubset{std::numeric_limits<long unsigned>::max()};
    size_t predLabelPosition{std::numeric_limits<size_t>::max()};
    EdgeId lastArc{std::numeric_limits<uint32_t>::max()};
};

class Permanents {
public:
    Permanents():
            elements(1)
    {}

    inline void addElement(size_t predIndex, EdgeId predArcId) {
        assert(currentIndex < labelsPerRow);
        if (currentIndex < labelsPerRow) {
            elements.back()[currentIndex].predLabelPosition = predIndex;
//            elements.back()[currentIndex].predSubset = pS;
            elements.back()[currentIndex].lastArc = predArcId;
            this->increaseIndex();
        }
    }

    inline size_t getCurrentIndex() const {
        assert(currentIndex < labelsPerRow);
        return (this->elements.size()-1)*labelsPerRow + this->currentIndex;
    }

    inline const PermanentTree& getElement(size_t index) const {
        size_t rowIndex = index/labelsPerRow;
        size_t indexInRow = index%labelsPerRow;
        return this->elements[rowIndex][indexInRow];
    }

    inline size_t size() const {
        return (this->elements.size()-1)*labelsPerRow + currentIndex;
    }

private:
    void increaseIndex() {
        if (currentIndex + 1 == labelsPerRow) {
            this->elements.emplace_back(std::array<PermanentTree, labelsPerRow>());
            currentIndex = 0;
        }
        else {
            ++currentIndex;
        }
    }

    std::vector<std::array<PermanentTree, labelsPerRow>> elements;
    uint16_t currentIndex{0};
};

#endif