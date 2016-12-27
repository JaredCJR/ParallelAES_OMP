#!/bin/bash

#make clean
#make
test_num=5
rm -r ./test_files/output
rm all_result.txt
for test_case in `ls test_files/`
do
    echo "***************************************************" >> all_result.txt
    echo "FILE: $test_case" >> all_result.txt
    echo "***************************************************" >> all_result.txt
    for (( i=1; i<=${test_num}; i=i+1 ))
    do
        echo "###############case ${i}" >> all_result.txt
        make clean && make
        #echo "./test_files/${test_case}"
        ./aes "test_files/${test_case}" >> all_result.txt
    done
    echo "===================================================================================" >> all_result.txt
done

echo "DONE!!!!!!"
echo "Please check 'all_result.txt'"
