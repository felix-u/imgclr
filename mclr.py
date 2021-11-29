#!/usr/bin/env pypy3

from PIL import Image

import argparse
import psutil # for getting CPU count
import multiprocessing

# from tqdm import tqdm

#
# parsing
parser = argparse.ArgumentParser()

parser.add_argument("input",
                    help='input file')
parser.add_argument("output",
                    help='output file')
parser.add_argument("-c", "--colours", nargs="+",
                    help='input custom palette')
parser.add_argument("-p", "--processes", nargs="+",
                    help='override process count')

args = parser.parse_args()

inputfile=args.input
outputfile=args.output

# establish thread count
global procs
if not args.processes:
    procs = psutil.cpu_count()
    print(f"Starting {procs} processes (custom process count not specified).")
elif int(args.processes[0]) > psutil.cpu_count():
    procs = psutil.cpu_count()
    print(f"Starting {procs} processes (custom process count not possible).")
else:
    procs = int(args.processes[0])
    print(f"Starting {procs} processes.")

# open image
image = Image.open(inputfile)

# prepare new image
out = Image.new('RGB', image.size, 0xffffff)

# width, height of input image
width, height = image.size

# get palette scheme to compare to
global colours
if not args.colours:
    colours = ['#1d1f21', '#cc6666', '#b5bd68', '#81a2be', '#c5c8c6']
    print('Using placeholder scheme (custom palette not specified).')
else:
    colours = args.colours

# convert hex values to RGB
def hexToRGB(hex):
    # remove hash at the front, if present
    hex = hex.lstrip('#')
    # convert every set of two characters to base 10
    return tuple(int(hex[i:i+2], 16) for i in (0, 2, 4))

# convert every value in the colour scheme from hex to RGB
for i in range(len(colours)):
    colours[i] = hexToRGB(colours[i])

# create colour scheme dictionary to use for comparing to the colour scheme
scheme = {}
for i in range(len(colours)):
    scheme[i] = colours[i]

# create dictionary in which to save matches
matches = {}

# iterate over every pixel in range
def processAndConvert(widthStart, widthEnd):
    for x in range(widthStart, widthEnd):
        for y in range(height):

            # get pixel RGB colour values
            r,g,b = image.getpixel((x,y))
            pixelVal = (r,g,b)

            # check if match previously found
            if pixelVal in matches:
                out.putpixel((x,y), scheme[matches[pixelVal]])

            # otherwise, calculate best match and save it
            else:
                comparison = {}
                for i in range(len(scheme)):

                    diffR = scheme[i][0] - r
                    if diffR < 0:
                        diffR = -diffR

                    diffG = scheme[i][1] - g
                    if diffG < 0:
                        diffG = -diffG

                    diffB = scheme[i][2] - b
                    if diffB < 0:
                        diffB = -diffB

                    comparison[i] = diffR + diffG + diffB

                bestMatch = min(comparison, key=comparison.get)
                out.putpixel((x,y), scheme[bestMatch])

                # save match
                matches[pixelVal] = bestMatch

# create list of jobs, then append each process to it
jobs = []
for i in range(1, procs+1):
    widthStart = int((i-1)*width/procs)
    widthEnd = int(i*width/procs)
    process = multiprocessing.Process(target=processAndConvert(widthStart, widthEnd))
    jobs.append(process)

for j in jobs:
    j.start()

for j in jobs:
    j.join()

# save output
out.save(outputfile)
