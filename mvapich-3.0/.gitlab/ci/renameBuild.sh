#!/bin/bash

#TYPE will usually be LATEST_
TYPE=$1

echo "running on" 
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

if [[ $CI_MERGE_REQUEST_TARGET_BRANCH_NAME == "master" ]] || [[ $PIPELINE_BRANCH_TYPE == "master" ]]; then
  BUILDER_SUFFIX="master"
elif [[ $CI_MERGE_REQUEST_TARGET_BRANCH_NAME == "master-x" ]] || [[ $PIPELINE_BRANCH_TYPE == "master-x" ]]; then
  BUILDER_SUFFIX="x"
else
  BUILDER_SUFFIX="gdr"
fi

for channel in "${channels[@]}"
do

  #SHOULD BE OF THE FORM LATEST_cluster_channel_branch
  if [ -d "/home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/$CI_COMMIT_SHA" ]; then
    echo "Removing previous build copy of current commit"
    rm -rf /home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/$CI_COMMIT_SHA
  fi

  echo "Creating copy to /home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/$CI_COMMIT_SHA"
  mkdir /home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/$CI_COMMIT_SHA

  cp -r /home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/LATEST/* /home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/$CI_COMMIT_SHA/

  #Get the SHAS of the last 5 stable builds from prevbuilds file

  SHA_1=`echo "$1" | sed -E -n 's|^Current:(.*)|\1|p' /home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/prevbuilds`
  SHA_2=`echo "$1" | sed -E -n 's|^Previous:(.*)|\1|p' /home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/prevbuilds`
  SHA_3=`echo "$1" | sed -E -n 's|^3:(.*)|\1|p' /home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/prevbuilds`
  SHA_4=`echo "$1" | sed -E -n 's|^4:(.*)|\1|p' /home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/prevbuilds`
  SHA_5=`echo "$1" | sed -E -n 's|^5:(.*)|\1|p' /home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/prevbuilds`

  #Update list of latest 5 builds
  sed -E -i "s|^Current:(.*)|Current:$CI_COMMIT_SHA|" /home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/prevbuilds
  sed -E -i "s|^Previous:(.*)|Previous:$SHA_1|" /home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/prevbuilds
  sed -E -i "s|^3:(.*)|3:$SHA_2|" /home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/prevbuilds 
  sed -E -i "s|^4:(.*)|4:$SHA_3|" /home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/prevbuilds
  sed -E -i "s|^5:(.*)|5:$SHA_4|" /home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/prevbuilds

  #Update compiler wrapper scripts
  compiler=gcc

  for wrapper in mpicc mpicxx mpifort mpif77
  do

    sed -E -i "s|^prefix=(/home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/)LATEST/.*|prefix=\1$CI_COMMIT_SHA/$compiler|" /home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/$CI_COMMIT_SHA/$compiler/bin/$wrapper
    sed -E -i "s|^exec_prefix=(/home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/)LATEST/.*|exec_prefix=\1$CI_COMMIT_SHA/$compiler|" /home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/$CI_COMMIT_SHA/$compiler/bin/$wrapper
    sed -E -i "s|^sysconfdir=(/home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/)LATEST/.*|sysconfdir=\1$CI_COMMIT_SHA/$compiler/etc|" /home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/$CI_COMMIT_SHA/$compiler/bin/$wrapper
    sed -E -i "s|^includedir=(/home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/)LATEST/.*|includedir=\1$CI_COMMIT_SHA/$compiler/include|" /home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/$CI_COMMIT_SHA/$compiler/bin/$wrapper
    sed -E -i "s|^libdir=(/home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/)LATEST/.*|libdir=\1$CI_COMMIT_SHA/$compiler/lib|" /home/runbot/mvapich2/merges/$BRANCH/$TYPE${CLUSTER_ABBREV}_${channel}_$BUILDER_SUFFIX/$CI_COMMIT_SHA/$compiler/bin/$wrapper

  done

done
