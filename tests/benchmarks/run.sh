#!/bin/bash

echo -e "\n\n# Node" >> time_output.txt
for i in {1..5}; do
	{ time node $(dirname "$0")/fizzbuzz.js; } 2>> time_output.txt
done

echo -e "\n\n # lua" >> time_output.txt
for i in {1..5}; do
	{ time lua $(dirname "$0")/fizzbuzz.lua; } 2>> time_output.txt
done

echo -e "\n\n # Toy" >> time_output.txt
for i in {1..5}; do
	{ time out/repl.out -f $(dirname "$0")/fizzbuzz.toy; } 2>> time_output.txt
done