#define BN_ALGO
#define IG_MDA

#include <chrono>
#include <ctime>
#include <iomanip>

#include "datastructures/includes/graph.h"
#include "datastructures/includes/GraphCompacter.h"
#include "preprocessing/includes/Preprocessor.h"
#include "search/includes/MultiPrim.h"
#include "search/includes/BN.h"

#include "valgrind/callgrind.h"
#include <boost/asio/ip/host_name.hpp>

int main(int argc, char *argv[]) {
    (void) argc;
    std::unique_ptr<Graph> G_ptr = setupGraph(argv[1]);
    Graph& G = *G_ptr;
    const auto host_name = boost::asio::ip::host_name();

    std::stringstream name;
    std::vector<std::string> splittedName = split(argv[1], '/');
//    name << splittedName[splittedName.size()-5] << ";" << splittedName[splittedName.size()-4];
    name << "SANTOS;";
    name << splittedName.end()[-5];
    name <<  ";";
    name << splittedName.end()[-4];
    name <<  ";";
    name << splittedName.back();
    const std::string graphName{name.str()};

    FILE* logCollectionFile;
    logCollectionFile = fopen("logs.txt", "a");
    std::stringstream outputStream;

    Preprocessor preprocessor;
    GraphCompacter contractedGraph = preprocessor.run(G);

#ifdef IG_MDA
    NEW_GENERATION::IGMDA biSearch(contractedGraph.compactGraph);
    std::clock_t c_start = std::clock();
    Solution solution = biSearch.run();
    std::clock_t c_end = std::clock();
    std::time_t end_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char resultsBuffer [350];
    sprintf(resultsBuffer, "IG-MDA;%uDIM;%s;%d;%d;%lu;%lu;%lf;%lf;%lf;%lu;%lu;%lu;%lu;%lu;%lu;%s;%s\n",
            DIM, graphName.c_str(), G.nodesCount, G.arcsCount,
            contractedGraph.blueArcs, contractedGraph.redArcs,
            preprocessor.duration, solution.time, (1000* (c_end - c_start) / CLOCKS_PER_SEC)/1000.,
            solution.trees, solution.extractions, solution.insertions, solution.nqtIt, solution.transitionNodes, solution.transitionArcs,
            host_name.c_str(), std::ctime(&end_time));
    std::cout << resultsBuffer << std::endl;
    fprintf(logCollectionFile, "%s", resultsBuffer);
#endif


#ifdef BN_ALGO
    EdgeSorterBN sorter;
    std::sort(contractedGraph.compactGraph.arcs.begin(), contractedGraph.compactGraph.arcs.end(), sorter);
    ArcSorterBN arcSorter;
    for (Node n = 0; n < contractedGraph.compactGraph.nodesCount; ++n) {
        NodeAdjacency& neighborhood = contractedGraph.compactGraph.node(n);
        std::sort(neighborhood.adjacentArcs.begin(), neighborhood.adjacentArcs.end(), arcSorter);
    }

    BN::MultiobjectiveSearch bnSearch(contractedGraph.compactGraph);
    std::clock_t c_start_bn = std::clock();
    Solution bnSolution = bnSearch.run();
    std::clock_t c_end_bn = std::clock();
    //bnSearch.printParetoFront(*contractedGraph.connectedComponents);

    char bnResultsBuffer [350];
    std::time_t bn_end_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    sprintf(bnResultsBuffer, "MultiBN;%uDIM;%s;%d;%d;%lu;%lu;%lf;%lf;%lf;%lu;%lu;%lu;%lu;%lu;%lu;%s;%s",
            DIM, graphName.c_str(), G.nodesCount, G.arcsCount, contractedGraph.blueArcs, contractedGraph.redArcs,
            preprocessor.duration, bnSolution.time, (1000* (c_end_bn - c_start_bn) / CLOCKS_PER_SEC)/1000.,
            bnSolution.trees, bnSolution.extractions, bnSolution.insertions, bnSolution.nqtIt, bnSolution.transitionNodes, bnSolution.transitionArcs, host_name.c_str(),
            std::ctime(&bn_end_time));
    std::cout << bnResultsBuffer << std::endl;
    fprintf(logCollectionFile, "%s", bnResultsBuffer);
#endif


//    if (bnSolution.trees != solution.trees) {
//        printf("!!!!!!!!!!!!!!!!!!!!!Solutions do not coincide!!!!!!!!!!!!!!!!!!!!!\n");
//    }

    fclose(logCollectionFile);
    return (0);
}
