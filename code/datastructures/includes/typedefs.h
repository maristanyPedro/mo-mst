//
// Created by bzfmaris on 24.01.22.
//

#ifndef BI_MST_TYPEDEFS_H
#define BI_MST_TYPEDEFS_H

//
// Created by pedro on 10.08.21.
//
#include <array>
#include <cstddef> //For size_t.
#include <limits>
#include <list>
#include <numeric> //iota
#include <cstdint>
#include <cinttypes>
#include <iostream>

#include "boost/dynamic_bitset.hpp"

typedef uint32_t Node;
typedef uint16_t NeighborhoodSize;
typedef uint32_t ArcId;
typedef uint32_t CostType;

typedef uint32_t CostType;
typedef unsigned short Dimension;

constexpr Dimension DIM = 3;

constexpr Node INVALID_NODE = std::numeric_limits<Node>::max();
constexpr NeighborhoodSize MAX_DEGREE = std::numeric_limits<NeighborhoodSize>::max();
constexpr ArcId INVALID_ARC = std::numeric_limits<ArcId>::max();
constexpr CostType MAX_COST = std::numeric_limits<CostType>::max();
constexpr uint16_t MAX_PATH = std::numeric_limits<uint16_t>::max();
typedef boost::dynamic_bitset<> NodesSubset;

template <typename T>
using Info = std::array<T, DIM>;
typedef Info<CostType> CostArray;

inline void initialize(CostArray& c, CostType value) {
    std::fill(c.begin(), c.end(), value);
}

inline CostArray generate(CostType value) {
    CostArray c;
    std::fill(c.begin(), c.end(), value);
    return c;
}

inline void printCosts(const CostArray& c) {
    for (Dimension i = 0; i < DIM; ++i) {
        printf("%u ", c[i]);
    }
}

typedef Info<Dimension> DimensionsVector;

inline DimensionsVector standardSorting() {
    DimensionsVector dimO;
    std::iota(std::begin(dimO), std::end(dimO), 0);
    return dimO;
}

inline CostArray sorted(const CostArray& c, const DimensionsVector& dims) {
    CostArray cNew;
    for (Dimension i = 0; i < DIM; ++i) {
        cNew[i] = c[dims[i]];
    }
    return cNew;
}

inline bool lexSmaller(const CostArray& lh, const CostArray& rh) {
    for (size_t i = 0; i < DIM; ++i) {
        if (lh[i] < rh[i]) {
            return true;
        }
        else if (lh[i] == rh[i]) {
            continue;
        }
        else {
            return false;
        }
    }
    //Happens if both arrays coincide.
    return false;
}

inline bool lexSmaller(const CostArray& lh, const CostArray& rh, const DimensionsVector& dim) {
    for (size_t i = 0; i < DIM; ++i) {
        if (lh[dim[i]] < rh[dim[i]]) {
            return true;
        }
        else if (lh[dim[i]] == rh[dim[i]]) {
            continue;
        }
        else {
            return false;
        }
    }
    return false;
}

template <typename T>
using DimReductedInfo = std::array<T, DIM-1>;
typedef DimReductedInfo<CostType> TruncatedCosts;
typedef std::list<TruncatedCosts> TruncatedFront;

inline bool lexSmaller(const TruncatedCosts& lh, const TruncatedCosts& rh) {
    for (size_t i = 0; i < DIM-1; ++i) {
        if (lh[i] < rh[i]) {
            return true;
        }
        else if (lh[i] == rh[i]) {
            continue;
        }
        else {
            return false;
        }
    }
    //Happens if both arrays coincide.
    return false;
}

inline TruncatedCosts truncate(const CostArray& c) {
    TruncatedCosts tc;
    for (Dimension i = 1; i < DIM; ++i) {
        tc[i-1] = c[i];
    }
    return tc;
}

inline bool tc_dominates(const TruncatedCosts& lhs, const TruncatedCosts& rhs) {
    for (size_t i = 0; i < DIM-1; ++i) {
        if (lhs[i] > rhs[i]) {
            return false;
        }
    }
    return true;
//    return lhs[0] <= rhs[0] && lhs[1] <= rhs[1] && lhs[2] <= rhs[2];
}

