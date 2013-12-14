# bitmapdd

*Creates a bitmap from a file (or device). It’s mainly used for creating a usage map of input but it can also do conversions.*

Examples:

| Blocksize | Input (hexadecimal)     | Output (binary) |
|:---------:|:-----------------------:|:----------------|
| 1         | 00 31 00 40 A0 FF 02 00 | 01011110        |
| 1*        | 30 31 30 30 31 30 30 31 | 01001001        |
| 2         | 16 00 72 B0 00 00 00 00 | 1100            |
| 4         | 00 00 00 00 00 A3 00 F7 | 01              |

\* With an additional "-null 48" parameter (48 in decimal is 30 in hexadecimal).

## Usage

```bitmapdd [options]```

**Options:**

* **--null NUM**
Character code of null (default: 0).
For example you should set this to 48 when you need to convert a text file consists of 0/1 characters to a binary file where each character is mapped to a bit.
* **--bs NUM**
Block size of input file (default: 512).
If all bytes of a block is 0 than it will mapped to 0 bit, else it will mapped to 1 bit.
* **--count NUM**
Count of blocks to process (default: till end of input).
* **--if FILE**
Specifies the input file (default: stdin).
* **--of FILE**
Specifies the output file (default: stdout)
* **--help**
Prints usage information.
* **--version**
Prints version information.

Program supports stdout/stdin redirections, so the following should work:
``` cat /dev/sda | bitmapdd > output.dat```

You can get progress information by sending a SIGUSR1 signal to the process:
``` kill -SIGUSR1 pid ``` (where pid is the process identifier)

### Scenerio 1

You have a file which consists of blocks of data. If a block is full of zeros than it’s free, otherwise it’s used. You want to make a bitmap from it where a bit in the file is 0 if the corresponding block is zero, otherwise it’s 1.

For example with block size set to 4:

```bitmapdd --bs 4 --if input.dat --of output.dat```

```
00 00 00 00 | 34 00 56 00 | 00 00 00 00 | F3 11 23 00
         \            |       |            /
           \          |       |          /
             \        |       |        /
               0      1       0      1

```

### Scenerio 2

Converting a text of zeros and ones to a binary file where every bit corresponds to one character in the original file.

```bitmapdd --bs 1 --null 48 --if usagemap.txt --of usagemap.dat```

Note: null byte has been set to 48 which is the code of character “0″ in the ASCII character table. 

usagemap.txt contains text:
```
001100000100000001000001
```

usagemap.dat will contain text:
```
0@A
```

| Character | Decimal code | Binary code |
|:---------:|:------------:|:-----------:|
| 0         | 48           | 00110000    |
| @         | 64           | 01000000    |
| A         | 65           | 01000001    |

## Build
Program should build on any UNIX like operation system with a standard C compiler and make utility.

To compile:
```make```

To compile without signal handling:
```make CFLAGS='-DNOSIGNAL'```

To run:
```./bin/bitmapdd```
(however it is not recommended to call this way because it will wait for input from stdin)

To clean:
```make clean```

## Author
Written by Andras Majdan.
License: GNU General Public License Version 3
Report bugs to <majdan.andras@gmail.com>

## See also
[bitmap2pbm](https://github.com/andmaj/bitmap2pbm/blob/master/README.md),  [fat2bitmap](https://github.com/andmaj/fat2bitmap/blob/master/README.md)
