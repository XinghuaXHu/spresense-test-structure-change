#! /usr/bin/env python3
import sys
import os
import subprocess
import time
import datetime

TEST_CAMERA_DATA_PATH = os.getcwd()

def usbcommand(command):
    p3 = subprocess.Popen(command, stdout=subprocess.PIPE, shell=True)
    (output3, err3) = p3.communicate()
    p3.wait()
    #print("output:{}".format(output3))
    #print("err3:{}".format(err3))
    print("p3.returncode:{}".format(p3.returncode))


folderlist = os.listdir(TEST_CAMERA_DATA_PATH)
tests = []

for foldername in folderlist:
    if foldername.split("_")[0] != "Test":
        continue

    testcase = foldername.split("_", 1)[-1]
    folderpath = TEST_CAMERA_DATA_PATH + '/' + foldername
    test_filename = ""
    usbcommand("mkdir {}/log".format(folderpath))
    for root, dirs, files in os.walk(folderpath):
        for filename in files:
            if filename.split(".")[0] != "god" and filename.split(".",1)[-1] != "yuv.jpg":
               test_filename = filename
               god_filename = "god." + test_filename.split(".")[-1]
               usbcommand("mv {}/{} {}/{}".format(folderpath,test_filename,folderpath,god_filename))
               break
               

    
