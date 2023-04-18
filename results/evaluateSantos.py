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

def mergeInstancesAndResults(instances, results):
    instances.sort_values(by=["SOURCE", "TARGET"], inplace=True, ignore_index=True)

    results_by_algorithms = results.groupby(by="ALGO")
    master_df = instances
    for algo, group_results in results_by_algorithms:
        group_results = group_results.reset_index(drop=True)
        group_results = group_results.add_suffix(".{}".format(algo))
        group_results = group_results.rename(
            columns={"SOURCE.{}".format(algo): "SOURCE", "TARGET.{}".format(algo): "TARGET",
                     "GRAPH.{}".format(algo): "GRAPH"})
        master_df = master_df.merge(right=group_results, how='left', on=['SOURCE', 'TARGET'])
        master_df["ALGO.{}".format(algo)] = algo
        # Generate Boolean column that indicates whether an instance was solved by the considered algorithm!
        master_df["SOLVABILITY.{}".format(algo)] = master_df.apply(
            lambda x: define_solvability(x["TIME.{}".format(algo)]), axis=1)
    return master_df


def geoMeanWithoutNans(df, columnName):
    # return gmean(df[~df[columnName].isnull()][columnName])
    return df[~df[columnName].isnull()][columnName].mean()


def latexLineSantos(nodes, df):
    line = "{} & ".format(nodes)
    line += "{} & ".format(len(df))
    line += "{} & ".format(gmean(df['SOLUTIONS_PRIM']).round(2))
    for algo in ["BN", "PRIM"]:
        #line += "{} & ".format(len(df[~df['SOLUTIONS_{}'.format(algo)].isna()]))
        line += "{} & ".format(gmean(df['TRANSITION_NODES_COUNT_{}'.format(algo)]).round(2))
        line += "{} & ".format(gmean(df['EXTRACTIONS_{}'.format(algo)]).round(2))
        line += "{} & ".format(gmean(df['WALL_TIME_{}'.format(algo)]).round(2))

    line += "{} \\\\".format(gmean(df['SPEEDUP']).round(2))
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
instances = "santos_3d.inst"
multiPrimResults = "multiPrim_SANTOS_3d.csv"
multiBNResults = "multiBN_SANTOS_3d.csv"
plot_names = "SCPACYC-3d"

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
for nodesCount, resultsByNodes in master_df.groupby(by="NODES_PRIM"):
    #print("Nodes: {} Speedup: {}\n".format(nodesCount, resultsByNodes["SPEEDUP"].mean()))
    #print("Nodes: {} Gmean Speedup: {}\n".format(nodesCount, gmean(resultsByNodes["SPEEDUP"])))
    santos_line = latexLineSantos(nodesCount, resultsByNodes)
    print(santos_line)
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

