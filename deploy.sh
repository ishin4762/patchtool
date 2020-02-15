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

if [[ "$ARCH" == "win64" ]]; then
    rm -r dist/${ARCH}/*
    cp build/src/patchgen/patchgen.exe dist/${ARCH}/
    cp build/src/patchapply/patchapply.exe dist/${ARCH}/
fi

if [[ "$ARCH" == "win32" ]]; then
    rm -r dist/${ARCH}/*
    cp build/src/patchgen/patchgen.exe dist/${ARCH}/
    cp build/src/patchapply/patchapply.exe dist/${ARCH}/
fi

if [[ "$ARCH" == "macos" ]]; then
    rm -r dist/${ARCH}/*
    cp build/src/patchgen/patchgen dist/${ARCH}/
    cp build/src/patchapply/patchapply dist/${ARCH}/
fi

# generate texts
generate_license

# push
if [[ "$PUSH" == "push" ]]; then
    openssl aes-256-cbc -K $encrypted_61dd9981ab75_key -iv $encrypted_61dd9981ab75_iv -in travis_key.enc -out ~/.ssh/id_rsa -d
    chmod 600 ~/.ssh/id_rsa    
    echo -e "Host github.com\n\tStrictHostKeyChecking no\n" >> ~/.ssh/config
    git config --global user.email "brokendesk0206@gmail.com"
    git config --global user.name "ishin4762"
    mkdir __git_repo
    cd __git_repo
    git clone git@github.com:ishin4762/patchtool
    cd patchtool
    cp ../../dist/${ARCH}/* ./dist/${ARCH}/
    git add ./dist/${ARCH}
    git commit -m "[ci skip] commit by Travis CI (JOB ${TRAVIS_JOB_NUMBER})"
    git push origin ${TRAVIS_BRANCH}
fi
