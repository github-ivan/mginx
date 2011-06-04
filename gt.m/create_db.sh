#!/bin/bash

. ../gt.m/gtmprofile

$gtm_exe/mumps -run GDE <<-aaa
ch -seg DEFAULT -file=../gt.m/mumps.dat
exit
aaa

$gtm_exe/mupip create

