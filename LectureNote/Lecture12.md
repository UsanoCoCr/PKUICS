# Linking
## Why Linkers
- Efficiency
    - time: seperate compilation
    - space: libraries
- Modularity

## What do Linkers do?
### Compiler Driver
- Gcc is the compiler driver in compilation toolchain
- Gcc invokes seceral other compilation phases
    - cpp, the Preprocessor
    - cc1, the Compiler
    - as/gas, the Assembler
    - ld, the Linker

#### Preprocessor
- first, gcc compiler driver invoke cpp to generate expanded source
    - preprocessor just does text substitution/ gcc with option -E
    - convert the C source file to another C source file
    - expands "#" directives
- Directives of Preprocessor   
    - included files: #include
    - define constants: #define
    - define macros: #define MIN(x,y) ((x)<(y)?(x):(y))
    - conditional compilation: #ifdef, #ifndef, #if, #else, #elif, #endif
    - control your optimization flags: #pragma optimize("-fno-inline")
    - line control: #line

#### Assembler
- Furthermore, gcc invokes gas to generate object code
    - translates assembly code into binary object code

#### Linking
- programs are translated and linked using a compiler driver
    ```
    linux> gcc -c -o foo.o foo.c
    ```

### Step1: Symbol resolution
- during symbol resolution step, the linker associates each symbol reference with exactly one symbol definition

### Step2: Relocation
- merge separate code and data sections into single sections
- relocates symbols from their relative locations in the .o files to their final absolute memory locations in the executable
- updates all references to these symbols to reflect their new positions

### three kinds of object files
- relocatable object file(.o file)
    - contains code and data ina form that can be combined with other relocatable object files at compile time to create an executable object file
    - each .o file is produced from exactly one .c file
- executable object file(a.out)
    - contains code and data in a form that can be copied directly into memory and executed
    - contains header information that the kernel user to load the program
- shared object file(.so file)
    - special type of relocatable object file that can be loaded into memory and linked dynamically

### ELF object file formate
- ELF header
    - identifies the file as an ELF object file
    - contains information that the kernel needs to load the file
- segment headers
    - describe the program and data layout of the object file
    - page size, virtual address memory segments, segment sizes
- text section
- .rodata section
    - read only data: jumpt tables, constant strings
- .data section
    - global and static variables
- .bss section
    - uninitialized global variables
    - has section header but no place in the file
- .symtab section
    - symbol table
    - contains information about every defined and referenced symbol
    - section names and locations
- .rel .text section
    - relocation information for the .text section
    - instructions for modifying
    - address of instructions that will need to be modified in the executable
- .rel .data section
    - relocation information for the .data section
    - Addresses of pointer data that will need to be modified in the merged executable
- debug section
    - info for debugging
- section header table