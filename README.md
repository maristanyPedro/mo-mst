# mo-mst
This repository contains code, computational results, and conference slides related to a new dynamic programming algorithm for the Multiobjective Minimum Spanning Tree (MO-MST) problem. The algorithm, called the Implicit Graph Multiobjective Dijkstra Algorithm (IG-MDA), was developed at the Zuse Institute Berlin and the Universidad de La Laguna. The authors are Pedro Maristany de las Casas, Antonio Sedeño Noda, and Ralf Borndörfer. The repository also contains our implementation of the Built Network (BN) algorithm. This is also a dynamic programming algorithm for the MO-MST problem. It was first published in https://doi.org/10.1016/j.cor.2018.05.007 By default the exectuables generated in this repository run both the IG-MDA and the BN algorithm on the input MO-MST instance. In case you have any questions, contact Pedro Maristany de las Casas -- maristany [a t] zib.de .

### Referencing/Citing the Code

To cite this code, please always use the permanent bibliographic information specified here: ...

### Disclaimer

We have tested the code and this manual on UNIX based systems. Some basic support can be provided if users encounter problems on these platforms. Users on Windows based systems will have to figure out how to compile and run these algorithms on their own.

## Generate executables

This is a CMake project, the source code is written in C++ and uses some C++17 features. Moreover, the dynamic bitsets from the Boost library are used. If not installed, start installing the newest CMake and Boost version available for your system. Once installed, use the terminal to navigate either to the 'code' folder in the project. Now create a new folder called 'build' (name can be chosen arbitrarily) and navigate into the folder. This step is not mandatory but it helps to keep the root folders clean; CMake users consider it a best practice. From the new 'build' folder, call 
```
cmake .. -DCMAKE_BUILD_TYPE=Release 
```
to generate the Makefile for the project. In case this step succeeds, you should now have a 'Makefile' in the 'build' folder. Call 
```
make
```
This will generate an executable called 'BN_AND_IGMDA_Release.o' in the 'build' folder. By default the code is configured to run 3 dimensional MO-MST instances. In case you want to solve instances with other edge cost dimensions, go open the file /code/datastructures/includes/typedefs.h and in Line 30, change the problem's dimension accordingly. After doing so, recompile the code to get a new executable.

## Running an example

We assume that you have compiled the code as explained before for 3 dimensional problems. From the 'build' folder described in the last section, call

```
./BN_AND_IGMDA_Release.o ../exampleInstances/3_a_9_90_2.tree
```
Here, the first argument is the name of the executable, the second argument is a (relative) path to a graph with 3-dimensional edge costs. If everything works well, you should see an output like this:

```
MultiPrim;3DIM;SANTOS;;;3_a_9_90_2.tree;9;33;2;7;0.000025;0.000912;0.000000;272;1837;1771;0;64;514;opt-008549;Tue Apr 18 16:13:32 2023
MultiBN;3DIM;SANTOS;;;3_a_9_90_2.tree;9;33;2;7;0.000025;0.002310;0.002000;272;2079;0;0;64;0;opt-008549;Tue Apr 18 16:13:32 2023
```

Each line's entries are explained in the file code/m_mst.cpp. The three floats are the preprocessing time, the wall time, and the cpu time used by the corresponding algorithm. Right after these three floats the output lines indicate the cardinality of the solution sets. In this case 272 spanning trees were computed. If you want to see the actual solution trees, go to code/search/src/MultiPrim.cpp and uncomment Line 129. Then, recompile the project and run the new executable as before. The program will print all efficient trees calculated by the IG-MDA.

## Graph files

The first two lines of an instance file have to look as follows

```
mmst 9 33 3 1
e 0 1 57 70 33
```

The first line indicates that we are considering a mo-mst instance with a graph that has 9 nodes and 33 edges. The edge cost dimension is 3. Moreover, the file just considers 1 instance (this number is actually ignored).

The second line is the first line specifying an edge (e ...) in the input graph. In this case the edge connects node 0 with node 1 and the three dimensional edge costs are (57, 70, 33). In this example, the program expects further 32 edge lines to follow the line that we just explained. See the file in code/exampleInstances/ to see two complete instances taken from https://doi.org/10.1016/j.cor.2018.05.007

## Running evaluation scripts for results from ...

The results from the computational experiments presented in the paper are in the folder results/ .

### Santos et al. instances

For the MO-MST instances from https://doi.org/10.1016/j.cor.2018.05.007 please use the file evaluateSantos.py . In lines 73-75 you can choose the results that the script is supposed to consider. The lines are self-explanatory and the names of the result files in the result/ chosen accordingly. For example, in the current version of the evaluation script, the script will generate the contents for Table 3 in our paper. 


### Fernandes et al. instances

For the MO-MST instances from https://doi.org/10.1007/s10589-019-00154-1 please use the file evaluataIslamlifelipe.py . In lines 47-50 you can choose the results that the script is supposed to consider. The lines are self-explanatory and the names of the result files in the result/ chosen accordingly. For example, in the current version of the evaluation script, the script will generate the contents for Table 12 in our paper. 