inline bool tc_dominates(const TruncatedCosts& lhs, const CostArray& rhs) {
    for (size_t i = 0; i < DIM-1; ++i) {
        if (lhs[i] > rhs[i+1]) {
            return false;
        }
    }
    return true;
//    return lhs[0] <= rhs[0] && lhs[1] <= rhs[1] && lhs[2] <= rhs[2];
}

inline void truncatedInsertion(TruncatedFront& front, const CostArray& c) {
    TruncatedCosts tc = truncate(c);
    if (front.empty()) {
        front.push_back(tc);
        return;
    }
    auto it = front.begin();
    while (it != front.end() && lexSmaller(*it, tc)) {
        ++it;
    }
    it = front.emplace(it, tc);
    ++it;
    while (it != front.end()) {
        if (tc_dominates(tc, *it)) {
            it = front.erase(it);
        } else {
            ++it;
        }
    }
}

inline bool lexSmallerOrEquiv(const TruncatedCosts& lh, const CostArray& rh) {
    for (size_t i = 0; i < DIM-1; ++i) {
        if (lh[i] < rh[i+1]) {
            return true;
        }
        else if (lh[i] == rh[i+1]) {
            continue;
        }
        else {
            return false;
        }
    }
    //Happens if both arrays coincide.
    return true;
}

inline bool truncatedDominance(const TruncatedFront& front, const CostArray& c) {
    if (front.empty()) {
        return false;
    }
    auto it = front.begin();
    while (it != front.end() && lexSmallerOrEquiv(*it, c)) {
        if (tc_dominates(*it, c)) {
             return true;
        }
        ++it;
    }
    return false;
}

inline const CostArray substract(const CostArray& rhs, const CostArray& lhs) {
    CostArray res;
    for (size_t i = 0; i < DIM; ++i) {
        res[i] = rhs[i] - lhs[i];
    }
    return res;
}

inline void addInPlace(CostArray& rhs, const CostArray& lhs) {
    for (size_t i = 0; i < DIM; ++i) {
        rhs[i] += lhs[i];
    }
}

inline CostArray add(const CostArray& rhs, const CostArray& lhs) {
    CostArray res;
    for (size_t i = 0; i < DIM; ++i) {
        res[i] = rhs[i] + lhs[i];
    }
    return res;
}

//Dominance relationship that is also true if lhs = rhs.
inline bool dominates(const CostArray& lhs, const CostArray& rhs) {
    for (size_t i = 0; i < DIM; ++i) {
        if (lhs[i] > rhs[i]) {
            return false;
        }
    }
    return true;
//    return lhs[0] <= rhs[0] && lhs[1] <= rhs[1] && lhs[2] <= rhs[2];
}

//Dominance relationship that is false if lhs = rhs.
//size_t weakDomSuccess{0};
inline bool weakDominates(const CostArray& lhs, const CostArray& rhs) {
    for (size_t i = 0; i < DIM; ++i) {
        if (lhs[i] >= rhs[i]) {
            return false;
        }
    }
    return true;
//    return lhs[0] < rhs[0] && lhs[1] < rhs[1] && lhs[2] < rhs[2];
}

inline CostArray max(const CostArray& c1, const CostArray& c2, const DimensionsVector& dimOrdering) {
    CostArray result;
    for (size_t i = 0; i < DIM; ++i) {
        result[dimOrdering[i]] = std::max(c1[dimOrdering[i]], c2[i]);
    }
    return result;
}

inline static unsigned long toDecimal(const NodesSubset& binary) {
    bool fits_in_ulong = true;
    unsigned long ul = 0;
    try {
        //Our node subsets contain n nodes. However, since the spanning trees all contain node 0, we only need
        //2^(n-1) different node-subsets. As a consequence, we divide the computed index by two.
        ul = binary.to_ulong()/2;
    } catch(std::overflow_error &) {
        fits_in_ulong = false;
    }

    if(fits_in_ulong) {
        return ul;
    } else {
        std::cout << "(overflow exception)" << std::endl;
        exit(1);
        return 0;
    }
}


#endif //BI_MST_TYPEDEFS_H
