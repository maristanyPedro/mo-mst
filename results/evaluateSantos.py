from pathlib import Path
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy.stats.mstats import gmean

plt.rcParams['text.usetex'] = True

TIME_LIMIT = 7200


# Only the instances solved matching the conditions defined in this function are regarded as solved!
def define_solvability(solTime):
    if pd.isnull(solTime) or solTime > TIME_LIMIT:
        return False
    return True


def geoMeanWithoutNans(df, columnName):
    # return gmean(df[~df[columnName].isnull()][columnName])
    return df[~df[columnName].isnull()][columnName].mean()


def latexLineSantos(nodes, df):
    line = "{} & ".format(nodes)
    line += "{} & ".format(len(df))
    line += "{} & ".format(gmean(df['SOLUTIONS_PRIM']).round(2))
    for algo in ["BN", "PRIM"]:
        #line += "{} & ".format(len(df[~df['SOLUTIONS_{}'.format(algo)].isna()]))
        line += "{:.2f} & ".format(gmean(df['TRANSITION_NODES_COUNT_{}'.format(algo)]))
        line += "{:.2f} & ".format(gmean(df['EXTRACTIONS_{}'.format(algo)]))
        line += "{:.4f} & ".format(gmean(df['WALL_TIME_{}'.format(algo)]))

    line += "{:.2f} \\\\".format(gmean(df['SPEEDUP']))
    return line
    # print("Number of instances: {}".format(len(df)))
    # print("Average number of solutions: {}".format(gmean(df['SOLUTIONS_PRIM']).round(2)))
    #
    # print("Solved by PRIM {}".format(len(df[~df['SOLUTIONS_PRIM'].isna()])))
    #
    # print("Nodes in transition graph PRIM {}".format(gmean(df['TRANSITION_NODES_COUNT_PRIM']).round(2)))
    #
    # print("Extraction PRIM {}".format(gmean(df['EXTRACTIONS_PRIM']).round(2)))
    #
    # print("TIME PRIM {}".format(gmean(df['WALL_TIME_PRIM']).round(2)))
    # print("SPEEDUP {}".format(gmean(df['SPEEDUP']).round(2)))
####################################################################

inst_columns = ["INSTANCE"]
results_columns = ["ALGO", "DIMENSION", "INST_TYPE", "DUMMY", "PATH", "INSTANCE", "NODES", "EDGES", "BLUE_EDGES",
                   "RED_EDGES", "PREP_TIME", "WALL_TIME", "CPU_TIME", "SOLUTIONS", "EXTRACTIONS", "INSERTIONS",
                   "NQP_IT", "TRANSITION_NODES_COUNT", "TRANSITION_ARCS_COUNT", "HOST", "DATE"]
print(len(results_columns))
instances = "santos_4d.inst"
multiPrimResults = "multiPrim_SANTOS_4d.csv"
multiBNResults = "multiBN_SANTOS_4d.csv"
plot_names = "SCPACYC-4d"

#filename = solutionsFileName.split('.')[0]
instances_df = pd.read_csv(instances, sep=';', header=None, names=inst_columns)
prim_df = pd.read_csv(multiPrimResults, sep=';', header=None, names=results_columns)
bn_df = pd.read_csv(multiBNResults, sep=';', header=None, names=results_columns)

master_df = pd.merge(left=instances_df, right=prim_df, how='left', on="INSTANCE")
master_df = master_df.merge(right=bn_df, how='left', on="INSTANCE", suffixes=("_PRIM", "_BN"))

master_df["SPEEDUP"] = master_df["WALL_TIME_BN"]/master_df["WALL_TIME_PRIM"]
master_df["EQUIVALENT"] = master_df["SOLUTIONS_BN"] == master_df["SOLUTIONS_PRIM"]
print(master_df.to_string())

c1 = '#FFC857'
c2 = '#255F85'

BN_averages = ""
ITS_PER_SECOND_BN = []
ITS_PER_SECOND_PRIM = []
for algo in ["BN", "PRIM"]:
    master_df["ITS_PER_SECOND_{}".format(algo)] = master_df["EXTRACTIONS_{}".format(algo)]/master_df["WALL_TIME_{}".format(algo)]

