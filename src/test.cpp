#include <signal.h>

#include <fstream>
#include <iostream>
#include <string>

using namespace std;

int main() {
  while (true) {
    string input;
    cin >> input;

    if (input == "quit") {
      break;
    }

    // write to a file
    ofstream file;
    file.open("temp");
    file << "position startpos moves " << input << endl;
    file << "go movetime 5000" << endl;
    file.close();

    system("./main < temp > temp2");

    ifstream file2;
    file2.open("temp2");
    string output;
    while (getline(file2, output)) {
      if (output.size() > 8) {
        if (output.substr(0, 8) == "bestmove") {
          cout << "[" << output.substr(9, 4) << "]" << endl;
          break;
        }
      }
    }
    file2.close();
  }

  //   // communicate with a process
  //   int fd[2];
  //   pipe(fd);
  //   int pid = fork();
  //   if (pid == 0) {
  //     // child
  //     dup2(fd[1], STDOUT_FILENO);
  //     char *argv[] = {"./main", NULL};
  //     execv("./main", argv);
  //   } else {
  //     // parent
  //     char buffer;
  //     while (true) {
  //       string input;
  //       cin >> input;
  //       string output;
  //       write(fd[0], input.c_str(), input.size());
  //       while (read(fd[0], &buffer, 1) > 0) {
  //         cout << buffer;
  //         if (buffer != '\n')
  //           output += buffer;
  //         else if (output.size() > 8) {
  //           if (output.substr(0, 8) == "bestmove") {
  //             cout << "[" << output.substr(9, 4) << "]" << endl;
  //             break;
  //           }
  //           output = "";
  //         }
  //         buffer = 0;
  //       }
  //       cout << endl;
  //     }
  //     kill(pid, SIGKILL);
  //   }
}