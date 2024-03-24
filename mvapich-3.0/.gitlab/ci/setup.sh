#!/bin/bash

# to be sourced in the prescript and set the branch name in the environment
if [ $CI_MERGE_REQUEST_TARGET_BRANCH_NAME ] 
then
    BRANCH=$CI_MERGE_REQUEST_TARGET_BRANCH_NAME
elif [ $PIPELINE_BRANCH_TYPE ]
then
    BRANCH=$PIPELINE_BRANCH_TYPE
fi

echo Setting up for branch: $BRANCH

# TODO: add logic for triggering a PSM pipeline
case $BRANCH in
    "master" )
        export BRANCH   
        export BUILDER="MR_mrail_master"
        mkdir .buildbot
        echo 'try_builders = ["MR_ofi_master", "MR_ucx_master", "MR_sock_master", "REG_mri_ucx_master", "REG_nowlab_ofi_master" ]' > .buildbot/options
	#disabled builders ["MR_mrail_master", "REG_ri2_mrail_master"]
        ;;
    "master-x" | "x" )
        export BRANCH="master-x"
        export BUILDER="MR_mrail_x"
        mkdir .buildbot
        echo 'try_builders = ["MR_mrail_x", "MR_psm_x", "MR_sock_x", "REG_x"]' > .buildbot/options
        ;;
    "gdr" | "next-gdr" )
        export BRANCH="next-gdr"
        export BUILDER="MR_mrail_gdr"
        mkdir .buildbot
        echo 'try_builders = ["MR_mrail_nvidia_gdr", "MR_mrail_amd_gdr", "MR_sock_gdr", "REG_gdr"]' > .buildbot/options
        ;;
    "plus" | "master-plus" )
        export BRANCH="master-plus"
        export BUILDER="MR_ucx_nvidia_plus"
        mkdir .buildbot
        echo 'try_builders = ["MR_ucx_nvidia_plus", "MR_ucx_amd_plus","MR_ofi_nvidia_plus", "MR_ofi_amd_plus", "MR_sock_plus"]' > .buildbot/options
        ;;

    * ) 
        export BRANCH="invalid"
        export BUILDER="invalid"
        echo "$BRANCH does not match a valid pipeline branch"
        exit 1
        ;;
esac
