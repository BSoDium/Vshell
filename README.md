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

## Compilation & execution
Pretty straightforward because there's no makefile :
```sh
cd Vshell
gcc *.c
./minishell
```
