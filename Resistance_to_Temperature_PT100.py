#!/usr/bin/env python

# Convert resistance of PT100(385)  RTD to temperature
#(R0*A-SQRT((R0*A)^2  - 4 * R0 * B * (R0 - R)))/(-2*R0*B)

import sys
import math
display_in_Fahrenheit = 0
R0 = 100            #  PT100
A = 0.0039083       #  (385)
B = -0.0000005775   #
try:
    R = float(sys.argv[1]) 
    print()
    if(display_in_Fahrenheit):
        print("%.3f °F" % float((R0*A-math.sqrt((R0*A)**2  - 4 * R0 * B * (R0 - R)))/(-2*R0*B)*1.8+32))
    else:
        print("%.3f °C" % float((R0*A-math.sqrt((R0*A)**2  - 4 * R0 * B * (R0 - R)))/(-2*R0*B)))
except Exception:
    print()
    print("Please enter the resistance value after ", sys.argv[0])