#!/bin/bash
echo "Looking for line '$1' .."
lines1=(`cat "./configure.out" | grep -c "$1"`)
if [[ "$lines1" -ne "1" ]]; then
  echo "Error: Line $1 not found!"
  exit 1
fi
echo "Line '$1' was found."
