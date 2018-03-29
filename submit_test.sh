#!/bin/bash
# P4 submission script - tests tar, compile, and sssp.mips output
# Lee B. Hinkle, Texas State University
# Instructions - run this on zeus
#   make a new temp directory $mkdir temp
#   copy the following files into this directory
#     this script file
#     your tar file
#     the input file sssp.mips
#     the output file sssp.out from TRACS
#   if you haven't already cd into the temp directory
#   change the filename below to match your tar filename
#   Note:  a quirk of bash scripts is no spaces can be in assignment statements
#   execute the script
#   if you get an file error you may have to $chmod +x submit_test
#   You should see the result of untar, make, execute, and compare
#   with final message of: 
#   "Files sssp.out and temp.out are identical"
filename=yourNetID_project4.tgz
tar xzvf $filename
if [ -e CPU.cpp ]; then
    echo "untar successful"
        make
        if [ -e ./simulator ]; then
	    ./simulator sssp.mips | tee temp.out
            #tee redirects output to screen and file
            diff -w -s -y sssp.out temp.out
	else
            echo "ERROR:  Did not compile, no ./simulator found"
        fi
else
    echo "ERROR: tar issue, no CPU.cpp found"
fi
