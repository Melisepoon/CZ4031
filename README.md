# CZ4031

To compile and run C++ on VSCode

- Can follow ths link to install https://code.visualstudio.com/docs/languages/cpp

- After install, define the `tasks.json` file with corresponding `args` to include all C++ files:

  ```
  "tasks": [
    {
      "args": ["-g", "${workspaceFolder}\\*.cpp", "-o", "${fileDirname}\\${fileBasenameNoExtension}.exe"],
    }
  ]

- `cd` to `main.cpp` under the `src` folder and compile the executable.

1) Install MinGW http://bit.ly/mingw10

2) Unzip and install MinGW in the C:/ folder

3) Go to This PC, open MinGW and copy path of MinGW/bin

4) Select properties in ThisPC -> Advanced System Settings -> Environment Variables -> 
Path -> Edit -> New -> then paste the path of MinGW there

5) Download Code Runner extension in VSCode

6) Code Runner: To run code, can press Ctrl+Alt+N, to stop press Ctrl+Alt+M

7) Install C/C++ in VSCode
