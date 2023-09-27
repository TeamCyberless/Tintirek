#!/bin/bash
#
#	GetDependencies.sh
#
#	This shell script file runs install-deps
#	tool for installing project dependencies,
#

cd ./tools/install-deps
exec go run main.go
cd ../..