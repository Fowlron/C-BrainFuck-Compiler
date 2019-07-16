# C-BrainFuck-Compiler
CBFC -- C BrainFuck Compiler. Compiles BrainFuck code to a decently optimized c source file, then compiles that source file.

# Instalation
cd into the directory with the cbfc source file and run
`gcc cbfc.c -o cbfc`
Optionally, add the resulting cbfc file to your path by adding it to a binaries folder you might have added to your path previously.

# Execution
Just run the file with the input brainfuck file as an argument, and the file will be compiled to a.out and a.out.c (the executable and the c source, respectively).
By using the optional arguments, you can change the output file name, whether to remove the c source file (leaving you only with the input and output files), whether to use gcc or tcc (for extremely long brainfuck files, the c source might come out as too long for gcc, in which case tcc should be used), the tapesize to be used, and get extra information in verbose mode.
Verbose mode will report the current depth of the program (1 meaning no loops, 2 meaning 1 while loop, 3 meaning 2 while loops, so on and so forth). The code length is how much more code is left until the end of the file, not until the end of the current loop!

For more information, run `cbfc --help`.
