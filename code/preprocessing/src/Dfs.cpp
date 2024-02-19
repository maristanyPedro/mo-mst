//
// Created by bzfmaris on 28.04.22.
//

#include <cassert>

#include "../../datastructures/includes/graph.h"
#include "../includes/Dfs.h"

using namespace std;

DFS::DFS(Graph &G, Node source, Arc &a):
    G{G}, source{source}, relevantArc{a}, visited(G.nodesCount, false) {}

void DFS::runRed() {
    this->searchRed(source);
    if (this->visited[relevantArc.n]) {
        relevantArc.redArc = true;
        this->G.edges[relevantArc.idInEdgesVector].isRed = true;
    }
}

void DFS::searchRed(const Node startNode) {
    EdgeId relevantArcId = this->relevantArc.idInEdgesVector;
    visited[startNode] = true;
    if (startNode != relevantArc.n) {
        const Neighborhood& arcs{G.adjacentArcs(startNode)};
        for (const Arc& a : arcs) {
            const Edge& edge = this->G.edges[a.idInEdgesVector];
            if (edge.isRed || a.idInEdgesVector == relevantArcId || visited[a.n]) {
                continue;
            }
//            if (a.c[0] <= relevantArc.c[0] && a.c[1] <= relevantArc.c[1]) {
            if (dominates(a.c, relevantArc.c)) {
                searchRed(a.n);
            }
        }
    }
}

void DFS::runBlue() {
    this->searchBlue(source);
    if (!this->visited[relevantArc.n]) {
        relevantArc.blueArc = true;
        this->G.edges[relevantArc.idInEdgesVector].isBlue = true;
    }
}

void DFS::searchBlue(const Node startNode) {
    EdgeId relevantArcId = this->relevantArc.idInEdgesVector;
    visited[startNode] = true;
    if (startNode != relevantArc.n) {
        const Neighborhood& arcs{G.adjacentArcs(startNode)};
        for (const Arc& a : arcs) {
            if (a.idInEdgesVector == relevantArcId || visited[a.n]) {
                continue;
            }
//            if (!(relevantArc.c[0] <= a.c[0] && relevantArc.c[1] <= a.c[1]) || a.blueArc) {
            if (!dominates(relevantArc.c, a.c) || a.blueArc) {
                searchBlue(a.n);
            }
        }
    }
}

size_t findRedArcs(Graph& G) {
    size_t redArcs{0};
    vector<bool> processed(G.arcsCount, false);
    for (Node u = 0; u < G.nodesCount; u++) {
        Neighborhood& arcs = G.adjacentArcs(u);
        for (Arc& a : arcs) {
            //assert(a.n == u);
            if (processed[a.idInEdgesVector]) {
                continue;
            }
            processed[a.idInEdgesVector] = true;
            DFS dfs(G, u, a);
            dfs.runRed();
            if (a.redArc) {
                ++redArcs;
            }
        }
    }
    size_t redArcsOppositeDirection = 0;
    for (Node u = 0; u < G.nodesCount; u++) {
        Neighborhood& arcs = G.adjacentArcs(u);
        for (Arc& a : arcs) {
            const Edge& e = G.edges[a.idInEdgesVector];
            if (e.isRed && !a.redArc) {
                a.redArc = true;
                ++redArcsOppositeDirection;
            }
        }
    }
    assert (redArcs == redArcsOppositeDirection);
    return redArcs;
}

size_t findBlueArcs(Graph& G) {
    size_t blueArcs{0};
    vector<bool> processed(G.arcsCount, false);
    for (Node u = 0; u < G.nodesCount; u++) {
        Neighborhood& arcs = G.adjacentArcs(u);
        for (Arc& a : arcs) {
            //assert(a.n == u);
            if (processed[a.idInEdgesVector]) {
                continue;
            }
            processed[a.idInEdgesVector] = true;
            DFS dfs(G, u, a);
            dfs.runBlue();
            if (a.blueArc) {
                ++blueArcs;
            }
        }
    }
    return blueArcs;
}
