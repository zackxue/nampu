#! /bin/sh

#**********************************************************
#* build_script.sh
#*
#* Compile the project Depend on input para, 
#* and all so install to the target directory before 
#* rebuild the static-library.
#* 
#* 
#* Copyright(c) by HiMickey, 2010~2014
#* Author: HiMickey
#* Date: Jan 11th, 2011
#*
#**********************************************************

usage()
{
	echo "usage: sh $0 [option] [os-platform]" 
	echo ""
	echo "build: Try 'sh $0 help' for more info"
	echo ""

	exit 1  
}


if [ $# -ne 2 ] 
then
	usage $*
else
	if [ "debug" ==  $1 -o "release" ==  $1 ]
	then
		OPTION=$1
	else
		usage $*
	fi
	
	if [ "$2" == "x86" ]
	then
		TARGET="x86"
#	else if [ "$2" == "3516" ]
#	then
#		TARGET="hi3516-ipc"
#	else if [ "$2" == "3518" ]
#	then
#		TARGET="hi3518-ipc"
#	else if [ "$2" == "3520" ]
#	then
#		TARGET="hi3520-dvr"
#	else if [ "$2" == "3531" ]
#	then
#		TARGET="hi3531-nvr"
#	else if [ "$2" == "ti36x" ]
#	then
#		TARGET="ti36x-ipnc"
	else if [ "$2" == "help" ]
	then
		echo "Available options:"
		echo ""
		echo "    debug"
		echo "    release"
		echo ""
		
		echo "Available os-platform:"
		echo ""
		echo "    x86"
#		echo "    3516"
#		echo "    3518"
#		echo "    3520"
#		echo "    3531"
#		echo "    ti36x"
		echo ""

		exit 1
	else
		echo "usage: 'sh $0 $1 $2' Error: '$2'" 
		echo "build: Unavailable [os-platform: $2]"
		echo "build: Try 'sh $0 help' for more info"
		echo ""

		exit 1
	fi fi #fi fi fi fi fi
fi

SETENV="set-$OPTION-env"
source ./../env/$TARGET
source ./$SETENV

set -x

make clean
#make distclean

#sh autogen.sh
#sh config.sh

make uninstall
make -j9999

#sh rebuild.sh
make install

