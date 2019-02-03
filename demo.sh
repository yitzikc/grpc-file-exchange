#!/bin/bash
set -e

if [[ $# -ne 1 ]] ; then
    2>&1 echo "usage: $0 filename"
    exit 1
fi

filename=$1
if [[ ! -r $filename ]] ; then
    2>&1 echo "The file" $filename "is not readable"
    exit 2
fi

mkdir -p data
rm -f data/*

( cd data/ && exec ../file_exchange_server ) &
server_pid=$!

./file_exchange_client put 1 $filename
file_basename=`basename $filename`
diff -q "data/${file_basename}" $filename || ( 2>&1  echo "The file was not uploaded correctly" && exit 3 )

./file_exchange_client get 1
diff -q $file_basename $filename || ( 2>&1 echo "The file was not retrieved from the server correctly" && exit 3 )

echo "Cleaning up..."
rm -f $file_basename "data/${file_basename}"
kill -INT $server_pid
