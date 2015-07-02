# Linked List-based Malloc

## How it works

This is a simple implementation of `malloc()` and `free()`. It uses a first 
fit approach, backed by a linked list. That is, whenever `free()` is called, 
it stores the newly freed memory fragments in a linked list sorted by memory 
address and merges fragments which are adjacent in memory. Then when
`malloc()` is called, it first scans this list for an available memory
fragment. If none is found, it issues a request to the kernel to allocate
another piece of memory for it.

## Building and running

The static object (.so) library can be built simply by running

    make

There are no external dependencies. Running `make` produces `libmalloc.so`,
which can be used in another process by executing e.g.

    env LD_PRELOAD="./libmalloc.so" ls

Replace `ls` with the command you want to run. Alternately, you can use your
`libmalloc.so` for everything by executing

    export LD_PRELOAD="/path/to/libmalloc.so"

To switch back to the standard library, issue

    export LD_PRELOAD=""

## Testing

A test script, `test.sh`, has been provided. This script simply runs a unit
test with the custom malloc library preloaded, as well as `ls --color=auto`.
The script can be easily modified to test other commands.

## License

Copyright (C) 2012 Bryan Cuccioli

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
