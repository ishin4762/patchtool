#!/bin/bash
if [ $# -lt 1 ] ; then
    echo "usage: deploy.sh architecture [push]"
    exit 1
fi

ARCH=$1
PUSH=$2

function generate_license() {
    OUTFILE=dist/${ARCH}/LICENSE
    # patchtool
    echo -e "patchtool\n" > $OUTFILE
    cat ./LICENSE >> $OUTFILE

    # bzip2
    echo -e "\n---\n" >> $OUTFILE
    echo -e "bzip2\n" >> $OUTFILE
    cat src/lib/bzip2/LICENSE >> $OUTFILE

    # bsdiff
    echo -e "\n---\n" >> $OUTFILE
    echo -e "bsdiff\n" >> $OUTFILE
    cat src/lib/bsdiff/LICENSE >> $OUTFILE
}

function refresh_dist() {
    if [[ "$ARCH" == "win64" ]]; then
        rm -r dist/${ARCH}/*
        cp build/src/cli/patchgen.exe dist/${ARCH}/
        cp build/src/cli/patchapply.exe dist/${ARCH}/
        cp build/src/cli/selfapply.exe dist/${ARCH}/
    fi

    if [[ "$ARCH" == "win32" ]]; then
        rm -r dist/${ARCH}/*
        cp build/src/cli/patchgen.exe dist/${ARCH}/
        cp build/src/cli/patchapply.exe dist/${ARCH}/
        cp build/src/cli/selfapply.exe dist/${ARCH}/
        # in order to escape win binary push simultaneously
        sleep 10
    fi

    if [[ "$ARCH" == "macos" ]]; then
        rm -r dist/${ARCH}/*
        cp build/src/cli/patchgen dist/${ARCH}/
        cp build/src/cli/patchapply dist/${ARCH}/
        cp build/src/cli/selfapply dist/${ARCH}/
    fi
}

# generate texts
generate_license

# push
if [[ "$PUSH" == "push" ]]; then
    git checkout ${TRAVIS_BRANCH}
    git pull https://${GITHUB_TOKEN}@github.com/${TRAVIS_REPO_SLUG}.git ${TRAVIS_BRANCH}
    refresh_dist
    git add ./dist/${ARCH}
    git commit -m "[ci skip] commit by Travis CI (JOB ${TRAVIS_JOB_NUMBER})"
    git push https://${GITHUB_TOKEN}@github.com/${TRAVIS_REPO_SLUG}.git ${TRAVIS_BRANCH}
fi
