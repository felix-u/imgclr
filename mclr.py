#!/usr/bin/env python3

from PIL import Image
import argparse
#
# parsing
parser = argparse.ArgumentParser()

parser.add_argument("input",
                    help='input file')
parser.add_argument("output",
                    help='output file')
parser.add_argument("-p", "--palette", nargs="+",
                    help='input custom palette')

args = parser.parse_args()

inputfile=args.input
outputfile=args.output

# open image
image = Image.open(inputfile)

# prepare new image
out = Image.new('RGB', image.size, 0xffffff)

# width, height of input image
width, height = image.size

# get palette scheme to compare to
colours = []
colours = args.palette

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

matchCount = 0
newCount = 0

# iterate over every pixel
for x in range(width):
    for y in range(height):

        # get pixel RGB colour values
        r,g,b = image.getpixel((x,y))
        pixelVal = f"{r}{g}{b}"

        # check if match previously found
        if pixelVal in matches:
            out.putpixel((x,y), scheme[matches[pixelVal]])
            matchCount += 1


        # otherwise, calculate best match
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
            newCount += 1

            # save match
            matches[pixelVal] = bestMatch

# save output
out.save(outputfile)

print(f"{newCount} original matches calculated.")
print(f"{matchCount} calculated matches repeated.")
