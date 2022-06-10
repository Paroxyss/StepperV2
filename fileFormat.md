# Cmid File Format (.cm)

## Header : 

char* name          16
byte tempo          1
Uint timeDivision   4
Total :             21 octets

## Events

byte off/on (0/1)   1
byte note           1
byte channel        1
Uint delta          4
TOTAL :             7 octets