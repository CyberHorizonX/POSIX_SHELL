#include <iostream>
#include <set>
#include <map>
#include <string>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <termios.h>
#include <unistd.h>
#include <algorithm>
using namespace std;
set<string> commands = {"echo", "exit", "type", "pwd", "cd"};
set<string> splitPathes()
{
  char *path = getenv("PATH");
  string p = string(path);
  string pth = "";
  set<string> pathes;
  for (int i = 0; i < p.size(); i++)
  {
    if (p[i] == ':')
    {
      pathes.insert(pth);
      pth = "";
    }
    else
      pth += p[i];
  }
  pathes.insert(pth);
  return pathes;
}
string get_path(string command, bool cQ = false, string Quote = "")
{
  set<string> pathes = splitPathes();
  for (string cmd : pathes)
  {
    string file = cmd + "/" + command;
    if (filesystem::exists(file))
    {
      string resolved_path = filesystem::canonical(file).string();
      if (cQ)
        return cmd + "/" + Quote + command + Quote;
      else
        return resolved_path;
    }
  }
  return "";
}
string get_basename(const string &path)
{
  return filesystem::path(path).filename().string();
}
bool is_exe(string command)
{
  string path = get_path(command);
  if (filesystem::exists(path))
  {
    auto perms = filesystem::status(path).permissions();
    return (perms & filesystem::perms::owner_exec) != filesystem::perms::none ||
           (perms & filesystem::perms::group_exec) != filesystem::perms::none ||
           (perms & filesystem::perms::others_exec) != filesystem::perms::none;
  }
  return false;
}
vector<string> splitArgs(string arg, char del = '\'')
{
  string part = "";
  vector<string> results;
  for (int i = 0; i < arg.size(); i++)
  {
    if (part == " " && arg[i] == ' ' && part.size() == 1)
    {
      continue;
    }
    if (arg[i] == del && (arg[i + 1] == ' ' || arg[i + 1] == del) || part == " ")
    {
      results.push_back(part);
      part = "";
    }
    if (arg[i] == del)
    {
      continue;
    }
    if (arg[i] == '\\' and del == '\"')
    {
      if (i + 1 < arg.size() && (arg[i + 1] == '$' || arg[i + 1] == '"' || arg[i + 1] == '\\'))
      {
        part += arg[i + 1];
        i++;
      }
      else
      {
        part += '\\';
      }
    }
    else
    {
      part += arg[i];
    }
  }
  results.push_back(part);
  return results;
}
vector<string> splitForSpaces(string args)
{
  vector<string> results;
  string part = "";
  for (int i = 0; i < args.size(); i++)
  {
    if (args[i] != ' ')
      part += args[i];
    else
    {
      results.push_back(part);
      part = "";
    }
  }
  results.push_back(part);
  return results;
}
// handle qouted commands
vector<string> getCommand(string input)
{
  vector<string> tokens(2);
  string command = "";
  int i = 1;
  char Quote = input[0];
  while (input[i] != Quote)
  {
    command += input[i];
    i++;
  }
  // cout << "command : " << command << endl;
  tokens[0] = command;
  i++;
  command = "";
  while (i < input.size())
  {
    command += input[i];
    i++;
  }
  // cout << "args : " << command << endl;
  tokens[1] = command;
  return tokens;
}
void Echo(string arguments, bool inFile, bool stderr_2, ofstream &out)
{
  bool cQ = (arguments[0] == '\'' || arguments[0] == '\"');
  string output = "";
  if (cQ)
  {
    vector<string> args = splitArgs(arguments, arguments[0]);
    for (auto &arg : args)
    {
      // for (int i = 0; i < arg.size(); i++)
      // {
      //   output += arg[i];
      // }
      if (inFile && stderr_2 == false)
      {
        out << arg << endl;
        // out.close();
      }
      else if (stderr_2)
      {
        try
        {
          cout << arg << endl;
        }
        catch (const std::exception &e)
        {
          out << e.what();
          // out.close();
        }
      }
      else
        cout << arg;
      // output = "";
    }
    if (!inFile)
      cout << endl;
  }
  else
  {
    // bool space = false;
    for (int i = 0; i < arguments.size(); i++)
    {
      if ((i > 0 && arguments[i] == '\\' && arguments[i - 1] == '\\') || arguments[i] != ' ' && arguments[i] != '\\')
        output += arguments[i];
      if (arguments[i] != ' ' && arguments[i + 1] == ' ')
      {
        output += " ";
      }
    }
    //  \'\"example world\"\'
    //  '"example world "'
    if (inFile)
    {
      out << output << endl;
      // out.close();
    }
    else
      cout << output << endl;
  }
}
void Type(string arguments)
{
  bool isCommand = true;
  if (commands.find(arguments) != commands.end())
  {
    cout << arguments << " is a shell builtin\n";
    isCommand = false;
  }
  else
  {
    string path = get_path(arguments);
    if (path != "")
    {
      cout << arguments << " is " << path << endl;
      isCommand = false;
    }
  }
  if (isCommand)
    cout << arguments << ": not found\n";
}
void Cd(string arguments)
{
  try
  {
    if (arguments.empty() || arguments == "~")
    {
      char *home = getenv("HOME");
      if (home)
      {
        filesystem::current_path(home);
      }
      else
      {
        cerr << "cd: HOME not set" << endl;
      }
    }
    else if (filesystem::exists(arguments) && filesystem::is_directory(arguments))
    {
      filesystem::current_path(arguments);
    }
    else
    {
      cerr << "cd: " << arguments << ": No such file or directory" << endl;
    }
  }
  catch (const filesystem::filesystem_error &e)
  {
    cerr << "cd: " << arguments << ": No such file or directory" << endl;
  }
}
void Cat(string arguments, bool inFile, bool stderr_2, ofstream &out)
{
  bool cQ = (arguments[0] == '\'' || arguments[0] == '\"');
  vector<string> files;
  if (cQ)
    files = splitArgs(arguments, arguments[0]);
  else
    files = splitForSpaces(arguments);
  bool all_exists = true;
  bool firstFile = true;
  fstream fileOut;
  string line;
  if (inFile)
  {
    for (const auto &file : files)
    {
      if (file == " ")
        continue;
      fileOut.open(file);
      if (!fileOut.is_open())
      {
        if (!stderr_2)
        {
          cerr << "cat: " << file << ": No such file or directory" << endl;
          all_exists = false;
          break;
        }
        else
        {
          out << "cat: " << file << ": No such file or directory" << endl;
        }
        continue;
      }
      while (getline(fileOut, line))
      {
        if (!stderr_2)
        {
          out << line;
        }
        else
          cout << line << endl;
      }
      fileOut.close();
      firstFile = false;
      // cout << "output : " << output << endl;
    }
    // cout << endl;
    // break;
  }
  if (!inFile)
  {
    for (const auto &file : files)
    {
      if (file == " ")
        continue;
      fileOut.open(file);
      if (!fileOut.is_open())
      {
        cerr << "cat: " << file << ": No such file or directory" << endl;
        continue;
      }
      while (getline(fileOut, line))
      {
        if (!cQ)
        {
          cout << line << endl;
        }
        else
          cout << line;
      }
      fileOut.close();
      fileOut.clear();
    }
  }
  if (cQ)
    cout << endl;
}
void QuotedCommands(string command, string arguments, string input)
{
  try
  {
    string resolvedPath = get_path(command, true, string(1, input[0]));
    string fullExe = resolvedPath + " " + arguments;
    int result = system(fullExe.c_str());
    if (result != 0)
    {
      cerr << "Error: Command execution failed." << endl;
    }
  }
  catch (const filesystem::filesystem_error &e)
  {
    cerr << "Error: " << e.what() << endl;
  }
}
void ExecCommand(string command, string arguments, bool inFile, bool stderr_2, ofstream &out)
{
  string fullExe = get_basename(get_path(command)) + " " + arguments;
  if (inFile)
  {
    string cmdWithRedirect = fullExe + " 2>&1";
    FILE *pipe = popen(stderr_2 ? cmdWithRedirect.c_str() : fullExe.c_str(), "r");
    if (!pipe)
    {
      cerr << "Error: Failed to execute command: " << fullExe << endl;
      // out.close();
      return;
    }
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
      out << buffer;
    }
    pclose(pipe);
    // out.close();
  }
  else
  {
    system(fullExe.c_str());
  }
}
vector<string> completeCustomCommand(string customCommand)
{
  vector<string> commands;
  set<string> pathes = splitPathes();
  for (auto directory : pathes)
  {
    std::filesystem::path path(directory);
    if (filesystem::exists(path) && filesystem::is_directory(path))
    {
      for (auto &cmd : filesystem::directory_iterator(path))
      {
        string filename = cmd.path().filename().string();
        if (filename.find(customCommand) == 0)
        {
          commands.push_back(filename);
        }
      }
    }
  }
  return commands;
}
string getMaxOccurrenceKey(string command, const std::map<string, int> &occurrences)
{
  if (occurrences.empty())
  {
    throw std::runtime_error("Map is empty");
  }
  string maxKey = "";
  int maxValue = 0;
  for (const auto &[key, value] : occurrences)
  {
    if (value > maxValue || (value == maxValue && key.size() > maxKey.size()))
    {
      maxValue = value;
      maxKey = key;
    }
  }
  return maxKey;
}
int completeCommand(string &command, vector<string> &customs)
{
  for (auto cmd : commands)
  {
    if (cmd.find(command) != string::npos)
    {
      cout << "\r$ " << cmd << " ";
      command = cmd;
      return 1;
    }
  }
  customs = completeCustomCommand(command);
  if (!customs.empty())
    return 2;
  return 0;
}
string partialComplete(string &command, vector<string> &customs)
{
  string common = command;
  map<string, int> countCommon;
  for (auto cmd : customs)
  {
    if (cmd.rfind(common, 0) == 0)
    {
      string prefix = common;
      for (int i = common.size(); i < cmd.size(); i++)
      {
        prefix += cmd[i];
        countCommon[prefix]++;
        // cout<<prefix<<" " << countCommon[prefix]<<endl;
      }
    }
  }
  return getMaxOccurrenceKey(command, countCommon);
}
void enableRawMode()
{
  termios term;
  tcgetattr(STDIN_FILENO, &term);
  term.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &term);
}
void disableRawMode()
{
  termios term;
  tcgetattr(STDIN_FILENO, &term);
  term.c_lflag |= (ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &term);
}
bool isDoubleTabPressed()
{
  struct timeval timeout;
  timeout.tv_sec = 0;       // No seconds, just milliseconds
  timeout.tv_usec = 200000; // 200ms timeout
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  int result = select(STDIN_FILENO + 1, &fds, NULL, NULL, &timeout);
  if (result > 0)
  {
    char nextChar;
    read(STDIN_FILENO, &nextChar, 1);
    return nextChar == '\t'; // Return true if second TAB is detected
  }
  return false; // No second TAB detected
}
void readInputWithTabSupport(string &input)
{
  enableRawMode();
  int countTab = 0;
  char c;
  while (true)
  {
    vector<string> customs;
    c = getchar();
    if (c == '\n')
    {
      std::cout << std::endl;
      break;
    }
    else if (c == '\t')
    {
      if (isDoubleTabPressed())
      {
        countTab = 2; // Double TAB detected
      }
      else
      {
        countTab = 1; // Only a single TAB was pressed
      }
      int complete = completeCommand(input, customs);
      sort(customs.begin(), customs.end());
      if (countTab == 1)
      {
        if (complete == 1)
        {
          completeCommand(input, customs);
          input += " ";
        }
        else if (complete == 2)
        {
          if (customs.size() == 1)
          {
            cout << "\r$ " << customs.front() << " ";
            input = customs.front();
            input += " ";
          }
          else
          {
            string part = partialComplete(input, customs);
            input = part;
            cout << "\r$ " << input;
          }
        }
        else
          cout << "\a";
        countTab = 0;
      }
      else if (countTab == 2 && !customs.empty())
      {
        cout << endl;
        cout << "\a";
        for (auto custom : customs)
          cout << custom << "  ";
        cout << endl;
        cout << "$ " << input;
        countTab = 0;
      }
      else
        cout << "\a";
    }
    else if (c == 127)
    {
      if (!input.empty())
      {
        input.pop_back();
        std::cout << "\b \b";
      }
    }
    // else if (c == 32)
    // {
    //   cout<<"space"<<endl;
    //   input += " ";
    // }
    else
    {
      input += c;
      std::cout << c;
    }
    // if (!customs.empty())
    // {
    //   for (auto custom : customs)
    //     cout << "hhhhh " << custom << endl;
    // }
  }
  disableRawMode();
}
int main()
{
  cout << unitbuf;
  cerr << unitbuf;
  string input;
  while (true)
  {
    cout << "$ ";
    // getline(std::cin, input);
    readInputWithTabSupport(input);
    // cout << "input : " << input << endl;
    if (input == "exit 0")
      break;
    bool cQ = (input[0] == '\'' || input[0] == '\"');
    vector<string> tokens = cQ ? getCommand(input) : vector<string>();
    string command = cQ ? tokens[0] : input.substr(0, input.find(" "));
    string arguments = cQ ? tokens[1] : input.substr(input.find(" ") + 1);
    string outPutFile;
    bool inFile = false;
    size_t redirPos = arguments.find(">");
    bool stderr_2 = arguments[redirPos - 1] == '2';
    bool append = arguments[redirPos + 1] == '>';
    if (redirPos != string::npos)
    {
      outPutFile = append ? arguments.substr(redirPos + 2) : arguments.substr(redirPos + 1);
      size_t firstChar = outPutFile.find_first_not_of(" ");
      size_t lastChar = outPutFile.find_last_not_of(" ");
      outPutFile = outPutFile.substr(firstChar, lastChar - firstChar + 1);
      if (arguments[redirPos - 1] == '1' || arguments[redirPos - 1] == '2')
        arguments = arguments.substr(0, redirPos - 2);
      else
        arguments = arguments.substr(0, redirPos - 1);
      inFile = true;
    }
    ofstream out(outPutFile, append ? ios::app : ios::out);
    if (command == "echo")
      Echo(arguments, inFile, stderr_2, out);
    else if (command == "type")
      Type(arguments);
    else if (command == "pwd")
      cout << filesystem::current_path().string() << endl;
    else if (command == "cd")
      Cd(arguments);
    else if (command == "cat")
      Cat(arguments, inFile, stderr_2, out);
    else if (input[0] == '\'' || input[0] == '\"')
      QuotedCommands(command, arguments, input);
    else if (is_exe(command))
      ExecCommand(command, arguments, inFile, stderr_2, out);
    else
      cout << input << ": command not found\n";
    input = "";
  }
}



