import math
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


def latexLineIslamifelipe(nodes, df, df_commonSolvability):
    line = "{} & ".format(nodes)
    line += "{} & ".format(len(df))
    for algo in ["BN", "PRIM"]:
        df_solved = df[~df['SOLUTIONS_{}'.format(algo)].isna()]
        line += "{} & ".format(len(df_solved))
        line += "{:.2f} & ".format(gmean(df_solved['SOLUTIONS_{}'.format(algo)]))
        line += "{:.2f} & ".format(gmean(df_solved['TRANSITION_NODES_COUNT_{}'.format(algo)]))
        line += "{:.2f} & ".format(gmean(df_solved['EXTRACTIONS_{}'.format(algo)]))
        line += "{:.4f} & ".format(gmean(df['WALL_TIME_{}'.format(algo)]))

    #line += "{} \\\\".format(gmean(df[~df['SPEEDUP'].isna()]['SPEEDUP']).round(2))
    line += "{:.2f} \\\\".format(gmean(df_commonSolvability[~df_commonSolvability['SPEEDUP'].isna()]['SPEEDUP']))
    return line

####################################################################

inst_columns = ["GRAPH_TYPE", "GROUP", "INSTANCE"]
results_columns = ["ALGO", "DIMENSION", "INST_TYPE", "GRAPH_TYPE", "GROUP", "INSTANCE", "NODES", "EDGES", "BLUE_EDGES",
                   "RED_EDGES", "PREP_TIME", "WALL_TIME", "CPU_TIME", "SOLUTIONS", "EXTRACTIONS", "INSERTIONS",
                   "NQP_IT", "TRANSITION_NODES_COUNT", "TRANSITION_ARCS_COUNT", "HOST", "DATE"]
print(len(results_columns))
instances = "4d_corr_grid.inst"
# instances = "grid_corr_3d.inst"
multiPrimResults = "multiPrim_grid_corr_4d.csv"
multiBNResults = "multiBN_grid_corr_4d.csv"
plot_names = "Grid-4d-Corr"


#filename = solutionsFileName.split('.')[0]
instances_df = pd.read_csv(instances, sep=';', header=None, names=inst_columns)
prim_df = pd.read_csv(multiPrimResults, sep=';', header=None, names=results_columns)
bn_df = pd.read_csv(multiBNResults, sep=';', header=None, names=results_columns)


#instances_df["NODES"] = instances_df.apply(lambda x: define_solvability(x["TIME.{}".format(algo)]), axis=1)
instances_df["NODES"] = instances_df.apply(lambda x: int(x["INSTANCE"].split('.')[0]), axis=1)
print(instances_df)
print(instances_df.to_string())

master_df = pd.merge(left=instances_df, right=prim_df, how='left', on=["GRAPH_TYPE", "GROUP", "INSTANCE", "NODES"])
master_df = master_df.merge(right=bn_df, how='left', on=["GRAPH_TYPE", "GROUP", "INSTANCE", "NODES"], suffixes=("_PRIM", "_BN"))

master_df["EQUIVALENT"] = master_df["SOLUTIONS_BN"] == master_df["SOLUTIONS_PRIM"]
print(master_df.to_string())
print(len(master_df[~master_df["SOLUTIONS_BN"].isna()]))

###Set unsolved instances to TIME_LIMIT
master_df["SOLVED_BN"] = ~master_df["SOLUTIONS_BN"].isna()
master_df["SOLVED_PRIM"] = ~master_df["SOLUTIONS_PRIM"].isna()
master_df.loc[master_df["SOLUTIONS_BN"].isna(), ["WALL_TIME_BN"]] = TIME_LIMIT
master_df.loc[master_df["SOLUTIONS_PRIM"].isna(), ["WALL_TIME_PRIM"]] = TIME_LIMIT

###Correct an error with the BN counter of extractions. There is always at least one extraction.
for algo in ["PRIM", "BN"]:
    for keyword in ["EXTRACTIONS", "TRANSITION_NODES_COUNT"]:
        columnName = "{}_{}".format(keyword, algo)
        master_df.loc[master_df[columnName] == 0, [columnName]] = 1


###In case we want running times to be in milliseconds.
#master_df['WALL_TIME_BN'] = master_df['WALL_TIME_BN'].multiply(0.000001)
#master_df['WALL_TIME_PRIM'] = master_df['WALL_TIME_PRIM'].multiply(0.000001)
master_df.loc[master_df["WALL_TIME_BN"] == 0, ["WALL_TIME_BN"]] = 0.000001
master_df.loc[master_df["WALL_TIME_PRIM"] == 0, ["WALL_TIME_PRIM"]] = 0.000001

###Compute Speedup and in case an instance's solution time is zero, consider both algorithms equally fast.
master_df["SPEEDUP"] = master_df["WALL_TIME_BN"]/master_df["WALL_TIME_PRIM"]
master_df.loc[master_df["SPEEDUP"] == math.inf, ["SPEEDUP"]] = 1
master_df.loc[master_df["SPEEDUP"] == 0, ["SPEEDUP"]] = 1
print(master_df.to_string())

c1 = '#FFC857'
c2 = '#255F85'


BN_averages = ""
for nodesCount, resultsByNodes in master_df.groupby(by="NODES"):
    #print("Nodes: {} Speedup: {}\n".format(nodesCount, resultsByNodes["SPEEDUP"].mean()))
    #print("Nodes: {} Gmean Speedup: {}\n".format(nodesCount, gmean(resultsByNodes["SPEEDUP"])))
    solvedByBoth = resultsByNodes[resultsByNodes["SOLVED_BN"] & resultsByNodes["SOLVED_PRIM"]]
    santos_line = latexLineIslamifelipe(nodesCount, resultsByNodes, solvedByBoth)
    print(santos_line)
    ax1 = resultsByNodes.plot(kind='scatter', x="SOLUTIONS_PRIM", y='WALL_TIME_PRIM',
                        label='IG-MDA', color=c1)
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
