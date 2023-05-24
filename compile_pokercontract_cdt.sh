#!/bin/bash

a=$(ls -l |grep contracts |cut -b 1)
if [ -z $a ]
then
	mkdir contracts
	mkdir contracts/pokercontract
fi

eosio-cpp -o ./contracts/pokercontract/pokercontract.wasm pokercontract.cpp -abigen --contract pokercontract
