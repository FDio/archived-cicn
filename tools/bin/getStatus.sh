#!/bin/sh

GIT_DIRECTORY=$1
GIT_REPO=`basename ${GIT_DIRECTORY}`

cd ${GIT_DIRECTORY}

CURRENT_SHA=`git branch -av | grep "\*" | awk '{print $3}'`
CURRENT_BRANCH=`git branch -av | grep "\*" | awk '{print $2}'`
UPSTREAM_SHA=`git branch -av | grep "remotes/ccnx_upstream/master" | awk '{print $2}'`

if [ x${UPSTREAM_SHA} = x ]; then
  # We don't have an upstream...
  UPSTREAM_SHA="NO_UPSTREAM"
else
  PARC_MASTER_IN_BRANCH=`git branch -v --contains $UPSTREAM_SHA | grep "\*" | awk '{print $3}'`
fi


echo '===================================================================='

if [ "x${CCNX_MASTER_IN_BRANCH}" != "x" ]; then
  # This branch is master OR ahead of master
  if [ x${CURRENT_SHA} = x${UPSTREAM_SHA} ]; then
    echo "CCNX    ${GIT_REPO} ${CURRENT_BRANCH} ${CURRENT_SHA}::${UPSTREAM_SHA}"
  else
    echo "CCNX++  ${GIT_REPO} ${CURRENT_BRANCH} ${CURRENT_SHA}::${UPSTREAM_SHA}"
  fi
else
    echo "------  ${GIT_REPO} ${CURRENT_BRANCH} ${CURRENT_SHA}::${UPSTREAM_SHA}"
fi
git status -s 
