#!/bin/bash

#make clean
#make
test_num=5
rm -r ./test_files/output
rm single_result.txt
test_case=6000mb_file
    echo "***************************************************" >> single_result.txt
    echo "FILE: $test_case" >> single_result.txt
    echo "***************************************************" >> single_result.txt
    for (( i=1; i<=${test_num}; i=i+1 ))
    do
        echo "###############case ${i}" >> single_result.txt
        make clean && make
        #echo "./test_files/${test_case}"
        ./aes "test_files/${test_case}" >> single_result.txt
    done
    echo "===================================================================================" >> single_result.txt

echo "DONE!!!!!!"
echo "Please check 'single_result.txt'"
