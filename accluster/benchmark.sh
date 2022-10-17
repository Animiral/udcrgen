#!/bin/bash

WDIR="/home1/e00725263/wudcrgen"
STATSFILE="stats.csv"
LOGFILE="benchmark.log"
TARFILE="benchmark.tar.gz"
OPTS="--algorithm benchmark --stats-file $STATSFILE --spine-min 2 --spine-max 5 --log-file $LOGFILE --log-level error"
JOBLOGFILE=$WDIR/bench1.log

echo "Start Lobster benchmark on $HOSTNAME..." >> $JOBLOGFILE

cd $TMPDIR
echo "Starting benchmark with options: $OPTS" >> $JOBLOGFILE
$WDIR/udcrgen $OPTS
echo "Completed benchmark. retcode=$?" >> $JOBLOGFILE

tar -czvf $TARFILE $STATSFILE $LOGFILE
echo "Completed archive $TARFILE. retcode=$?" >> $JOBLOGFILE

cp $TARFILE $WDIR/dump/$TARFILE
echo "Copied archive to $WDIR/dump/$TARFILE. retcode=$?" >> $JOBLOGFILE
echo >> $JOBLOGFILE

