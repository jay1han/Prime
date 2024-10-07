# Prime Factor Decomposition

## Theory

A strictly positive integer can be written as a product of primes.

N = &Pi;~i~ p~i~^k^

Each prime divisor can be present once (k = 1) of multiple times (k > 1).
The *degree* of the integer is the sum of the exponents of its prime factors.

*d*~N~ = &Sigma;~i~ k~i~

So for a prime number, the degree is 1 (itself, exponentiated once);
for 65536 which is 2^8^, the degree is 8.

The goal of these programs is to calculate prime decompositions
and manipulate files containing them.
As a collateral, we generate and use a list of prime numbers.

### Number conventions

- A representation of a number is broken into groups of 3 digits separated by "`_`",
e.g. `1_000_000_000` represents one billion (10^9^).

- In some cases, we can use the suffix `m` and `g` to denote a million or a billion,
e.g. `100m` is `100_000_000`.

## Programs

### `primes`

### `print`

### `merge`

### `repair`

## File formats

### Flexible integers (flexints)

In order to save space in RAM and disk, we use a "flexible integer" type,
which encodes a 64-bit unsigned integer in as small a space as possible.
The principle is similar to UTF encoding.
The most significant bit of each byte denotes whether there are more bits beyond this byte.
The encoding is essentially big-endian.

- If the MSB of the first byte is 0, then the remaining 7 bits represent a number between 0 and 2^7^-1.

- Otherwise, we take those 7 bits, plus the 8 bits of the following byte, to form a 15-bit unit.
If the MSB of this unit (which is bit 6 of the first byte) is 0, then we take the remaining 14 bits
to represent a number between 2^7^ and 2^14^-1.

- Otherwise, if both bits 7 and 6 of the first byte are 1, then we take another byte
to form a 22-bit unit.

- And so on, until the 7th byte. At that point, the system can represent any positive integer
between 0 and 2^49^-1. This is probably the largest number we'll be dealing with (about 5.10^14^).

- If the number is greater than this, we set the first byte as 0xFF and the other 8 bytes as
a straight representation of a 64-bit positive integer.

### Primes

The file named `Primes.N.dat` contains the primes between 2 and N included.
There must be only one such file in the directory.

The file contains a sequence of flexints representing the gap between one prime and the next.
The count starts at zero, so the first prime, 2, is represented by a single byte, 2.
The next, 3, is 1 away from 2, so is represented by a single byte, 1.
Next is 5, which is 2 away from 2, so again we have a single byte, 2.
In many cases where gaps are smaller than 128 (2^7^), a prime takes a single byte.

### Numbers

There may be multiple Numbers file in the directory.

The basic variant, named `Numbers.X-Y.dat`, contains prime factor decompositions of each integer between X and Y both included.
An integer is expressed as the product of divisors, with each divisor being defined by a unique prime and an exponent.

N = &Pi;~i~ p~i~^k^

- Each integer starts with one byte denoting the number of divisors. For each divisor

  - p~i~ is written in flexint

  - and k~i~ as a single byte

This representation is very compact but because the length is variable, it's impossible to directly access an arbitrary number.
The "chunked" variant is named `Numbers.X-Y.Z`, where Z is a small integer (the "chunk" size).
In this file, each number is represented by exactly Z bytes, so that a program can directly access any number.

The "reduced" variant, names `Numbers.X-Y.red`, contains only the *degree* of each integer (the sum of exponents),
in one byte per number.

## Code

### `prime.c`

### `number.c`

### `flexint.c`

### `worker.c`

# Applications

## Visualization

Let's define the function &delta;(*n*) as the degree of the prime decomposition for each integer n.
Let's plot the &delta;(*n*) of all integers within a certain range, say ]0;100][^1].
Prime numbers within that range will be at the bottom (degree = 1),
while powers of 2, which have the highest degree, will "tentpole" the graph.

[^1]: We will set &delta;(1)=0.

Now reduce the scale of *n* by a factor of 2.
This is like plotting &delta;(*2x*) for *x* &isin; &Nopf;/2.
We notice that each point on the previous graph now has a point one step higher,
since for any *n* &isin; &Nopf;, *2n* has one more prime factor, namely 2.

So let's plot &delta;(*2x*)-1 instead. Now all the points on the previous graph
are also present in the new graph, and there are more points in between.
But when *2x* is prime, the point lies on *y=0*.
We need a slightly different transformation.

Partially define a function &lambda;(*n*) for *n* &isin; &Nopf; such that

- &lambda;~2~(*2n*) = &delta;(*n*) / &delta;(*2n*)

- &lambda;~2~(*2n-1*) = 1/2

Then, we define the function &delta;~2~(*x*) = &lambda;~2~(*2x*).&delta;(*2x*),
we have the following properties:

- for every *n* &isin; &Nopf;, &delta;~2~(*n*) = &delta;(*n*)

- for *x* &isin; (&Nopf;/2 - &Nopf;), &delta;~2~(*x*) = &delta;(*2x*)/2[^2]

[^2]: So &delta;~2~(1/2) = 0.

Likewise, we can define &lambda;~p~ for any prime *p*:

- &lambda;~p~(*p.n*) = &delta;(*n*)/&delta;(*p.n*)

- &lambda;~p~(*p.n-i) = 1 / *p*), for all *i* &isin; [1;*p-1*]

We can also define *k*-times repeated applications of the scale reduction: for *k* > 1,

- &lambda;^k^~p~(*p.n*) = &delta;^k-1^~p~(*n*)/&delta;^k-1^~p~(*p.n*)

- &lambda;^k^~p~(*p.n-i*) = 1 / *p*^k^

We want to visualize the graphs of &delta;^k^~p~ for successive values of *k*,
and possibly stack those graphs in the third dimension with different values of *p*.
