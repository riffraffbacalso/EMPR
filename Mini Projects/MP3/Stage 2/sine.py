# run as `python3.6 sine.py > sine.h`

from math import sin , pi

amplitude = 1023 # 1023 is the maximum for the DAC
samples = 2**10 # must be power of two, 1024 ~= 1 second
hertz = 1000

step = ((2 * pi) / samples) * hertz

print('static const uint32_t sin_table[' , samples , '] = {')

for i in range(samples):
    r = i * step # angle in radians
    s = sin(r)
    s = s + 1 # shift into range [0,2]
    s = s / 2 # divide into range [0,1]
    s = round(s * amplitude) # multiply by amplitude
    print(s,',')

print('};\n')