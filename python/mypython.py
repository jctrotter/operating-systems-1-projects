import random
from random import randint

f1 = open("cobra","w+")
f2 = open("viper","w+")
f3 = open("snake","w+")

alphabet = 'abcdefghijklmnopqrstuvwxyz'

wString = ''

for y in range(0,10):
    wString = wString + random.choice(alphabet)
wString = wString
f1.write(wString)
print(wString)
wString = ''

for y in range(0,10):
    wString = wString + random.choice(alphabet)
wString = wString
f2.write(wString)
print(wString)
wString = ''

for y in range(0,10):
    wString = wString + random.choice(alphabet)
wString = wString
f3.write(wString)
print(wString)

num1 = randint(1,42)
num2 = randint(1,42)
product = num1 * num2
print(str(num1) + '\n' + str(num2) + '\n' + str(product))                                                                  
