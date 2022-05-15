#!/bin/bash
set -e -u

export LC_ALL=C.UTF-8

outdir="$1"

for _i in `seq 1 2500` ; do
    uid=$(tr -cd [:alpha:] < /dev/urandom | head -c8)
    name=$(tr -cd [:alpha:] < /dev/urandom | head -c32)
    printf "%s:x:%d:1000:%s:/home/%s:/bin/nologin\n" "$uid" $_i "$name" "$uid"
done > "${outdir}/passwd.cache"

for _i in `seq 1 2500` ; do
    gid=$(tr -cd [:alpha:] < /dev/urandom | head -c8)
    name=$(tr -cd [:alpha:] < /dev/urandom | head -c32)
    printf "%s:x:%d:\n" "$gid" $_i
done > "${outdir}/group.cache"

while read line ; do
    uid=$(echo $line | cut -f1 -d:)
    printf "%s::::::::\n" $uid
done < "${outdir}/passwd.cache" > "${outdir}/shadow.cache"

while read line ; do
    gid=$(echo $line | cut -f1 -d:)
    printf "%s:::\n" $gid
done < "${outdir}/group.cache" > "${outdir}/gshadow.cache"
