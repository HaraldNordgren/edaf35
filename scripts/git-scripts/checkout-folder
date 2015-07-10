#!/bin/bash

if [ -z "$1" ]
then
    echo "usage: repo-url [folder]"
    exit 1
fi

git init
git remote add -f origin $1

if [ -n "$2" ]
then
    git config core.sparseCheckout true
    echo $2 >> .git/info/sparse-checkout
fi

git pull origin master
