# Brainfuck Compiled [Embedded DSL](https://en.wikipedia.org/wiki/Domain-specific_language#External_and_Embedded_Domain_Specific_Languages) in C++

"Consteval is a procedural macro system."

This program takes a compile-time constant string, containing a Brainfuck program, and generates a C++ function *at compile time* that is functionally equivalent to it.

The general approach is to recursively generate lambda functions that each perform one Brainfuck operation, and perform recursive calls to generated functions that implement the rest of the program.

The result is agonisingly slow to compile, and impossible to debug, and only works properly for non-trivial programs at `-O3` for some reason (probably because at that point it can eliminate the recursive calls). At `-O3`, it can correctly handle some more complex examples I found online, including tic-tac-toe and Conway's game of life.

As a party trick, at `-O3`, GCC can evaluate most of the included hello world program at compile time ([Compiler Explorer](https://godbolt.org/z/1PMPYYq4M)), reducing to just a series of calls to `putc`:

```assembly
 ...
 mov    rsi,QWORD PTR [rip+0x2ead]        # 404018 <stdout@GLIBC_2.2.5>
 mov    edi,0x48                          # 'H'
 call   401030 <putc@plt>
 mov    rsi,QWORD PTR [rip+0x2e9c]        # 404018 <stdout@GLIBC_2.2.5>
 mov    edi,0x65                          # 'e'
 ...
```

The fun "embedded string literal" interface is only possible as of C++23; I used a compile-time constant string class I found online that relies on some obscure template stuff that [perhaps only C++ committee members can understand](https://www.quora.com/How-do-you-pass-a-string-literal-as-a-parameter-to-a-C-template-class).
