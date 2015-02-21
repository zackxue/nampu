#! /bin/sh

#**********************************************************
#* config.sh
#*
#* .
#*
#* Copyright(c) by HiMickey, 2010~2014
#* Author: HiMickey
#* Date: Jan 11th, 2011
#*
#**********************************************************


if [ $TAR_DIR ]
then
	echo ""
else
	echo "Current prefix is wrong!"
	echo ""
	echo "Try: source set-<version>-env"
	echo ""
	exit 1
fi

MY_TARGET=$TAR_DIR/h264bitstream

./configure \
	--prefix=$MY_TARGET \
	--host=$SHARE_HOST \
	--target=$SHARE_TARGET

echo ""
echo "+++++++++++++++++++++ $MY_TARGET +++++++++++++++++++++"
echo ""


