#/bin/bash

SQLITE_VERSION=${SQLITE_VERSION:-"3.43.0"}
OPENSSL_VERSION=${OPENSSL_VERSION:-"1.1.1w"}
# Used for SQLite download URL
SQLITE_VERSION_RELEASE_YEAR=2023

SQLITE_VERSION_LIST=(${SQLITE_VERSION//./ })
SQLITE="sqlite-amalgamation-$(printf "%d%02d%02d%02d" "${SQLITE_VERSION_LIST[@]}")"
OPENSSL="openssl-${OPENSSL_VERSION}"

BASEDIR=$(pwd)
DEPDIR="$BASEDIR/deps"
TEMPDIR="$BASEDIR/temp"

# usage helper
usage()
{
	echo "Usage: $0"
	exit 1
}

# download helper
download()
{
	local url="$1"
	local output="$2"

	if command -v curl >/dev/null; then
		curl -SLO "$url" -o "$output"
	elif command -v wget >/dev/null; then
		wget --progress=bar:force:noscroll -O "$output" "$url"
	elif command -v fetch >/dev/null; then
		fetch -o "$output" "$url"
	else
		echo "Error: Neither 'curl', nor 'wget', nor 'fetch' commandd found. Please install one or check your PATH environment variable." >&2
		exit 1
	fi
}

# dependencies: SQLite
get_sqlite()
{
	if [ -d "$DEPDIR/sqlite-amalgamation" ]; then
		echo "SQLite folder already exists locally. The downloaded copy will not be used." >&2
		return
	fi

	cd "$TEMPDIR"
	download "https://www.sqlite.org/$SQLITE_VERSION_RELEASE_YEAR/$SQLITE.zip" "$TEMPDIR/$SQLITE.zip"
	cd "$BASEDIR"

	unzip -q "$TEMPDIR/$SQLITE.zip" -d "$TEMPDIR"
	mv "$TEMPDIR/$SQLITE" "$DEPDIR/sqlite-amalgamation"
}

# dependencies: OpenSSL
get_openssl()
{
	if [ -d "$DEPDIR/openssl-1.1.1w" ]; then
		echo "OpenSSL folder already exists locally. The downloaded copy will not be used." >&2
		return
	fi

	cd "$TEMPDIR"
	download "https://www.openssl.org/source/openssl-$OPENSSL_VERSION.tar.gz" "$TEMPDIR/$OPENSSL.tar.gz"
	tar -zxf "$TEMPDIR/$OPENSSL.tar.gz" --directory "$TEMPDIR"
	cd "$BASEDIR"

	# Build OpenSSL and get files
	cd "$TEMPDIR/$OPENSSL"

	./Configure no-shared --release --api=1.1.0 no-deprecated no-ssl2 no-ssl3 no-md2 no-rc4 no-idea no-camellia no-ec no-engine no-tests linux-x86_64
	make

	mkdir -p "$DEPDIR/$OPENSSL"
	cd "$BASEDIR"
	mv "$TEMPDIR/$OPENSSL/libcrypto.a" "$DEPDIR/$OPENSSL/libcrypto.a"
	mv "$TEMPDIR/$OPENSSL/libssl.a" "$DEPDIR/$OPENSSL/libssl.a"
	mv "$TEMPDIR/$OPENSSL/include" "$DEPDIR/$OPENSSL/include"
}

# main process
get_deps()
{
	if [ "$#" -eq 1 ] && [ "$1" == "help" ]; then
		usage
		exit 0
	elif [ "$#" -ne 0 ]; then
		echo "Error: Invalid argument. Use '$0 help' for usage instructions." >&2
		exit 1
	fi

	mkdir -p "$TEMPDIR"
	mkdir -p "$DEPDIR"

	get_sqlite
	get_openssl

	rm -rf "$TEMPDIR"
}

get_deps "$@"
