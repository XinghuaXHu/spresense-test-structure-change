#! /usr/bin/env python3
import sys
import os
import time
import numpy
import wave
import subprocess
#import serial

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)

from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.adn_dual_players.player_device import PlayerDevice
from tc.adn_dual_players.recorder_device import RecorderDevice
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['']
DSP_PATH = 'sdk/modules/audio/dsp'
DECODERS = ['MP3DEC']
ENCODERS = ['SRC']
AUDIO_FILES = ['Sound0.mp3', 'Sound1.mp3']
AUDIO_EXAMPLE_PATH = '/mnt/sd0/'

global gmedia_dict
gmedia_dict = {}
global gmedia_dict2
gmedia_dict2 = {}
global gmedia_path
gmedia_path = {}

TEST_AUDIO_FILE_FREQUENCY = 1000
TEST_AUDIO_FILE_FREQUENCY2 = 500
FREQUENCY_VERIFY_ACCURACY = 2



# noinspection PyUnusedLocal
def findpeaks(data, spacing=1, limit=None):
    """Finds peaks in `data` which are of `spacing` width and >=`limit`.
    :param data: values
    :param spacing: minimum spacing to the next peak (should be 1 or more)
    :param limit: peaks should have value greater or equal
    :return:
    """
    len = data.size
    x = numpy.zeros(len + 2 * spacing)
    x[:spacing] = data[0] - 1.e-6
    x[-spacing:] = data[-1] - 1.e-6
    x[spacing:spacing + len] = data
    peak_candidate = numpy.zeros(len)
    peak_candidate[:] = True
    for s in range(spacing):
        start = spacing - s - 1
        h_b = x[start: start + len]  # before
        start = spacing
        h_c = x[start: start + len]  # central
        start = spacing + s + 1
        h_a = x[start: start + len]  # after
        peak_candidate = numpy.logical_and(peak_candidate, numpy.logical_and(h_c > h_b, h_c > h_a))

    ind = numpy.argwhere(peak_candidate)
    ind = ind.reshape(ind.size)
    if limit is not None:
        ind = ind[data[ind] > limit]

    return ind


# noinspection PyUnusedLocal
def verify_recorded_file(file):
    #test_file_path = os.path.join(os.getcwd(), 'output', file)

    wave_read = wave.open(file)
    sample_rate = wave_read.getframerate()
    frames_number = wave_read.getnframes()

    k = numpy.arange(frames_number)
    T = frames_number / sample_rate
    frequencies = k / T
    frequencies = frequencies[range(int(frames_number / 2))]

    signal = wave_read.readframes(frames_number)
    signal = numpy.frombuffer(signal, numpy.int16)

    ft = numpy.fft.fft(signal) / frames_number
    ft = ft[range(int(frames_number / 2))]
    ft = abs(ft)

    indexes = findpeaks(ft, spacing=3, limit=max(ft)/8)
    frequency = frequencies[indexes]
    #frequency = frequencies[numpy.argmax(ft)]
    #frequency = frequencies[numpy.argmin(ft)]

    freq_range = [i for i in range(TEST_AUDIO_FILE_FREQUENCY - FREQUENCY_VERIFY_ACCURACY,
                                   TEST_AUDIO_FILE_FREQUENCY + FREQUENCY_VERIFY_ACCURACY)]
    freq_range.extend([i for i in range(TEST_AUDIO_FILE_FREQUENCY2 - FREQUENCY_VERIFY_ACCURACY,
                                        TEST_AUDIO_FILE_FREQUENCY2 + FREQUENCY_VERIFY_ACCURACY)]),

    for f in frequency:
        print('Found frequency {} Hz'.format(f))
    return True

if __name__ == "__main__":
    verify_recorded_file("/home/spritzer/16282.wav") 