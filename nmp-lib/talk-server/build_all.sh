#! /bin/sh

#**********************************************************
#* build_all.sh
#*
#* build all version(debug or release depend on input para)
#*
#* Copyright(c) by HiMickey, 2010~2014
#* Author: HiMickey
#* Date: Nov 8th, 2010
#*
#**********************************************************

usage()
{
	echo "usage: sh $0 [option]" 
	echo ""
	echo "build: Not enough arguments provided"
	echo "build: Try 'sh $0 help' for more info"
	echo ""
	
	exit 1  
}

if [ $# -ne 1 ]
then
	usage $*
else
	if [ "debug" ==  "$1" -o "release" ==  "$1" ]
	then
		TARGET=$1
	else
		echo "Available options:"
		echo ""
		echo "    debug"
		echo "    release"
		echo ""

		exit 1
	fi
fi

#sh build_script.sh $TARGET 3516
#sh build_script.sh $TARGET 3518
#sh build_script.sh $TARGET 3520
#sh build_script.sh $TARGET 3531
#sh build_script.sh $TARGET ti36x
sh build_script.sh $TARGET x86 