for nodesCount, resultsByNodes in master_df.groupby(by="NODES_PRIM"):
    ITS_PER_SECOND_BN.append(gmean(resultsByNodes["ITS_PER_SECOND_BN"]))
    ITS_PER_SECOND_PRIM.append(gmean(resultsByNodes["ITS_PER_SECOND_PRIM"]))
    #print("Nodes: {} Speedup: {}\n".format(nodesCount, resultsByNodes["SPEEDUP"].mean()))
    #print("Nodes: {} Gmean Speedup: {}\n".format(nodesCount, gmean(resultsByNodes["SPEEDUP"])))
    santos_line = latexLineSantos(nodesCount, resultsByNodes)
    print(santos_line)
    BN_averages += "& {} ".format(resultsByNodes['WALL_TIME_BN'].mean())
    ax1 = resultsByNodes.plot(kind='scatter', x="SOLUTIONS_PRIM", y='WALL_TIME_PRIM',
                        label='IGMDA', color=c1)
    ax2 = resultsByNodes.plot(kind='scatter', x="SOLUTIONS_BN", y='WALL_TIME_BN',
                        label='BN', color=c2, ax=ax1)
    ax1.yaxis.grid(True)
    ax1.set_yscale('log')
    ax1.set_xscale('log')
    ax1.set_xlabel("Efficient trees")
    ax1.set_ylabel("Duration [s]")
    # ax1.set_yscale('log')
    ax1.set_title("{} MO-MST instances with {} nodes".format(plot_names, int(nodesCount)))

    #plt.show()
    plt.savefig("{}-{}.pdf".format(plot_names, int(nodesCount)), format="pdf")

print(BN_averages)
print(ITS_PER_SECOND_BN)
print(ITS_PER_SECOND_PRIM)
# solutions_df["GRAPH"] = solutions_df["GRAPH"].apply(lambda x: Path(x).stem)
# solutions_df["NET-GROUP"] = solutions_df["GRAPH"].apply(lambda x: netMakerParseGraphName(x))

