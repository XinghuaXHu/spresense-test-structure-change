#! /usr/bin/env python3
from junitparser import JUnitXml
import os
import time

class xmllog():
    def __init__(self):
        self.filename       = ""
        self.result         = "pass"
        self.errors         = 0

    def getResult(self):
        return self.result
    
    def getErrors(self):
        return self.errors

    def readXmlLogDir(self, path, timeout):
        if os.path.isdir(path) == False:
            return 0

        return self.readXmlLog(path, timeout)
        

    def readXmlLog(self, path, timeout):
        timeout_count = 0

        while(timeout > timeout_count):
            time.sleep(1)
            timeout_count += 1

            filelist = os.listdir(path)
            if len(filelist) == 0:
                continue 
            else:
                filelist.sort(reverse=True)
                filename = path + "/"+ filelist[0]
                if os.path.isfile(filename) == True:
                   self.filename = filename
                   break


        if timeout == timeout_count:
           self.result = "fail"
           return 0
        
        for i in range(3):
          try:
            suite = JUnitXml.fromfile(filename)
          except:
            time.sleep(1)
            continue
          else:
            break
      
        self.errors  = suite.errors

        if self.errors == 0:
            self.result = "pass"
        else:
            self.result = "fail"

        return 1
