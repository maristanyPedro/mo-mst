#ifndef SOLUTION_H_
#define SOLUTION_H_

#include <vector>
#include <list>
#include "./Permanents.h"

class GraphCompacter;


struct Solution {
    Solution():
        permanents(std::make_unique<Permanents>()),
        trees{1},
        extractions{0},
        insertions{0},
        nqtIt{0},
        transitionArcsCount{0},
        transitionNodes{0},
        transitionArcs{0},
        time{0} {}

    void printSpanningTrees(const GraphCompacter& compactGraph);


    std::vector<size_t> spanningTreeIndices;
    std::unique_ptr<Permanents> permanents;

    std::size_t trees{0};
    std::size_t extractions{0};
    std::size_t insertions{0};
    std::size_t nqtIt{0};
    std::size_t transitionArcsCount{0};
    std::size_t transitionNodes{0};
    std::size_t transitionArcs{0};
    double time{0};
};


#endif