# algos = [a for a, _ in solutions_df.groupby(by="ALGO", sort=False)]
# for i in range(len(algos)):
#     print("{}: {}".format(i, algos[i]))
# algoIndex = int(
#     input("What's the index of the algorithm you want to use as the reference algorithm when calculating speedups? "))
# referenceAlgo = algos[algoIndex]
#
# print("Reference algorithm set to: {}.".format(referenceAlgo))
#
# # for roadGraphName, results_df in solutions_df.groupby(by="GRAPH"):
# for nodesCount, results_df in solutions_df.groupby(by="NODES"):
#     print("There are {} instances with {} nodes!\n".format(len(results_df), nodesCount))
#     print(results_df)
# exit()
    # instances_df = []
    # for netName in results_df.groupby(by="GRAPH").groups.keys():
    #     instances_df.append(
    #         pd.read_csv("{}.inst".format(netName), sep=' ', header=None, names=["GRAPH", "SOURCE", "TARGET"]))
    # instance_df = pd.concat(instances_df, ignore_index=True)
    # # print(instance_df.to_string())
    # # instance_df = pd.read_csv("{}.inst".format(roadGraphName), sep=' ', header=None, names=["GRAPH", "SOURCE", "TARGET"])
    # # instance_df["GRAPH"] = instance_df["GRAPH"].apply(lambda x: Path(x).stem)
    #
    # c1 = '#FFC857'
    # c2 = '#255F85'
    #
    # total_df = mergeInstancesAndResults(instance_df, results_df)
    #
    # ax1 = total_df.plot(kind='scatter', x="AT_TARGET.T-MDA", y='TIME.T-MDA',
    #                     label='T-MDA', color=c1)
    # ax2 = total_df.plot(kind='scatter', x="AT_TARGET.NAMOA_LAZY", y='TIME.NAMOA_LAZY',
    #                     label='NAMO$A^*_{dr}$-lazy', color=c2, ax=ax1)
    # ax1.yaxis.grid(True)
    # ax1.set_yscale('log')
    # ax1.set_xscale('log')
    # ax1.set_xlabel("Efficient $s$-$t$-paths")
    # ax1.set_ylabel("Duration [s]")
    # # ax1.set_yscale('log')
    # ax1.set_title("One-to-One 3-dim. MOSP instances on {} networks".format(roadGraphName))
    #
    # plt.savefig("{}-{}.pdf".format(filename, roadGraphName), format="pdf")
    #
    # graphs = [g for g, _ in total_df.groupby(by="GRAPH")]
    # graphName = graphs[0]
    #
    # solved_instances_only = {}
    # speedup_by_instance = {}
    # for algo in algos:
    #     total_df.loc[total_df["SOLVABILITY.{}".format(algo)] == False, "TIME.{}".format(algo)] = TIME_LIMIT
    #     solved_instances_only[algo] = total_df.loc[total_df["SOLVABILITY.{}".format(algo)]]
    #     if algo == referenceAlgo:
    #         continue
    #     speedup_by_instance[algo] = total_df["TIME.{}".format(algo)] / total_df["TIME.{}".format(referenceAlgo)]
    #
    # slicesBounds = [0, 100, 1000, 5000, 10000, 1000000]
    # for i in range(len(slicesBounds) - 1):
    #     # Take only the instances that the reference algorithm solved within the specified time range.
    #     sliced_df = total_df.loc[
    #         (total_df['AT_TARGET.T-MDA'] > slicesBounds[i]) & (total_df['AT_TARGET.T-MDA'] <= slicesBounds[i + 1])]
    #     # print(sliced_df.to_string())
    #
    #     speedup_by_instance = {}
    #     toBePrinted = ""
    #     # if i == 0:
    #     #    toBePrinted += "\\multirow{{5}}{{*}}{{{}}} & ({}, {}] & {:.0f} & ".format(\
    #     #        roadGraphName, slicesBounds[i], slicesBounds[i+1], geoMeanWithoutNans(sliced_df, "AT_TARGET.T-MDA"))
    #     # else:
    #     #    toBePrinted += "& ({}, {}] & {:.0f} & ".format(\
    #     #        slicesBounds[i], slicesBounds[i+1], geoMeanWithoutNans(sliced_df, "AT_TARGET.T-MDA"))
    #
    #     # for algo in algos:
    #     #    toBePrinted += "{}/{} & {:.0f} & {:.2f} & ".format(\
    #     #    len(sliced_df.loc[total_df["SOLVABILITY.{}".format(algo)]==True]), len(sliced_df), \
    #     #        geoMeanWithoutNans(sliced_df, "EXTRACTED.{}".format(algo)),
    #     #        geoMeanWithoutNans(sliced_df, "TIME.{}".format(algo)))
    #     #    if algo == referenceAlgo:
    #     #        continue
    #     #    else:
    #     #        speedup_by_instance = sliced_df["TIME.{}".format(algo)]/sliced_df["TIME.{}".format(referenceAlgo)]
    #     #        toBePrinted += "{:.2f} \\\\".format(gmean(speedup_by_instance))
    #     # print(toBePrinted)
    #     lineStart = ""
    #     if i == 0:
    #         lineStart += "\\multirow{{5}}{{*}}{{{}}} &".format(roadGraphName)
    #     else:
    #         lineStart += "& "
    #     toBePrinted += lineStart + "({}, {}] & {:.0f} & {:.0f} & ".format( \
    #         slicesBounds[i], slicesBounds[i + 1], len(sliced_df), geoMeanWithoutNans(sliced_df, "AT_TARGET.T-MDA"))
    #
    #     for algo in algos:
    #         toBePrinted += "{:.0f} & {:.2f} & ".format( \
    #             geoMeanWithoutNans(sliced_df, "EXTRACTED.{}".format(algo)),
    #             geoMeanWithoutNans(sliced_df, "TIME.{}".format(algo)))
    #         if algo == referenceAlgo:
    #             continue
    #         else:
    #             speedup_by_instance = sliced_df["TIME.{}".format(algo)] / sliced_df["TIME.{}".format(referenceAlgo)]
    #             toBePrinted += "{:.2f} \\\\".format(speedup_by_instance.mean())
    #     print(toBePrinted)
    #
    # print("\\midrule")
    # # print("RESULT: \n")
