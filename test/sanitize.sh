#!/bin/bash

if [[ $# != 2 ]]; then
    echo "usage: $0 lab-dir source-file"
    exit 0
fi

# Add BLESSING files to each directory
while IFS="" read -r line || [ -n "$line" ]; do
    part=$(echo $line | awk '{ print($(NF-1), $NF); }')
    canvasid=$(echo $part | cut -d' ' -f1)
    blessing=$(echo $part | cut -d' ' -f2)
    seed=$(grep -o 'Seed: [0-9]\+' $1/$canvasid/$2.out.txt)
    if [[ -z "$seed" ]]; then
        seed='0'
    else
        seed=$(head -n1 <<< "$seed" | cut -d' ' -f2)
    fi
    echo $seed > $1/$canvasid/BLESSING
    echo $blessing >> $1/$canvasid/BLESSING
done < $1/REPORT.tsv

# Delete all files other than source files, BLESSING files, and REPORT.tsv
find $1 -type f | grep -v "$2\$" | grep -v 'REPORT\.tsv' | grep -v 'BLESSING' | xargs -I{} rm -vf {}

# Remove comments and blank lines from source files
for src in $(ls $1/*/$2); do
    nocomment=$(sed 's/;.*//' $src)
    noblank=$(sed '/^[[:space:]]*$/d' <<< "$nocomment")
    sed 's/\r//' > $src <<< "$noblank"
done

# Sanity check to make sure there isn't any revealing data
search=$(cut -d'	' -f1-3 $1/REPORT.tsv | sed 's/\t/\n/g' | sed 's/,/\n/g')
for query in $search; do
    echo "========== $query =========="
    grep --color=always -i $query $1/*/$2
done

# Rename directories to further anonymize
numdir=$(ls -l $1 | grep '^d' | wc -l)
nums=$(shuf -i 100-999 -n $numdir)
for canvasid in $1/*/; do
    num=$(head -n1 <<< "$nums")
    nums=$(sed '1d' <<< "$nums")
    mv $canvasid $1/$num
done
