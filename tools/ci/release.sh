#! /usr/bin/env bash
#
# Script run by CI to create the release package

# Script fails on an unchecked error exit code
# script echos everything
set -ex

title () {
    set +x
    title_len=$(printf "$1" | wc -c)
    echo
    echo $1
    printf "=%.0s" $(seq $title_len)
    printf "\n"
    echo
    set -x
}

ROOT_DIR=$(pwd)
RELEASE_DIR=${ROOT_DIR}/release
mkdir -p ${RELEASE_DIR}
title "Files and folders to copy"
cat $(pwd)/tools/ci/release-sources-allowlist.txt
title "Create source release package"
cat $(pwd)/tools/ci/release-sources-allowlist.txt | xargs -I% cp -r --parents %  ${RELEASE_DIR}
title "Zip files"
VERSION=$(sed -n '4p' < $(pwd)/CHANGELOG.rst)
zip -r host_xvf_control_release_v${VERSION}.zip release
title "Completed"
