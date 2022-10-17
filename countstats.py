#!/usr/bin/python3

# countstats.py: aggregate instance data by size and spine count.
# The script reads from "stats.csv" and aggregates to "aggregate.csv".
# It requires that stats data rows are grouped by instance ID,
# and that all algorithms were applied to all instances.
import csv
import os
from collections import namedtuple

# aggregate record type
Record = namedtuple('Record', [
    'yescnt', # count of yes-instances
    'nocnt', # count of no-instances
    'dfsyes', # count of instances solved by heuristic with dfs
    'bfsyes', # count of instances solved by heuristic with bfs
    'dpyestime', # cumulative run time of dynamic program on yes-instances
    'dfsyestime', # cumulative run time of heuristic with dfs on yes-instances
    'bfsyestime', # cumulative run time of heuristic with bfs on yes-instances
    'dpnotime', # cumulative run time of dynamic program on no-instances
    'dfsnotime', # cumulative run time of heuristic with dfs on no-instances
    'bfsnotime']) # cumulative run time of heuristic with bfs on no-instances

final = {} # aaggregate data indexed by size, then by number of spines.

# spine-aggregate record type
SpineRecord = namedtuple('SpineRecord', [
    'yescnt', # count of yes-instances
    'dfsyes', # count of instances solved by heuristic with dfs
    'bfsyes']) # count of instances solved by heuristic with bfs

# Aggregate data by number of spines
byspine = {}

last_instance="" # current instance being collected from multiple input rows
dpyes = 0 # whether current instance was solved by reliable algorithm
dfsyes = 0 # whether current instance was solved by heuristic with dfs
bfsyes = 0 # whether current instance was solved by heuristic with bfs
dptime = 0 # run time of dynamic program on current instance
dfstime = 0 # run time of heuristic with dfs on current instance
bfstime = 0 # run time of heuristic with bfs on current instance

