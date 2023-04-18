//
// Created by bzfmaris on 28.04.22.
//

#ifndef BI_MST_DFS_H
#define BI_MST_DFS_H

#include <vector>
#include "../../datastructures/includes/typedefs.h"

class Graph;
struct Arc;

class DFS {
public:
    DFS(Graph& G, Node source, Arc& a);

    void runRed();
    void runBlue();

private:
    void searchRed(const Node startNode);

    void searchBlue(const Node startNode);
private:
    Graph& G;
    const Node source;
    Arc& relevantArc;
    std::vector<bool> visited;
};

size_t findRedArcs(Graph& G);

size_t findBlueArcs(Graph& G);

#endif //BI_MST_DFS_H
