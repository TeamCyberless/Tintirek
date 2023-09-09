#/bin/bash

SQLITE_VERSION=${SQLITE_VERSION:-"3.43.0"}
# Used for SQLite download URL
$SQLITE_VERSION_RELEASE_YEAR=2023

SQLITE_VERSION_LIST=`echo $SQLITE_VERSION | sed -e 's/\./ /g'`
SQLITE=sqlite-amalgamation-`printf %d%02d%02d%02d $SQLITE_VERSION_LIST`

BASEDIR=`pwd`
TEMPDIR=$BASEDIR/temp

# usage helper
usage()
{
	echo "Usage $0"
	exit $1
}

# dependencies: SQLite
get_sqlite()
{
	#test -d $BASEDIR/deps/sqlite-amalgamation && return

	#cd $TEMPDIR
	echo https://www.sqlite.org/$SQLITE_VERSION_RELEASE_YEAR/$SQLITE.zip
	#cd $BASEDIR

	#unzip -q $TEMPDIR/$sqlite.zip
	#mv $SQLITE deps/sqlite-amalgamation
}

# main process
get_deps()
{
	#mkdir -p $TEMPDIR

	for i in sqlite-amalgamation; do
		if [ -d $i ]; then
			echo "Local directory '$i' already exists. The downloaded copy will not be used." >&2
		fi
	done

	get_sqlite

	#rm -rf $TEMPDIR
}

get_deps "$@"
