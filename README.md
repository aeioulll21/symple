# Custom Python Syntax Converter

This repository lets you write custom Python-like syntax in `funcs.lang` and convert it to real Python using `main.c`.

- Edit `funcs.lang` to write your code using `func` and custom print rules.
- Edit `rules.conf` to define how your syntax should be converted.
- Compile with `gcc main.c -o funcs` and run with `./funcs`.