def addrecord(size, spines):
    global dpyes
    global dfsyes
    global bfsyes
    global dptime
    global dfstime
    global bfstime

    if size not in final:
        final[size] = {}

    if spines not in final[size]:
        final[size][spines] = Record(0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
        
    if spines not in byspine:
        byspine[spines] = SpineRecord(0, 0, 0)

    record = final[size][spines]
    spinerecord = byspine[spines]
    assert dptime > 0
    assert dfstime > 0
    assert bfstime > 0

    if dpyes > 0:
        record = record._replace(yescnt=record.yescnt + 1, dpyestime=record.dpyestime + dptime)
        spinerecord = spinerecord._replace(yescnt=spinerecord.yescnt + 1)
    else:
        record = record._replace(nocnt=record.nocnt + 1, dpnotime=record.dpnotime + dptime)

    if dfsyes > 0:
        record = record._replace(dfsyes=record.dfsyes + 1, dfsyestime=record.dfsyestime + dfstime)
        spinerecord = spinerecord._replace(dfsyes=spinerecord.dfsyes + 1)
    else:
        record = record._replace(dfsnotime=record.dfsnotime + dfstime)

    if bfsyes > 0:
        record = record._replace(bfsyes=record.bfsyes + 1, bfsyestime=record.bfsyestime + bfstime)
        spinerecord = spinerecord._replace(bfsyes=spinerecord.bfsyes + 1)
    else:
        record = record._replace(bfsnotime=record.bfsnotime + bfstime)
                
    final[size][spines] = record
    byspine[spines] = spinerecord
    dpyes = 0
    dfsyes = 0
    bfsyes = 0
    dptime = 0
    dfstime = 0
    bfstime = 0

print("Reading...")

with open('stats.csv', 'r') as statsfile:
    reader = csv.reader(statsfile)
    size = 0
    spines = 0

    for row in reader:
        algorithm = row[1]
        if "Algorithm" == algorithm:
            continue # skip header row
        instance = row[0]
        if instance != last_instance and last_instance != "": # finished collecting a data point to aggregate into a Record
            addrecord(size, spines)
        order = row[2]
        size = int(row[3])
        spines = int(row[4])
        success = int(row[5])
        duration = int(row[6])

        last_instance = instance
        if "cleve" == algorithm:
            if "depth-first" == order:
                dfsyes = success
                dfstime = duration
            elif "breadth-first" == order:
                bfsyes = success
                bfstime = duration
            else:
                assert False, "unknown order: " + order
        elif "dynamic-program" == algorithm:
            assert "depth-first" == order
            dpyes = success
            dptime = duration
        else:
            assert False, "unknown algorithm: " + algorithm

    if last_instance != "": # save last data point
        addrecord(size, spines)

    statsfile.close()
    
print("Writing Basic Aggregate...")

with open('aggregate.csv', 'w') as aggfile:
    writer = csv.writer(aggfile)
    fields = ['size', 'spines', 'yescnt', 'nocnt', 'dfsyes', 'bfsyes', 'dpyestime', 'dfsyestime', 'bfsyestime', 'dpnotime', 'dfsnotime', 'bfsnotime']
    writer.writerow(fields)

    for size in final:
        for spines, record in final[size].items():
            fields = [size, spines, record.yescnt, record.nocnt, record.dfsyes, record.bfsyes,
                record.dpyestime, record.dfsyestime, record.bfsyestime, record.dpnotime, record.dfsnotime, record.bfsnotime]
            writer.writerow(fields)

    aggfile.close()
    
print("Writing Runtime(DP) plot data...")

with open('runtime_dp.dat', 'w') as aggfile:
    fields = ['size', 'dptime', 'clsspines']
    aggfile.write(" ".join(fields) + os.linesep)

    for size in final:
        for spines, record in final[size].items():
            if record.yescnt > 0:
                fields = [size, record.dpyestime/record.yescnt, "yes" + str(spines)]
                aggfile.write(" ".join([str(f) for f in fields]) + os.linesep)

            if record.nocnt > 0:
                fields = [size, record.dpnotime/record.nocnt, "no" + str(spines)]
                aggfile.write(" ".join([str(f) for f in fields]) + os.linesep)

    aggfile.close()


print("Writing Runtime(DP, yes) plot data...")

with open('runtime_dpyes.dat', 'w') as aggfile:
    fields = ['size', 'dptime', 'clsspines']
    aggfile.write(" ".join(fields) + os.linesep)

    for size in final:
        for spines, record in final[size].items():
            if record.yescnt > 0:
                fields = [size, record.dpyestime/record.yescnt, "yes" + str(spines)]
                aggfile.write(" ".join([str(f) for f in fields]) + os.linesep)

    aggfile.close()

print("Writing Runtime(DFS) plot data...")

with open('runtime_dfs.dat', 'w') as aggfile:
    fields = ['size', 'dfstime', 'clsspines']
    aggfile.write(" ".join(fields) + os.linesep)

    for size in final:
        for spines, record in final[size].items():
            if record.dfsyes > 0:
                fields = [size, record.dfsyestime/record.dfsyes, "yes" + str(spines)]
                aggfile.write(" ".join([str(f) for f in fields]) + os.linesep)

            dfsno = record.yescnt + record.nocnt - record.dfsyes
            if dfsno > 0:
                fields = [size, record.dfsnotime/dfsno, "no" + str(spines)]
                aggfile.write(" ".join([str(f) for f in fields]) + os.linesep)

    aggfile.close()

print("Writing Runtime(BFS) plot data...")

with open('runtime_bfs.dat', 'w') as aggfile:
    fields = ['size', 'bfstime', 'clsspines']
    aggfile.write(" ".join(fields) + os.linesep)

    for size in final:
        for spines, record in final[size].items():
            if record.bfsyes > 0:
                fields = [size, record.bfsyestime/record.bfsyes, "yes" + str(spines)]
                aggfile.write(" ".join([str(f) for f in fields]) + os.linesep)

            bfsno = record.yescnt + record.nocnt - record.bfsyes
            if bfsno > 0:
                fields = [size, record.bfsnotime/bfsno, "no" + str(spines)]
                aggfile.write(" ".join([str(f) for f in fields]) + os.linesep)

    aggfile.close()
    
print("Writing Accuracy plot data...")

with open('accuracy.dat', 'w') as aggfile:
    fields = ['spines', 'p_solved', 'order', 'yescnt']
    aggfile.write(" ".join(fields) + os.linesep)

    for spines, record in byspine.items():
        yescnt = record.yescnt > 0 and record.yescnt or 1
        fields = [spines, record.dfsyes/yescnt, "dfs", record.yescnt]
        aggfile.write(" ".join([str(f) for f in fields]) + os.linesep)
        fields = [spines, record.bfsyes/yescnt, "bfs", record.yescnt]
        aggfile.write(" ".join([str(f) for f in fields]) + os.linesep)

    aggfile.close()

print("Finished.")
