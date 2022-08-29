#!/usr/bin/env bash

# Add any new files, add all updates to all files
echo "Adding all changes"
git add --all . 
git add -u :/

msg=${1:-"..."}
echo $msg
echo "test?"

# Commit using the message specified as first argument to this script
echo "Git commit"
git commit -m "$msg"

# Synchronize with master on github
echo "git pull"
git pull

echo "git push"
git push origin master
