# Vshell
A very basic Shell proof of concept in C. Use with caution, everything isn't meticulously tested.<br>
### Supported internal commands  
- `fg`
- `bg`
- `stop`
- `jobs`
- `exit`
- `ctrl + c` / `ctrl + z`
### Additional features
- `|` (pipes)
- `<`/`>` operators 
- `&` (background jobs)

## Compilation & execution
Pretty straightforward ~~because there's no makefile~~
<br>
Edit : there is one now
- using make
    ```sh
    cd Vshell
    make
    ./vshell
    ```
- the old way (using a standard compiler such as gcc)
    ```sh
    cd Vshell
    gcc *.c -o vshell
    ./vshell
    ```
