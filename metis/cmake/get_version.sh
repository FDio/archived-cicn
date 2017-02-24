#!/bin/sh

DATE_VERSION=`date "+%Y%m%d"`

if [ ! -d $1 ]; then
  echo 0.$DATE_VERSION
  exit 
fi

if [ -f $1/BASE_VERSION ]; then
  BASE_VERSION=`cat $1/BASE_VERSION`.
fi

GIT=`which git`
    
if test -x $GIT -a -f $1/.git/config; then 
	GIT_VERSION=.`git -C $1 rev-parse HEAD | cut -c 1-8`
fi

echo $BASE_VERSION$DATE_VERSION$GIT_VERSION
