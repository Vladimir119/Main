# Big Integer
## Problem
There is library with big integer in this branch. <br />
There is a flaw in programming language C++ - small size of integer type of data. Forexaple <br />
``` int - 4 bytes ``` from -2147483648 to 2147483647 <br /> 
``` long long - 8 bytes ``` from −9 223 372 036 854 775 808 to 9 223 372 036 854 775 807 <br />
It may be very short for other problem! <br />
## Solution
I made structure of date BigInteger. It has no length limit. Thanks to dynamic memory allocation, like a vector. <br />
## Functional
My bigint supports: <br />
 1) standard arithmetic
 2) standart input and output
 3) cast to string and int
 4) and cast from int and string
## P.S
There is class RATIONAL in this repository. It is class for bit rational number. It have denominator and numerator - bigint. 
And supports the same operations as bigint. He also always reduces the numerator and denominator to irreducible form using gcd algorithm.
