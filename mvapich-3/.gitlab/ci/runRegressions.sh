#!/bin/bash

REPORT_FAILURES=false
THRESHOLD=30
THRESHOLDMRI=120
THRESHOLDRI2=120

pushd $PWD

echo "Running on"
hostname

CLUSTER=$(hostname)
if [[ $CLUSTER == "head.ri2.cse.ohio-state.edu" ]]; then
 echo "cluster is ri2"
 CLUSTER_ABBREV="ri2"
 declare -a channels=("mrail")
elif [[ $CLUSTER == "mri.cluster" ]]; then
 echo "cluster is mri"
 CLUSTER_ABBREV="mri"
 declare -a channels=("ucx")
else
 echo "cluster is nowlab"
 CLUSTER_ABBREV="nowlab"
 declare -a channels=("ofi")
fi

cd /home/gitlab-runner/merges/tuning-suite/regression

FILE=settings.ini

if [[ -f "$FILE" ]]; then
  rm settings.ini
fi

if [[ $CI_MERGE_REQUEST_TARGET_BRANCH_NAME == "master" ]] || [[ $PIPELINE_BRANCH_TYPE == "master" ]]; then
  BUILDER_SUFFIX="master"
elif [[ $CI_MERGE_REQUEST_TARGET_BRANCH_NAME == "master-x" ]] || [[ $PIPELINE_BRANCH_TYPE == "master-x" ]]; then
  BUILDER_SUFFIX="x"
else
  BUILDER_SUFFIX="gdr"
fi

#Clean up old slurm files, verification files and reports

if test -n "$(find . -maxdepth 1 -name 'slurm-*' -print -quit)"
then
    echo "Removing old slurm files"
    rm slurm-*
fi
if test -n "$(find . -maxdepth 1 -name '1-nodes*' -print -quit)"
then
    echo "Removing old 1-node verification files"
    rm 1-nodes*
fi
if test -n "$(find . -maxdepth 1 -name '2-nodes*' -print -quit)"
then
    echo "Removing old 2-node verification files"
    rm 2-nodes*
fi
if test -n "$(find . -maxdepth 1 -name 'pt2pt-*' -print -quit)"
then
    echo "Removing old slurm files"
    rm pt2pt-*
fi
if test -n "$(find . -maxdepth 1 -name '*.html' -print -quit)"
then
    echo "Removing old regression reports"
    rm *.html
fi

