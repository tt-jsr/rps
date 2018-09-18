#!/bin/bash

if [[ -z $1 ]]
then
    echo -n "Exchange: "
    read exchange
else
    exchange=$1
fi
/opt/debesys/$exchange/run ~/rps

