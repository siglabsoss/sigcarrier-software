#!/bin/sh -e

# FAQ
#
# # ./run.sh
# ./packaging/build.sh: line 24: rpmspec: command not found
#
# -> dnf install rpm-build

./packaging/build.sh packaging/TomyGateway.spec .