for channel in "${channels[@]}"
do

 rm -f ./settings.ini
 cp ./templates/settings${CLUSTER_ABBREV}.ini ./settings.ini

 #OLD_CI_COMMIT_SHA=`echo "$1" | sed -E -n 's|^Previous:(.*)|\1|p' /home/runbot/mvapich2/merges/$BRANCH/LATEST_$BRANCH/prevbuilds`

 #Edit Necessary lines in settings.ini file
 sed -i 's$Arch: old$Arch: '$CLUSTER_ABBREV'-'$channel'-Merge-to-'$CI_COMMIT_SHA'$' ./settings.ini

 sed -i 's$Path: /old$Path: /home/runbot/mvapich2/merges/'$BRANCH'/REG_'$CLUSTER_ABBREV'_'$channel'_'$BUILDER_SUFFIX'/'$CI_COMMIT_SHA'/gcc$' ./settings.ini

 sed -i 's$Path_Old: /old$Path_Old: /home/runbot/mvapich2/merges/'$BRANCH'/LATEST_'$CLUSTER_ABBREV'_'$channel'_'$BUILDER_SUFFIX'/'$CI_COMMIT_SHA'/gcc$' ./settings.ini

 sed -i 's$Commit: /old$Commit: '$CI_COMMIT_SHA'$' ./settings.ini

 sed -i 's$Commit_Old: /old$Commit_Old: LATEST_'$BUILDER_SUFFIX'$' ./settings.ini

 sed -i 's$outfile: report.html$outfile: report-'$CLUSTER_ABBREV'-'$channel'-'$CI_COMMIT_SHA'.html$' ./settings.ini

 sed -i 's$title: old$title: '$CLUSTER_ABBREV'-'$channel'-'$BRANCH' '$CI_COMMIT_SHA' vs Old: LATEST_'$BUILDER_SUFFIX'$' ./settings.ini


 #Use standard submit-all script to batch all intra/internode and 1/2 node jobs
 #Passes in commit-sha for detection of the completion files
 bash ./submit-all.sh $CI_COMMIT_SHA $channel

 bash ./verify-all.sh $CI_COMMIT_SHA $channel

 source $HOME/miniconda3/bin/activate
 conda activate tuning-suite

 python main.py --report pt2pt-intra-intrasock pt2pt-intra-intersock pt2pt-inter 1-nodes 2-nodes

 #copy artifacts to proper location
 HTML_REPORT=/home/gitlab-runner/merges/tuning-suite/regression/report-${CLUSTER_ABBREV}-${channel}-${CI_COMMIT_SHA}.html
 cp $HTML_REPORT $CI_PROJECT_DIR

 cp /home/gitlab-runner/merges/tuning-suite/regression/slurm-* $CI_PROJECT_DIR

 #exit tuning-suite venv
 conda deactivate
 #exit base venv
 conda deactivate

 #check the HTML report for errors, custom exit code will signal without having to check the logs
 if grep -q "version failed to run" $HTML_REPORT; then
        echo "There may have been an error running the regressions, please check the following report in this job's artifacts:"
        echo "$HTML_REPORT"
        REPORT_FAILURES=true
        # If new version, fail pipeline
        if grep -q "The new version failed to run" $HTML_REPORT; then
            echo "One or more benchmarks of the new version failed to run"
            exit 1
        fi
 fi

 for i in $(ls | egrep -i 'slurm-*' ); do
  if grep -q "error" $i; then
         echo "There may have been an error in the following benchmark:"
         echo "/home/gitlab-runner/merges/tuning-suite/regression/${i}"
         REPORT_FAILURES=true
  fi
 done

 if [ "$REPORT_FAILURES" = true ]; then
        echo "Script succeeded, but errors may be present in regression files, please check above logs"
 fi

 rm -f /home/gitlab-runner/merges/tuning-suite/regression/negatives.txt

 grep -o '\-[0-9]*.[0-9]*%' $HTML_REPORT | grep -o '\-[0-9]*' > /home/gitlab-runner/merges/tuning-suite/regression/negatives.txt

 #Check number of degredations at or above 10%
 NUMBAD=$(awk '{ if($0 < -9) print $0;}' /home/gitlab-runner/merges/tuning-suite/regression/negatives.txt | wc -l)
 echo "Number of degredations over 10% for $channel on $CLUSTER_ABBREV: "
 echo $NUMBAD
 if [ "$NUMBAD" -gt "$THRESHOLD" ]; then
   echo "This is more than the allowed threshold of $THRESHOLD for $channel on $CLUSTER_ABBREV";
   rm -f /home/gitlab-runner/merges/tuning-suite/regression/negatives.txt
   if [ $CLUSTER_ABBREV == "mri" ]; then
     if [ "$NUMBAD" -gt "$THRESHOLDMRI" ]; then
       exit 1
     else
       echo "but within tolerance to account for system performance variance"
       exit 30
     fi
   fi
   if [ $CLUSTER_ABBREV == "ri2" ]; then
     if [ "$NUMBAD" -gt "$THRESHOLDRI2" ]; then
       exit 1
     else
       echo "but within tolerance to account for system performance variance"
       exit 30
     fi
   fi
   exit 1
 else
   echo "This is within the allowed limit of $THRESHOLD for $channel on $CLUSTER_ABBREV";
   rm -f /home/gitlab-runner/merges/tuning-suite/regression/negatives.txt
 fi

done

#return to original directory
popd

