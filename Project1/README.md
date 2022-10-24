# CZ4031

To compile and run C++ on VSCode


- Follow ths link to setup and install https://code.visualstudio.com/docs/languages/cpp

- After install, define the `tasks.json` file with corresponding `args` to include all C++ files:

  ```
  "tasks": [
    {
      "args": ["-g", "${workspaceFolder}\\*.cpp", "-o", "${fileDirname}\\${fileBasenameNoExtension}.exe"],
    }
  ]

- `cd` to `main.cpp` files's directory and compile the executable.
