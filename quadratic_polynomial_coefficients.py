#!/usr/bin/env python

'''

  To calculate the coefficients for the quadratic polynomial (temperature = a * ADC^2 + b * ADC + c)
used in the pt100 module, you’ll need three resistors with known values.  I used 100.12Ω, 
179.71Ω, and 219.34Ω. (sum of resistor and resistance of wires connecting the resistor to RTD plug)
  Look up the corresponding temperature for resistance for the type of RTD used.
For PT100(385) RTD, the temperature values would be 0.307°C for 100.12Ω, 210.498°C for 179.71Ω,
and 320.531°C for 219.34Ω. 
  Connect the resistors one at a time in place of the RTD and get the ADC values 
using the M305 command. I got  193 for 100.12Ω, 8958 for 179.71Ω, and 13153 for 219.34Ω. 
  Edit this file and replace ADC1, Temp1, ADC2, Temp2, ADC3, Temp3 with 
your ADC and temperature pairs.	
	Here are Cetus MK3 ADC vs. Temperature pairs as an example:
	adc_vs_temperature = [[193, 0.3], [8958, 210.5], [13153, 320.5]]
When you run this script, it’ll show you the values of a, b, and c.

'''

adc_vs_temperature = [[ADC1, Temp1], [ADC2, Temp2], [ADC3, Temp3]]

import copy
for i, (adc, temperature) in enumerate(adc_vs_temperature):
    adc_vs_temperature[i].append(adc*adc)
adc_vs_temperature_copy2 = copy.deepcopy(adc_vs_temperature[2])
adc_vs_temperature[1][0] -= adc_vs_temperature[0][0]
adc_vs_temperature[1][1] -= adc_vs_temperature[0][1]
adc_vs_temperature[1][2] -= adc_vs_temperature[0][2]
adc_vs_temperature[2][0] -= adc_vs_temperature[0][0]
adc_vs_temperature[2][1] -= adc_vs_temperature[0][1]
adc_vs_temperature[2][2] -= adc_vs_temperature[0][2]
adc_vs_temperature_copy = copy.deepcopy(adc_vs_temperature[2])
multiplier = adc_vs_temperature[2][0] / adc_vs_temperature[1][0]
adc_vs_temperature[1][1] *= multiplier
adc_vs_temperature[1][2] *= multiplier
adc_vs_temperature[2][1] -= adc_vs_temperature[1][1]
adc_vs_temperature[2][2] -= adc_vs_temperature[1][2]
a = adc_vs_temperature[2][1] / adc_vs_temperature[2][2]
print()
print ("a = ", '{:.20f}'.format(a))
b = (adc_vs_temperature_copy[1] - (adc_vs_temperature_copy[2] * a))/adc_vs_temperature_copy[0]
print ("b = ", b)
c = adc_vs_temperature_copy2[1] - (adc_vs_temperature_copy2[2] * a + adc_vs_temperature_copy2[0] * b)
print ("c = ", c)
print()