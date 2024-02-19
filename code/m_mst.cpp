#define BN_ALGO
#define IG_MDA

//#define PRINT_ALL_TREES

#include <chrono>
#include <ctime>
#include <iomanip>

#include "datastructures/includes/graph.h"
#include "datastructures/includes/GraphCompacter.h"
#include "preprocessing/includes/Preprocessor.h"
#include "search/includes/MultiPrim.h"
#include "search/includes/BN.h"

#include "search/includes/Solution.h"

//#include "valgrind/callgrind.h"
#include <boost/asio/ip/host_name.hpp>

int main(int argc, char *argv[]) {
    (void) argc;

    const auto host_name = boost::asio::ip::host_name();

    std::stringstream name;
    std::vector<std::string> splittedName = split(argv[1], '/');
//    name << splittedName[splittedName.size()-5] << ";" << splittedName[splittedName.size()-4];
//    name << splittedName.end()[-5];
//    name <<  ";";
//    name << splittedName.end()[-4];
//    name <<  ";";
    name << splittedName.back();
    const std::string graphName{name.str()};

    FILE* logCollectionFile;
    logCollectionFile = fopen("logs.txt", "a");

#ifdef IG_MDA
    {
        EdgeSorter edgeComparator{standardSorting()};
        std::unique_ptr<Graph> G_ptr = setupGraph(argv[1], edgeComparator);
        Graph& G = *G_ptr;
        Preprocessor preprocessor;
        GraphCompacter contractedGraph = preprocessor.run(G);
        IGMDA biSearch(contractedGraph.compactGraph);
        std::clock_t c_start = std::clock();
        Solution solution = biSearch.run();
        std::clock_t c_end = std::clock();
        std::time_t end_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
#ifdef PRINT_ALL_TREES
        solution.printSpanningTrees(contractedGraph);
#endif
        char resultsBuffer[350];
        sprintf(resultsBuffer, "IG-MDA;%uDIM;%s;%d;%d;%lu;%lu;%lf;%lf;%lf;%lu;%lu;%lu;%lu;%lu;%lu;%s;%s\n",
                DIM, graphName.c_str(), G.nodesCount, G.arcsCount,
                contractedGraph.blueArcs, contractedGraph.redArcs,
                preprocessor.duration, solution.time, (1000 * (c_end - c_start) / CLOCKS_PER_SEC) / 1000.,
                solution.trees, solution.extractions, solution.insertions, solution.nqtIt, solution.transitionNodes,
                solution.transitionArcs,
                host_name.c_str(), std::ctime(&end_time));
        std::cout << resultsBuffer << std::endl;
        fprintf(logCollectionFile, "%s", resultsBuffer);
    };
#endif


#ifdef BN_ALGO
    EdgeSorterBN sorter;
    std::unique_ptr<Graph> G_ptr = setupGraph(argv[1], sorter);
    Graph& G = *G_ptr;
    Preprocessor preprocessor;
    GraphCompacter contractedGraph = preprocessor.run(G);
    BN::ArcSorter arcSorter;
    sortArcs(contractedGraph.compactGraph, arcSorter);

    BN::MultiobjectiveSearch bnSearch(contractedGraph.compactGraph);
    std::clock_t c_start_bn = std::clock();
    Solution bnSolution = bnSearch.run();
    std::clock_t c_end_bn = std::clock();
#ifdef PRINT_ALL_TREES
    bnSolution.printSpanningTrees(contractedGraph);
#endif
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
