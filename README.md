# mo-mst
This repository contains code, computational results, and conference slides related to a new dynamic programming algorithm for the Multiobjective Minimum Spanning Tree (MO-MST) problem. The algorithm, called the Implicit Graph Multiobjective Dijkstra Algorithm (IG-MDA), was developed at the Zuse Institute Berlin and the Universidad de La Laguna. The authors are Pedro Maristany de las Casas, Antonio Sedeño Noda, and Ralf Borndörfer. The repository also contains our implementation of the Built Network (BN) algorithm. This is also a dynamic programming algorithm for the MO-MST problem. It was first published in https://doi.org/10.1016/j.cor.2018.05.007 By default the exectuables generated in this repository run both the IG-MDA and the BN algorithm on the input MO-MST instance. In case you have any questions, contact Pedro Maristany de las Casas -- maristany [a t] zib.de .

# Referencing/Citing the Code

To cite this code, please always use the permanent bibliographic information specified here: ...

## Disclaimer

We have tested the code and this manual on UNIX based systems. Some basic support can be provided if users encounter problems on these platforms. Users on Windows based systems will have to figure out how to compile and run these algorithms on their own.

## Generate executables

This is a CMake project, the source code is written in C++ and uses some C++17 features. Moreover, the dynamic bitsets from the Boost library are used. If not installed, start installing the newest CMake and Boost version available for your system. Once installed, use the terminal to navigate either to the 'code' folder in the project. Now create a new folder called 'build' (name can be chosen arbitrarily) and navigate into the folder. This step is not mandatory but it helps to keep the root folders clean; CMake users consider it a best practice. From the new 'build' folder, call cmake .. -DCMAKE_BUILD_TYPE=Release to generate the Makefile for the project. In case this step succeeds, you should now have a 'Makefile' in the 'build' folder. Call make. This will generate an executable called 'BN_AND_IGMDA_Release.o' in the 'build' folder. By default the code is configured to run 3 dimensional MO-MST instances. In case you want to solve instances with other edge cost dimensions, go open the file /code/datastructures/includes/typedefs.h and in Line 30, change the problem's dimension accordingly. After doing so, recompile the code to get a new executable.

##Running an example

We assume that you have compiled the code as explained before for 3 dimensional problems. From the 'build' folder described in the last section, call ./BN_AND_IGMDA_Release.o ../exampleInstances/3_a_9_90_2.tree . Here, the first argument is the name of the executable, the second argument is a (relative) path to a graph with 3-dimensional edge costs. If everything works well, you should see an output like this:

T-MDA;NY.gr;0;10100;2297421;2297421;6055;1.4252;25.76;3089
NAMOA;NY.gr;0;10100;2297407;2297407;6055;5.0103;5.91;3089
NAMOA_LAZY;NY.gr;0;10100;2559950;2870672;6055;1.4661;7.80;160658

Each line's entries are: algo-name, graph-name, sourceId, targetId, number of extracions, number of iterations, number of efficient paths at target, time to solve, memory,max number of paths in prio. queue.
Graph files

The first line of a graph file has to have the following self explanatory format (compare to the file 'aStarExample.gr' in the folder 'exampleGraphs')

p sp number_of_nodes number_of_arcs

The line should be followed by number_of_arcs many lines, each of them specifying an arc information as follows for the 3-dimensional case

a tailId headId c1 c2 c3

The ids of the nodes are assumed to be numbered consecutively from 0 to number_of_nodes-1. In case you use a 2-dimensional instance for the multi-dimensional code (currently only enabled for 3 dimensions), the program internally creates a third cost dimension with costs 1 for every arc in the graph.
