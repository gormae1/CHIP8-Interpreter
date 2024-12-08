# CHIP8-Interpreter
Graphical Interpreter of the CHIP8 Language in SDL. Requires libsdl2.

Example:
| PONG ROM  | IBM Logo ROM |
| ------------- | ------------- |
| ![Screenshot of Usage with Pong ROM](https://raw.githubusercontent.com/gormae1/CHIP8-Interpreter/refs/heads/main/pong_test_ROM.png?token=GHSAT0AAAAAAC3T7RUURDS6ZNMFNXXNDCRYZ2WBA2Q)  | ![Screenshot of Usage with IBM ROM](https://raw.githubusercontent.com/gormae1/CHIP8-Interpreter/refs/heads/main/IBM_test_full.png?token=GHSAT0AAAAAAC3T7RUUSSI7APFVCZP3ZLA4Z2WBCDA) |

# Usage
Compile with Make and run the target executable `c8emu`. Once prompted, enter the path to the input ROM via stdin.

Example:
```
cd <dir of project>
make
./c8emu
rom path:<rom path>
```
