#! /usr/bin/env python3
import subprocess
import time
import re
import pexpect
import getpass
import os

password = 'spresense'


def getCurrentDisk():
    p = subprocess.Popen("ls /dev/sd*", stdout=subprocess.PIPE, shell=True)
    (output, err) = p.communicate()
    p.wait()
    outputstr = output.decode('UTF-8')
    outputstrArr = outputstr.split()
    #print(outputstrArr)
    usbdict = {}
    for i in outputstrArr:
        usbdict[i] = i
    return usbdict


def autoRemoveMount(usbpath):
    user = getpass.getuser()
    runSudoBashFunction('umount {}'.format(usbpath))
    runSudoBashFunction('rm -fr {}'.format(usbpath))
    runBashFunction('mkdir -p {}'.format(usbpath))
    for (root, dirs, files) in os.walk('/media/' + user):
        for d in dirs:
            runSudoBashFunction('umount /media/{}/{}'.format(user, d))


# the following function can be use in other file
def runSudoBashFunction(command):
    process = pexpect.spawn('sudo /bin/bash -c "{}"'.format(command))
    time.sleep(1)
    process.expect(pexpect.EOF)


def runBashFunction(command):
    process = pexpect.spawn('/bin/bash -c "{}"'.format(command))
    process.expect(pexpect.EOF)


def runBashCommand(command):
    p3 = subprocess.Popen(command, stdout=subprocess.PIPE, shell=True)
    p3.communicate()
    p3.wait()


def umountCurrentUSBDisk(delaytime, usbpath, source, sdk=False):
    runSudoBashFunction('umount {}'.format(usbpath))
    if sdk:
        source.write('msdis')
        time.sleep(5)
        source.write('mount -t vfat /dev/mmcsd0 /mnt/sd0')
    else:
        source.write('end')
    time.sleep(delaytime)


def mountCurrentUSBDisk(delaytime, usbpath, source, sdk=False):
    gmedia_dict = getCurrentDisk()
    if sdk:
        source.write('umount /mnt/sd0')
        time.sleep(1)
        source.write('msconn')
    else:
        source.write('usbmsc')
    time.sleep(delaytime)
    gmedia_dict2 = getCurrentDisk()
    ret = ''
    for key2 in gmedia_dict2.keys():
        key2_count = 0
        for key in gmedia_dict.keys():
            if key == key2:
                key2_count += 1
        pattern = re.compile('[0-9]')
        match = pattern.findall(key2)
        devpath = key2
        if key2_count == 0 and match:
            autoRemoveMount(usbpath)
            time.sleep(5)
            runSudoBashFunction('mount {} {} -o umask=0'.format(devpath, usbpath))
            time.sleep(5)
            mountcmd = 'mount | grep ' + devpath
            p = subprocess.Popen(mountcmd, stdout=subprocess.PIPE, shell=True)
            (output, err) = p.communicate()
            p.wait()
            if err is None and output != b'':
                outputStr = output.decode('UTF-8')
                outputStrArr = outputStr.split()
                ret = outputStrArr[2]
            return ret
    return ret
