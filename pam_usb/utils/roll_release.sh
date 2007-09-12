#!/bin/sh
#
# roll_release.sh
#
# Rolls a distribution tarball from the svn trunk
# and performs basic QA checks.
#

TRUNK_PATH="../"

clean_sources()
{
	cd $TRUNK_PATH
	make clean >> /dev/null || exit

	if [ "`svn st`" ] ; then
		echo "! Directory $TRUNK_PATH is not clean !"
		svn st
		exit
	fi

	cd - > /dev/null
}

create_release()
{
	BUILD_ENV=`mktemp -d /tmp/build.XXXXXX`
	SRC_PATH=${BUILD_ENV}/pam_usb-${1}
	TARBALL=pam_usb-${1}.tar.gz
	TAG_PATH=${TRUNK_PATH}/../../tags/${1}

	if [ -d $TAG_PATH -o -f $TARBALL ] ; then
		rm -rf $BUILD_ENV
		echo "! Release $1 already exists !"
		exit
	fi

	echo "* Rolling release $1 on $BUILD_ENV..."

	svn cp $TRUNK_PATH/pam_usb $TAG_PATH
	svn export $TRUNK_PATH $SRC_PATH

	echo "* Cleaning up..."
	rm -rf $SRC_PATH/utils

	echo "* Tagging release \"$1\""
	sed -ri "s/(PUSB_VERSION) \"[^\"]*\"/\1 \"${1}\"/" ${SRC_PATH}/src/version.h
	cp -f ${SRC_PATH}/src/version.h ${TAG_PATH}/src/version.h

	echo "* Creating tarball..."
	cd $BUILD_ENV
	tar -zcf $TARBALL pam_usb-${1}
	cd - > /dev/null

	cp ${BUILD_ENV}/${TARBALL} .
	rm -rf $BUILD_ENV

	echo "* Release $1 successfully rolled."
	echo "* Tarball stored on `pwd`/${TARBALL}"
	md5sum $TARBALL
}

if [ "x$1" = "x" ] ; then
	echo "Usage: roll_release.sh <version>"
	exit
fi

cd `dirname $0`

clean_sources
create_release "$1"
