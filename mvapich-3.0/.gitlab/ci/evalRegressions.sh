#!/bin/bash

THRESHOLD=30
THRESHOLDMRI=120
THRESHOLDRI2=120

echo "Running on "
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

for channel in "${channels[@]}"
do

  rm -f /home/gitlab-runner/merges/tuning-suite/regression/negatives.txt

  grep -o '\-[0-9]*.[0-9]*%' /home/gitlab-runner/merges/tuning-suite/regression/report-${CLUSTER_ABBREV}-${channel}-${CI_COMMIT_SHA}.html | grep -o '\-[0-9]*' > /home/gitlab-runner/merges/tuning-suite/regression/negatives.txt

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

