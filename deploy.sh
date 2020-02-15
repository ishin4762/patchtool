#!/bin/bash
if [ $# -lt 1 ] ; then
    echo "usage: deploy.sh architecture [push]"
    exit 1
fi

ARCH=$1
PUSH=$2

function generate_license() {
    OUTFILE=release/${ARCH}/LICENSE
    # patchtool
    echo -e "patchtool\n" > $OUTFILE
    cat ./LICENSE >> $OUTFILE

    # bzip2
    echo -e "\n---\n" >> $OUTFILE
    echo -e "bzip2\n" >> $OUTFILE
    cat lib/bzip2/LICENSE >> $OUTFILE

    # bsdiff
    echo -e "\n---\n" >> $OUTFILE
    echo -e "bsdiff\n" >> $OUTFILE
    cat lib/bsdiff/LICENSE >> $OUTFILE
}

if [[ "$ARCH" == "win64" ]]; then
    cp patchgen/patchgen.exe release/${ARCH}/
    cp patchapply/patchapply.exe release/${ARCH}/
fi

if [[ "$ARCH" == "win32" ]]; then
    cp patchgen/patchgen.exe release/${ARCH}/
    cp patchapply/patchapply.exe release/${ARCH}/
fi

if [[ "$ARCH" == "macos" ]]; then
    cp patchgen/patchgen release/${ARCH}/
    cp patchapply/patchapply release/${ARCH}/
fi

# generate texts
cp README.md release/${ARCH}/
generate_license

# push
if [[ "$PUSH" == "push" ]]; then
    openssl aes-256-cbc -K $encrypted_61dd9981ab75_key -iv $encrypted_61dd9981ab75_iv -in travis_key.enc -out ~/.ssh/id_rsa -d
    chmod 600 ~/.ssh/id_rsa
    echo -e "Host github.com\n\tStrictHostKeyChecking no\n" >> ~/.ssh/config
    git config --global user.email "brokendesk0206@gmail.com"
    git config --global user.name "ishin4762"
    git pull origin ${TRAVIS_BRANCH}
    git add release/${ARCH}
    git commit -m "[ci skip] deploy."
    git push origin ${TRAVIS_BRANCH}
fi
