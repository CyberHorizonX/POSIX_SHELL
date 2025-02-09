# Code Documentation

## Overview
This program implements a simple shell-like command line interface in C++ that supports a subset of common shell commands, including `echo`, `exit`, `type`, `pwd`, `cd`, and `cat`. It allows for command execution, file manipulation, and completion of commands using tab support. The functionality is built using standard libraries such as `<iostream>`, `<filesystem>`, `<map>`, and `<set>`.

## Constants
```cpp
set<string> commands = {"echo", "exit", "type", "pwd", "cd"};
```
This set contains the built-in commands supported by the shell.

## Functions

### `set<string> splitPathes()`
- **Purpose**: Retrieves the directories listed in the `PATH` environment variable and splits them into a set of strings.
- **Returns**: A set of strings representing the directories in the `PATH`.

### `string get_path(string command, bool cQ = false, string Quote = "")`
- **Parameters**:
  - `command`: The command for which the path needs to be found.
  - `cQ`: A boolean indicating if quotes are involved (default is `false`).
  - `Quote`: The quote character if applicable.
- **Returns**: The resolved path of the command if it exists, otherwise an empty string.

### `string get_basename(const string &path)`
- **Parameters**: 
  - `path`: A string representing a file path.
- **Returns**: The base name (filename) of the provided path.

### `bool is_exe(string command)`
- **Parameters**: 
  - `command`: The command to check.
- **Returns**: `true` if the command is an executable file, otherwise `false`.

### `vector<string> splitArgs(string arg, char del = '\'')`
- **Parameters**:
  - `arg`: The string to split into arguments.
  - `del`: The delimiter character used for splitting (default is single quote).
- **Returns**: A vector of strings containing the split arguments.

### `vector<string> splitForSpaces(string args)`
- **Parameters**: 
  - `args`: The string to split by spaces.
- **Returns**: A vector of strings split by spaces.

### `vector<string> getCommand(string input)`
- **Parameters**: 
  - `input`: A string containing the command with arguments.
- **Returns**: A vector of two strings: the command and its arguments.

### `void Echo(string arguments, bool inFile, bool stderr_2, ofstream &out)`
- **Parameters**:
  - `arguments`: The arguments to be echoed.
  - `inFile`: Indicates if the output should be redirected to a file.
  - `stderr_2`: A flag to determine if the output should go to standard error.
  - `out`: The output file stream to write to if `inFile` is `true`.
- **Functionality**: Echoes the provided arguments either to standard output or a specified file.

### `void Type(string arguments)`
- **Parameters**: 
  - `arguments`: The command to be typed out.
- **Functionality**: Checks if the command is a built-in or is found in the file system and prints its type.

### `void Cd(string arguments)`
- **Parameters**: 
  - `arguments`: The directory to change to.
- **Functionality**: Changes the current working directory to the specified path.

### `void Cat(string arguments, bool inFile, bool stderr_2, ofstream &out)`
- **Parameters**: 
  - `arguments`: The names of the files to concatenate and display.
  - `inFile`: Indicates if the output should be redirected to a file.
  - `stderr_2`: A flag to determine if the output should go to standard error.
  - `out`: The output file stream to write to if `inFile` is `true`.
- **Functionality**: Reads the contents of specified files and outputs them to the console or a file.

### `void QuotedCommands(string command, string arguments, string input)`
- **Parameters**: 
  - `command`: The command to execute.
  - `arguments`: The arguments for the command.
  - `input`: The full input string containing quotes.
- **Functionality**: Executes commands that are enclosed in quotes.

### `void ExecCommand(string command, string arguments, bool inFile, bool stderr_2, ofstream &out)`
- **Parameters**: 
  - `command`: The command to execute.
  - `arguments`: The arguments to pass to the command.
  - `inFile`: Indicates if the output should be redirected to a file.
  - `stderr_2`: A flag indicating if standard error should be redirected.
  - `out`: The output file stream to write to if `inFile` is `true`.
- **Functionality**: Executes an external command with optional redirection of output.

### `vector<string> completeCustomCommand(string customCommand)`
- **Parameters**: 
  - `customCommand`: A string representing part of a command.
- **Returns**: A vector of possible command completions based on `customCommand`.

### `string getMaxOccurrenceKey(string command, const std::map<string, int> &occurrences)`
- **Parameters**: 
  - `command`: The base command for which to find the most common completion.
  - `occurrences`: A map of command completions and their occurrence counts.
- **Returns**: The key with the maximum occurrence from the map.

### `int completeCommand(string &command, vector<string> &customs)`
- **Parameters**: 
  - `command`: A reference to the command being completed.
  - `customs`: A vector to store custom completions.
- **Returns**: An integer indicating the completion status (0: no match, 1: matched built-in command, 2: matched custom commands).

### `string partialComplete(string &command, vector<string> &customs)`
- **Parameters**: 
  - `command`: A reference to the command being completed.
  - `customs`: A vector of custom completions.
- **Returns**: A string representing the common prefix of the matched commands.

### `void enableRawMode()`
- **Purpose**: Configures the terminal to raw mode to read user input without line buffering.

### `void disableRawMode()`
- **Purpose**: Restores the terminal to its normal mode.

### `bool isDoubleTabPressed()`
- **Returns**: `true` if a double TAB is detected, otherwise `false`.

### `void readInputWithTabSupport(string &input)`
- **Parameters**: 
  - `input`: A reference to the command input string.
- **Functionality**: Handles user input with support for tab completion, backspace, and command execution.

### `int main()`
- **Functionality**: The main loop of the application where user input is read, commands are parsed, and appropriate functions are called to execute commands or handle input.

## Conclusion
This C++ program provides a simple command line interface that mimics basic shell behavior. It supports command execution, input processing with tab completion, and file manipulation. The modular design enables easy extension for additional commands or functionalities.