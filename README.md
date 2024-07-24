## Scrypt
A simple lightweight programming language.

There are four main functions located in `/src`. Namely, `lex.cpp`, `calc.cpp`, `format.cpp` and `scrypt.cpp`. All four main functions take in text input of Scrypt program.

`lex.cpp` Prints out the tokenized version of the program.

`calc.cpp` Accepts a Scrypt program of just expressions and evaluates them line by line.

`format.cpp` Builds an AST of the entire program and prints it out formatted.

`scrypt.cpp` Is the interpreter and evaluates the entire program.


## Compilation
Make sure you are in the src directory. Note: C++17 is required.
```
g++ lex.cpp -std=c++17 -Wall -Wextra -Werror ./lib/*.cpp -o lex
```
```
g++ calc.cpp -std=c++17 -Wall -Wextra -Werror ./lib/*.cpp -o calc
```
```
g++ format.cpp -std=c++17 -Wall -Wextra -Werror ./lib/*.cpp -o format
```
```
g++ scrypt.cpp -std=c++17 -Wall -Wextra -Werror ./lib/*.cpp -o scrypt
```

## Running
To run any of the executables, call the executable and pipe in text input.

### Example
collatz.txt:
```
x     = 42
steps = 0

while x > 1 {
  steps = steps + 1
  if x % 2 == 0 {
    x = x / 2
  }
  else {
    x = 3 * x + 1
  }
}
```
`./lex < collatz.txt`:
```
   1    1  x
   1    7  =
   1    9  42
   2    1  steps
   2    7  =
   2    9  0
   4    1  while
   4    7  x
   .    .
   .    .
   .    .
  10    7  =
  10    9  3
  10   11  *
  10   13  x
  10   15  +
  10   17  1
  11    3  }
  12    1  }
  14    1  print
  14    7  steps
  14   12  END
```

`./format < collatz.txt`:
```
(x = 42)
(steps = 0)
while (x > 1) {
    (steps = (steps + 1))
    if ((x % 2) == 0) {
        (x = (x / 2))
    }
    else {
        (x = ((3 * x) + 1))
    }
}
print steps
```

`./scrypt < collatz.txt`:
```
8
```

## Type & Operator Support
Scrypt interpreter currently only supports numeric and boolean values.

operations.txt
```
(1 + 2) * 3 * (4 / 5 / (6 - 7))
13 == 14 - 1
true & false
true & 1 < 2
false | false
```
`./calc < operations.txt`
```
(((1 + 2) * 3) * ((4 / 5) / (6 - 7)))
-7.2
(13 == (14 - 1))
true
(true & false)
false
(true & (1 < 2))
true
(false | false)
false
```

List of support operations (highest precedence to lowest)
1. Parentheses (( and )).
2. Multiplication, division, and modulo (*, /, and %).
3. Addition and subtraction (+ and -).
4. Ordered comparison (<, <=, >, and >=).
5. Equality and inequality (== and !=).
6. Logical and (&).
7. Logical exclusive or (^).
8. Logical inclusive or (|).
9. Assignment (=).

## File structure
```
├── src
│   ├── ASTNode.cpp
│   ├── ASTNode.hpp
│   ├── Environment.cpp
│   ├── Environment.hpp
│   ├── Exception.cpp
│   ├── Exception.hpp
│   ├── Infix.cpp
│   ├── Infix.hpp
│   ├── Lexer.cpp
│   ├── Lexer.hpp
│   ├── Scrypt.cpp
│   ├── Scrypt.hpp
├── public
│   ├── lex.cpp
│   ├── format.cpp
│   ├── calc.cpp
│   ├── scrypt.cpp
├── README.md
```
