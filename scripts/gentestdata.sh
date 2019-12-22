#!/bin/sh


for _i in `seq 1 2500` ; do
    uid=$(tr -cd [:alpha:] < /dev/urandom | head -c8)
    name=$(tr -cd [:alpha:] < /dev/urandom | head -c32)
    printf "%s:x:%d:1000:%s:/home/%s:/bin/nologin\n" "$uid" $_i "$name" "$uid"
done > $1/passwd.cache

for _i in `seq 1 2500` ; do
    gid=$(tr -cd [:alpha:] < /dev/urandom | head -c8)
    name=$(tr -cd [:alpha:] < /dev/urandom | head -c32)
    printf "%s:x:%d:\n" "$gid" $_i
done > $1/group.cache

while read line ; do
    uid=$(echo $line | cut -f1 -d:)
    printf "%s::::::::\n" $uid
done < $1/passwd.cache > $1/shadow.cache
