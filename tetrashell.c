// I, Saman Sahebi (730403605), pledge that I have neither given nor received
// unauthorized aid on this assignment. Collaborators: Jean Cheng

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "tetris.h"

// Returns 1 if everything in the given game state looks OK, and 0 if not.
static int SanityCheckState(TetrisGameState* s) {
  int row, col, tmp;
  char c;
  tmp = s->location_x;
  if ((tmp < 0) || (tmp >= BLOCKS_WIDE)) return 0;
  tmp = s->location_y;
  if ((tmp < PIECE_START_Y) || (tmp >= BLOCKS_TALL)) return 0;

  // Important check: make sure the current piece is a valid piece ID.
  tmp = sizeof(tetris_pieces) / sizeof(const char*);
  if ((s->current_piece < 0) || (s->current_piece >= tmp)) return 0;
  if ((s->next_piece < 0) || (s->next_piece >= tmp)) return 0;

  // Make sure the board contains no invalid characters.
  for (row = 0; row < BLOCKS_TALL; row++) {
    for (col = 0; col < BLOCKS_WIDE; col++) {
      c = s->board[row * BLOCKS_WIDE + col];
      if ((c < ' ') || (c > '~')) return 0;
    }
  }
  return 1;
}

// resets foreground and background text color
static void resetColor(char color) {
  if (color != 0) {
    printf("\x1b[39m\x1b[49m");
  }
}

int main(int argc, char** argv) {
  if (argc != 1) {
    fprintf(stderr, "Do not use extra arguments.\n");
    return EINVAL;
  }

  // small variable for whether or not the console supports color
  short color;
  if (strstr(getenv("TERM"), "color") != NULL) {
    color = 1;
    // welcome message
    printf(
        "\x1b[38;5;15m\x1b[48;5;235m\n"
        "Welcome to...\n\n"
        "(\\___/)\n"
        "(='\x1b[38;5;219m.\x1b[38;5;15m'=)          (\\__/)                "
        "/)_/)\n"
        "(\x1b[38;5;219m\"\x1b[38;5;15m)_(\x1b[38;5;219m\"\x1b[38;5;15m)      "
        "\x1b[38;5;39m_\x1b[38;5;15m   ( . .)      \x1b[38;5;39m___  "
        "_\x1b[38;5;15m   (-\x1b[38;5;219m.\x1b[38;5;15m- ) \x1b[38;5;39m_   "
        "_\n"
        "\x1b[38;5;111m|_   _| ___ | |_ "
        "\x1b[38;5;15mC(\x1b[38;5;219m\"\x1b[38;5;15m)(\x1b[38;5;219m\"\x1b[38;"
        "5;15m)\x1b[38;5;111m_ _ / __|| "
        "|\x1b[38;5;15m(\x1b[38;5;219m\"\x1b[38;5;15m)(\x1b[38;5;219m\"\x1b[38;"
        "5;15m)_o\x1b[38;5;111m| | | |\n"
        "\x1b[38;5;183m  | |  / -_)|  _|| '_|/ _` |\\__ \\| ' \\ / -_)| | | |\n"
        "\x1b[38;5;219m  |_|  \\___| \\__||_|  \\__,_||___/|_||_|\\___||_| "
        "|_|\n\n"
        "\x1b[38;5;15mthe ultimate Tetris quicksave hacking tool!\n"
        "Enter the path to the quicksave you'd like to begin hacking: ");
  } else {
    color = 0;
    printf(
        "Welcome to...\n\n"
        "(\\___/)\n"
        "(='.'=)          (\\__/)                /)_/)\n"
        "(\")_(\")      _   ( . .)      ___  _   (-.- ) _   _\n"
        "|_   _| ___ | |_ C(\")(\")_ _ / __|| |(\")(\")_o| | | |\n"
        "  | |  / -_)|  _|| '_|/ _` |\\__ \\| ' \\ / -_)| | | |\n"
        "  |_|  \\___| \\__||_|  \\__,_||___/|_||_|\\___||_| |_|\n\n"
        "the ultimate Tetris quicksave hacking tool!\n"
        "Enter the path to the quicksave you'd like to begin hacking: ");
  }

  char path[PATH_MAX + 1];
  // checking the user-entered quicksave path
  while (1) {
    if (fgets(path, PATH_MAX + 1, stdin) != NULL) {
      if (path[strlen(path) - 1] == '\n') {
        path[strlen(path) - 1] = '\0';
      }
      if (strcmp(path, "e") == 0 || strcmp(path, "ex") == 0 ||
          strcmp(path, "exi") == 0 || strcmp(path, "exit") == 0) {
        resetColor(color);
        return 0;
      }

      FILE* fp = fopen(path, "rb");
      if (fp != NULL) {
        TetrisGameState save;
        size_t bytes = fread(&save, 1, sizeof(TetrisGameState), fp);
        fclose(fp);
        if (bytes == sizeof(TetrisGameState) && SanityCheckState(&save) == 1) {
          break;
        }
      }
    }
    fprintf(stderr, "Invalid quicksave path. Try again: ");
  }
  printf(
      "\'%s\' set as the current quicksave.\n"
      "Enter your command below to get started.\n",
      path);

  unsigned score_print;
  unsigned lines_print;
  char* login = getlogin();
  // command loop
  while (1) {
    // update score/lines
    FILE* currentfp = fopen(path, "rb");
    if (currentfp == NULL) {
      resetColor(color);
      error(errno, errno, "Error opening quicksave");
    } else {
      // temp currentSave
      TetrisGameState currentSave;
      size_t bytes = fread(&currentSave, 1, sizeof(TetrisGameState), currentfp);
      fclose(currentfp);
      if (bytes != sizeof(TetrisGameState) ||
          SanityCheckState(&currentSave) == 0) {
        resetColor(color);
        error(errno, errno, "Error reading file");
      }
      score_print = currentSave.score;
      lines_print = currentSave.lines;
    }
    printf("\x1b[38;5;15m%s@TShell[%c%c%c%c...][%u/%u]> ", login, path[0],
           path[1], path[2], path[3], score_print, lines_print);

    // now color is used to loop through text colors
    if (color != 0) {
      switch (color) {
        case 1:
          printf("\x1b[38;5;39m");
          break;
        case 2:
          printf("\x1b[38;5;75m");
          break;
        case 3:
        case 9:
          printf("\x1b[38;5;111m");
          break;
        case 4:
        case 8:
          printf("\x1b[38;5;147m");
          break;
        case 5:
        case 7:
          printf("\x1b[38;5;183m");
          break;
        case 6:
          printf("\x1b[38;5;219m");
          break;
        case 10:
          printf("\x1b[38;5;75m");
          color = 0;
          break;
      }
      color++;
    }

    // longest command is "recover <path>"
    char strbuf[PATH_MAX + 9];
    if (fgets(strbuf, PATH_MAX + 9, stdin) != NULL) {
      if (strbuf[strlen(strbuf) - 1] == '\n') {
        strbuf[strlen(strbuf) - 1] = '\0';
      }
      // 5 arguments max because max arguments needed is 3, so an extra 1 in
      // case too many arguments are used
      char* args[5] = {NULL, NULL, NULL, NULL, NULL};
      short argCount = 0;
      char* word = strtok(strbuf, " ");
      while (word != NULL && argCount < 4) {
        // makes the arguments lowercase for easier comparison
        for (int i = 0; i < strlen(word); i++) {
          word[i] = tolower(word[i]);
        }
        args[argCount] = word;
        argCount++;
        word = strtok(NULL, " ");
      }

      // checking which command
      if (argCount == 0) {
        fprintf(stderr,
                "Possible commands: modify, rank, check, recover, switch, "
                "visualize, help, exit\n");
      } else if (strcmp(args[0], "m") == 0 || strcmp(args[0], "mo") == 0 ||
                 strcmp(args[0], "mod") == 0 || strcmp(args[0], "modi") == 0 ||
                 strcmp(args[0], "modif") == 0 ||
                 strcmp(args[0], "modify") == 0) {
        // doing all the argument validation in TetraShell instead of the child
        // program to provide the most accurate error messages
        if (argCount != 3) {
          fprintf(stderr, "Usage: modify <field> <number>\n");
          continue;
        }
        char* end;
        unsigned long num = strtoul(args[2], &end, 10);
        if (strcmp(args[1], "s") == 0 || strcmp(args[1], "sc") == 0 ||
            strcmp(args[1], "sco") == 0 || strcmp(args[1], "scor") == 0 ||
            strcmp(args[1], "score") == 0) {
          args[1] = "score";
          // unsigned long, so also checks for negatives
          if (num > UINT_MAX) {
            fprintf(stderr, "Number is not within the valid range.\n");
            continue;
          }
        } else if (strcmp(args[1], "l") == 0 || strcmp(args[1], "li") == 0 ||
                   strcmp(args[1], "lin") == 0 ||
                   strcmp(args[1], "line") == 0 ||
                   strcmp(args[1], "lines") == 0) {
          args[1] = "lines";
          if (num > UINT_MAX) {
            fprintf(stderr, "Number is not within the valid range.\n");
            continue;
          }
        } else if (strcmp(args[1], "n") == 0 || strcmp(args[1], "ne") == 0 ||
                   strcmp(args[1], "nex") == 0 ||
                   strcmp(args[1], "next") == 0 ||
                   strcmp(args[1], "next_") == 0 ||
                   strcmp(args[1], "next_p") == 0 ||
                   strcmp(args[1], "next_pi") == 0 ||
                   strcmp(args[1], "next_pie") == 0 ||
                   strcmp(args[1], "next_piec") == 0 ||
                   strcmp(args[1], "next_piece") == 0) {
          args[1] = "next_piece";
          if (num > 18) {
            fprintf(stderr, "Number must be between 0 and 18.\n");
            continue;
          }
        } else {
          fprintf(stderr, "Possible fields: score, lines, next_piece\n");
          continue;
        }

        pid_t pid = fork();
        if (pid > 0) {
          int status = 0;
          wait(&status);
        } else if (pid == 0) {
          char* childArgs[5] = {"/playpen/a5/modify", args[1], args[2], path,
                                NULL};
          execv("/playpen/a5/modify", childArgs);
          // execv only returns on error
          resetColor(color);
          error(errno, errno, "Modify failed to run");
        } else {
          resetColor(color);
          error(errno, errno, "Error forking");
        }
      } else if (strcmp(args[0], "ra") == 0 || strcmp(args[0], "ran") == 0 ||
                 strcmp(args[0], "rank") == 0) {
        if (argCount == 1) {
          args[1] = "score";
          args[2] = "5";
        } else if (argCount <= 3) {
          if (strcmp(args[1], "s") == 0 || strcmp(args[1], "sc") == 0 ||
              strcmp(args[1], "sco") == 0 || strcmp(args[1], "scor") == 0 ||
              strcmp(args[1], "score") == 0) {
            args[1] = "score";
          } else if (strcmp(args[1], "l") == 0 || strcmp(args[1], "li") == 0 ||
                     strcmp(args[1], "lin") == 0 ||
                     strcmp(args[1], "line") == 0 ||
                     strcmp(args[1], "lines") == 0) {
            args[1] = "lines";
          } else {
            fprintf(stderr, "Possible sorts: score, lines\n");
            continue;
          }
          if (argCount == 2) {
            args[2] = "5";
          } else {
            char* end;
            unsigned long num = strtoul(args[2], &end, 10);
            if (num > 1000000) {
              fprintf(stderr, "Number must be between 0 and 1,000,000.\n");
              continue;
            }
          }
        } else {
          fprintf(stderr, "Usage: rank <sort> <number>\n");
          continue;
        }

        int pipes[2];
        if (pipe(pipes)) {
          resetColor(color);
          error(errno, errno, "Error creating pipe");
        }
        pid_t pid = fork();
        if (pid > 0) {
          close(pipes[0]);
          // parent writes the path to the pipe
          write(pipes[1], path, strlen(path));
          close(pipes[1]);
          int status = 0;
          wait(&status);
        } else if (pid == 0) {
          close(pipes[1]);
          // rank reads from the pipe
          dup2(pipes[0], 0);
          char* childArgs[5] = {"/playpen/a5/rank", args[1], args[2], "uplink",
                                NULL};
          execv("/playpen/a5/rank", childArgs);
          resetColor(color);
          error(errno, errno, "Rank failed to run");
        } else {
          resetColor(color);
          error(errno, errno, "Error forking");
        }
      } else if (strcmp(args[0], "c") == 0 || strcmp(args[0], "ch") == 0 ||
                 strcmp(args[0], "che") == 0 || strcmp(args[0], "chec") == 0 ||
                 strcmp(args[0], "check") == 0) {
        if (argCount != 1) {
          fprintf(stderr, "Usage: check\n");
          continue;
        }

        pid_t pid = fork();
        if (pid > 0) {
          int status = 0;
          wait(&status);
        } else if (pid == 0) {
          char* childArgs[3] = {"/playpen/a5/check", path, NULL};
          execv("/playpen/a5/check", childArgs);
          resetColor(color);
          error(errno, errno, "Check failed to run");
        } else {
          resetColor(color);
          error(errno, errno, "Error forking");
        }
      } else if (strcmp(args[0], "re") == 0 || strcmp(args[0], "rec") == 0 ||
                 strcmp(args[0], "reco") == 0 ||
                 strcmp(args[0], "recov") == 0 ||
                 strcmp(args[0], "recove") == 0 ||
                 strcmp(args[0], "recover") == 0) {
        if (argCount != 2) {
          fprintf(stderr, "Usage: recover <disk image path>\n");
          continue;
        }
        if (strlen(args[1]) > PATH_MAX) {
          fprintf(stderr, "Disk image path is too long.\n");
          continue;
        }

        int pipes[2];
        if (pipe(pipes)) {
          resetColor(color);
          error(errno, errno, "Error creating pipe");
        }
        pid_t pid = fork();
        if (pid > 0) {
          int status = 0;
          wait(&status);
        } else if (pid == 0) {
          close(pipes[0]);
          // send both output and error messages through the pipe
          dup2(pipes[1], 1);
          dup2(pipes[1], 2);
          char* childArgs[3] = {"/playpen/a5/recover", args[1], NULL};
          execv("/playpen/a5/recover", childArgs);
          resetColor(color);
          error(errno, errno, "Recover failed to run");
        } else {
          resetColor(color);
          error(errno, errno, "Error forking");
        }

        close(pipes[1]);
        // uses a file pointer to be able to read line by line with fgets
        FILE* fp = fdopen(pipes[0], "r");
        if (fp == NULL) {
          resetColor(color);
          error(errno, errno, "Error opening file");
        }
        char output[PATH_MAX + 1];
        // the number of recovered quicksaves
        unsigned recovered = 0;

        // getting output from recover program
        while (fgets(output, PATH_MAX + 1, fp) != NULL) {
          if (output[0] == 'r') {
            // only prints the header if there is at least one quicksave, and
            // reprints it every 50 quicksaves so you can see it
            if (recovered % 50 == 0) {
              if (recovered == 0) {
                printf("Recovered quicksaves:\n");
              }
              printf(
                  "--- --------------------- ---------- ----------\n"
                  "#   File path             Score      Lines\n"
                  "--- --------------------- ---------- ----------\n");
            }

            recovered++;
            if (output[strlen(output) - 1] == '\n') {
              output[strlen(output) - 1] = '\0';
            }
            FILE* tetrisfp = fopen(output, "rb");
            if (tetrisfp == NULL) {
              resetColor(color);
              error(errno, errno, "Error opening quicksave file");
            }
            TetrisGameState temp;
            size_t bytes = fread(&temp, 1, sizeof(TetrisGameState), tetrisfp);
            fclose(tetrisfp);
            if (bytes != sizeof(TetrisGameState)) {
              resetColor(color);
              error(errno, errno, "Error reading quicksave file");
            }
            printf("%-3u %-21s %-10u %-10u\n", recovered, output, temp.score,
                   temp.lines);
          } else if (output[0] == 'F') {
            // unneeded status messages
            continue;
          } else {
            // error messages
            printf("%s", output);
          }
        }
        fclose(fp);
        close(pipes[0]);

        if (recovered == 0) {
          continue;
        }
        if (recovered == 1) {
          printf("Would you like to switch to this (y/n): ");
        } else {
          printf("Would you like to switch to one of these (y/n): ");
        }
        // input is used for both y/n and the quicksave #
        char input[15];
        if (fgets(input, 11, stdin) == NULL) {
          fprintf(stderr, "Invalid command. Not switching.\n");
          continue;
        }
        if (input[strlen(input) - 1] == '\n') {
          input[strlen(input) - 1] = '\0';
        }
        for (int i = 0; i < strlen(input); i++) {
          input[i] = tolower(input[i]);
        }
        if (strcmp(input, "y") == 0 || strcmp(input, "ye") == 0 ||
            strcmp(input, "yes") == 0) {
          char newpath[PATH_MAX + 1];
          // skips asking for the quicksave number if there is only 1
          if (recovered == 1) {
            int result =
                snprintf(newpath, PATH_MAX + 1, "./recovered/out_1.bin");
            if (result < 0 || result >= PATH_MAX + 1) {
              fprintf(stderr, "Error creating path for new quicksave\n");
              return EXIT_FAILURE;
            }
          } else {
            printf("Which quicksave (enter a #): ");
            if (fgets(input, 11, stdin) == NULL) {
              fprintf(stderr, "Invalid number. Not switching.\n");
              continue;
            }
            if (input[strlen(input) - 1] == '\n') {
              input[strlen(input) - 1] = '\0';
            }
            if (strcmp(input, "e") == 0 || strcmp(input, "ex") == 0 ||
                strcmp(input, "exi") == 0 || strcmp(input, "exit") == 0) {
              resetColor(color);
              return 0;
            }
            char* end;
            unsigned long num = strtoul(input, &end, 10);
            if (num == 0 || num > recovered) {
              fprintf(stderr, "Invalid number. Not switching.\n");
              continue;
            }
            int result =
                snprintf(newpath, PATH_MAX + 1, "./recovered/out_%lu.bin", num);
            if (result < 0 || result >= PATH_MAX + 1) {
              fprintf(stderr, "Error creating path for new quicksave\n");
              return EXIT_FAILURE;
            }
          }

          // checking to see if the new path leads to the current quicksave
          int oldfd = open(path, O_RDONLY);
          if (oldfd == -1) {
            resetColor(color);
            error(errno, errno, "Error opening current quicksave file");
          }
          int newfd = open(newpath, O_RDONLY);
          if (newfd == -1) {
            resetColor(color);
            error(errno, errno, "Error opening new quicksave file");
          }
          struct stat oldstat, newstat;
          if (fstat(oldfd, &oldstat) == -1 || fstat(newfd, &newstat) == -1) {
            resetColor(color);
            error(errno, errno, "Error retrieving file info");
          }
          close(oldfd);
          close(newfd);
          if (oldstat.st_dev == newstat.st_dev &&
              oldstat.st_ino == newstat.st_ino) {
            fprintf(stderr,
                    "New quicksave is the same as the current quicksave.\n");
            continue;
          }

          // making sure the file is still a quicksave and wasn't changed
          FILE* newfp = fopen(newpath, "rb");
          if (newfp == NULL) {
            fprintf(stderr, "Invalid quicksave path. Did not switch.\n");
            continue;
          }
          TetrisGameState save;
          size_t bytes = fread(&save, 1, sizeof(TetrisGameState), newfp);
          fclose(newfp);
          if (bytes != sizeof(TetrisGameState) ||
              SanityCheckState(&save) == 0) {
            fprintf(stderr, "Invalid quicksave path. Did not switch.\n");
            continue;
          }

          printf("Switched current quicksave from \'%s\' to \'%s\'.\n", path,
                 newpath);
          strcpy(path, newpath);
        } else if (strcmp(input, "n") == 0 || strcmp(input, "no") == 0) {
          continue;
        } else if (strcmp(input, "e") == 0 || strcmp(input, "ex") == 0 ||
                   strcmp(input, "exi") == 0 || strcmp(input, "exit") == 0) {
          resetColor(color);
          return 0;
        } else {
          fprintf(stderr, "Invalid command. Not switching.\n");
        }
      } else if (strcmp(args[0], "s") == 0 || strcmp(args[0], "sw") == 0 ||
                 strcmp(args[0], "swi") == 0 || strcmp(args[0], "swit") == 0 ||
                 strcmp(args[0], "switc") == 0 ||
                 strcmp(args[0], "switch") == 0) {
        if (argCount != 2) {
          fprintf(stderr, "Usage: switch <new quicksave path>\n");
          continue;
        }
        if (strlen(args[1]) > PATH_MAX) {
          fprintf(stderr, "New quicksave path is too long.\n");
          continue;
        }

        int oldfd = open(path, O_RDONLY);
        if (oldfd == -1) {
          resetColor(color);
          error(errno, errno, "Error opening current quicksave file");
        }
        int newfd = open(args[1], O_RDONLY);
        if (newfd == -1) {
          fprintf(stderr, "Invalid quicksave path.\n");
          continue;
        }
        struct stat oldstat, newstat;
        if (fstat(oldfd, &oldstat) == -1 || fstat(newfd, &newstat) == -1) {
          resetColor(color);
          error(errno, errno, "Error retrieving file info");
        }
        close(oldfd);
        close(newfd);
        if (oldstat.st_dev == newstat.st_dev &&
            oldstat.st_ino == newstat.st_ino) {
          fprintf(stderr,
                  "New quicksave is the same as the current quicksave.\n");
          continue;
        }

        FILE* newfp = fopen(args[1], "rb");
        if (newfp == NULL) {
          fprintf(stderr, "Invalid quicksave path.\n");
          continue;
        }
        TetrisGameState save;
        size_t bytes = fread(&save, 1, sizeof(TetrisGameState), newfp);
        fclose(newfp);
        if (bytes != sizeof(TetrisGameState) || SanityCheckState(&save) == 0) {
          fprintf(stderr, "Invalid quicksave path.\n");
          continue;
        }

        printf("Switched current quicksave from \'%s\' to \'%s\'.\n", path,
               args[1]);
        strcpy(path, args[1]);
      } else if (strcmp(args[0], "v") == 0 || strcmp(args[0], "vi") == 0 ||
                 strcmp(args[0], "vis") == 0 || strcmp(args[0], "visu") == 0 ||
                 strcmp(args[0], "visua") == 0 ||
                 strcmp(args[0], "visual") == 0 ||
                 strcmp(args[0], "visuali") == 0 ||
                 strcmp(args[0], "visualiz") == 0 ||
                 strcmp(args[0], "visualize") == 0) {
        if (argCount != 1) {
          fprintf(stderr, "Usage: visualize\n");
          continue;
        }

        FILE* fp = fopen(path, "rb");
        if (fp == NULL) {
          resetColor(color);
          error(errno, errno, "Error opening current quicksave file");
        }
        TetrisGameState save;
        size_t bytes = fread(&save, 1, sizeof(TetrisGameState), fp);
        fclose(fp);
        if (bytes != sizeof(TetrisGameState) || SanityCheckState(&save) == 0) {
          resetColor(color);
          fprintf(stderr, "Error reading current quicksave file\n");
          return EXIT_FAILURE;
        }

        // puts the current piece on the board
        for (int i = 0; i < 16; i++) {
          if (tetris_pieces[save.current_piece][i] != ' ') {
            // position of game piece block on the board
            int pos = 10 * (save.location_y - i / 4) + save.location_x + i % 4;
            // if position is on the board (piece is not cut off from falling)
            if (pos >= 0) {
              // if position is already blocked by a different piece
              if (save.board[pos] != ' ') {
                resetColor(color);
                fprintf(stderr,
                        "Current game piece is in an illegal position.\n");
                return EXIT_FAILURE;
              }
              save.board[pos] = tetris_pieces[save.current_piece][i];
            }
          }
        }

        printf(
            "Visualizing savefile %s\n"
            "+----------------- Tetris ------------------+\n"
            "|                                           |\n"
            "| +--------------------+   +--- Next ----+  |\n"
            "| |%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c|   |             |  "
            "|\n"
            "| |%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c|   |  "
            "%c%c%c%c%c%c%c%c   |  |\n"
            "| |%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c|   |  "
            "%c%c%c%c%c%c%c%c   |  |\n"
            "| |%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c|   |  "
            "%c%c%c%c%c%c%c%c   |  |\n"
            "| |%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c|   |  "
            "%c%c%c%c%c%c%c%c   |  |\n"
            "| |%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c|   |             |  "
            "|\n"
            "| |%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c|   "
            "+-------------+  |\n"
            "| |%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c|                    "
            "|\n"
            "| |%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c|   +--- Score "
            "---+  |\n"
            "| |%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c|   |  %10u |  |\n"
            "| |%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c|   "
            "+-------------+  |\n"
            "| |%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c|                    "
            "|\n"
            "| |%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c|   +--- Lines "
            "---+  |\n"
            "| |%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c|   |  %10u |  |\n"
            "| |%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c|   "
            "+-------------+  |\n"
            "| |%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c|   Controls:      "
            "  |\n"
            "| |%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c|   q: quit        "
            "  |\n"
            "| |%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c|   l: quick load  "
            "  |\n"
            "| |%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c|   s: quick save  "
            "  |\n"
            "| |%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c|   space: pause   "
            "  |\n"
            "| +--------------------+   arrow keys:      |\n"
            "|                            move/rotate    |\n"
            "+-------------------------------------------+\n",
            path,
            // 1st row of board
            save.board[0], save.board[0], save.board[1], save.board[1],
            save.board[2], save.board[2], save.board[3], save.board[3],
            save.board[4], save.board[4], save.board[5], save.board[5],
            save.board[6], save.board[6], save.board[7], save.board[7],
            save.board[8], save.board[8], save.board[9], save.board[9],
            // 2nd row of board
            save.board[10], save.board[10], save.board[11], save.board[11],
            save.board[12], save.board[12], save.board[13], save.board[13],
            save.board[14], save.board[14], save.board[15], save.board[15],
            save.board[16], save.board[16], save.board[17], save.board[17],
            save.board[18], save.board[18], save.board[19], save.board[19],
            // 1st row of next piece
            tetris_pieces[save.next_piece][12],
            tetris_pieces[save.next_piece][12],
            tetris_pieces[save.next_piece][13],
            tetris_pieces[save.next_piece][13],
            tetris_pieces[save.next_piece][14],
            tetris_pieces[save.next_piece][14],
            tetris_pieces[save.next_piece][15],
            tetris_pieces[save.next_piece][15],
            // 3rd row of board
            save.board[20], save.board[20], save.board[21], save.board[21],
            save.board[22], save.board[22], save.board[23], save.board[23],
            save.board[24], save.board[24], save.board[25], save.board[25],
            save.board[26], save.board[26], save.board[27], save.board[27],
            save.board[28], save.board[28], save.board[29], save.board[29],
            // 2nd row of next piece
            tetris_pieces[save.next_piece][8],
            tetris_pieces[save.next_piece][8],
            tetris_pieces[save.next_piece][9],
            tetris_pieces[save.next_piece][9],
            tetris_pieces[save.next_piece][10],
            tetris_pieces[save.next_piece][10],
            tetris_pieces[save.next_piece][11],
            tetris_pieces[save.next_piece][11],
            // 4th row of board
            save.board[30], save.board[30], save.board[31], save.board[31],
            save.board[32], save.board[32], save.board[33], save.board[33],
            save.board[34], save.board[34], save.board[35], save.board[35],
            save.board[36], save.board[36], save.board[37], save.board[37],
            save.board[38], save.board[38], save.board[39], save.board[39],
            // 3rd row of next piece
            tetris_pieces[save.next_piece][4],
            tetris_pieces[save.next_piece][4],
            tetris_pieces[save.next_piece][5],
            tetris_pieces[save.next_piece][5],
            tetris_pieces[save.next_piece][6],
            tetris_pieces[save.next_piece][6],
            tetris_pieces[save.next_piece][7],
            tetris_pieces[save.next_piece][7],
            // 5th row of board
            save.board[40], save.board[40], save.board[41], save.board[41],
            save.board[42], save.board[42], save.board[43], save.board[43],
            save.board[44], save.board[44], save.board[45], save.board[45],
            save.board[46], save.board[46], save.board[47], save.board[47],
            save.board[48], save.board[48], save.board[49], save.board[49],
            // 4th row of next piece
            tetris_pieces[save.next_piece][0],
            tetris_pieces[save.next_piece][0],
            tetris_pieces[save.next_piece][1],
            tetris_pieces[save.next_piece][1],
            tetris_pieces[save.next_piece][2],
            tetris_pieces[save.next_piece][2],
            tetris_pieces[save.next_piece][3],
            tetris_pieces[save.next_piece][3],
            // 6th row of board
            save.board[50], save.board[50], save.board[51], save.board[51],
            save.board[52], save.board[52], save.board[53], save.board[53],
            save.board[54], save.board[54], save.board[55], save.board[55],
            save.board[56], save.board[56], save.board[57], save.board[57],
            save.board[58], save.board[58], save.board[59], save.board[59],
            // 7th row of board
            save.board[60], save.board[60], save.board[61], save.board[61],
            save.board[62], save.board[62], save.board[63], save.board[63],
            save.board[64], save.board[64], save.board[65], save.board[65],
            save.board[66], save.board[66], save.board[67], save.board[67],
            save.board[68], save.board[68], save.board[69], save.board[69],
            // 8th row of board
            save.board[70], save.board[70], save.board[71], save.board[71],
            save.board[72], save.board[72], save.board[73], save.board[73],
            save.board[74], save.board[74], save.board[75], save.board[75],
            save.board[76], save.board[76], save.board[77], save.board[77],
            save.board[78], save.board[78], save.board[79], save.board[79],
            // 9th row of board
            save.board[80], save.board[80], save.board[81], save.board[81],
            save.board[82], save.board[82], save.board[83], save.board[83],
            save.board[84], save.board[84], save.board[85], save.board[85],
            save.board[86], save.board[86], save.board[87], save.board[87],
            save.board[88], save.board[88], save.board[89], save.board[89],
            // 10th row of board
            save.board[90], save.board[90], save.board[91], save.board[91],
            save.board[92], save.board[92], save.board[93], save.board[93],
            save.board[94], save.board[94], save.board[95], save.board[95],
            save.board[96], save.board[96], save.board[97], save.board[97],
            save.board[98], save.board[98], save.board[99], save.board[99],
            // score
            save.score,
            // 11th row of board
            save.board[100], save.board[100], save.board[101], save.board[101],
            save.board[102], save.board[102], save.board[103], save.board[103],
            save.board[104], save.board[104], save.board[105], save.board[105],
            save.board[106], save.board[106], save.board[107], save.board[107],
            save.board[108], save.board[108], save.board[109], save.board[109],
            // 12th row of board
            save.board[110], save.board[110], save.board[111], save.board[111],
            save.board[112], save.board[112], save.board[113], save.board[113],
            save.board[114], save.board[114], save.board[115], save.board[115],
            save.board[116], save.board[116], save.board[117], save.board[117],
            save.board[118], save.board[118], save.board[119], save.board[119],
            // 13th row of board
            save.board[120], save.board[120], save.board[121], save.board[121],
            save.board[122], save.board[122], save.board[123], save.board[123],
            save.board[124], save.board[124], save.board[125], save.board[125],
            save.board[126], save.board[126], save.board[127], save.board[127],
            save.board[128], save.board[128], save.board[129], save.board[129],
            // 14th row of board
            save.board[130], save.board[130], save.board[131], save.board[131],
            save.board[132], save.board[132], save.board[133], save.board[133],
            save.board[134], save.board[134], save.board[135], save.board[135],
            save.board[136], save.board[136], save.board[137], save.board[137],
            save.board[138], save.board[138], save.board[139], save.board[139],
            // lines
            save.lines,
            // 15th row of board
            save.board[140], save.board[140], save.board[141], save.board[141],
            save.board[142], save.board[142], save.board[143], save.board[143],
            save.board[144], save.board[144], save.board[145], save.board[145],
            save.board[146], save.board[146], save.board[147], save.board[147],
            save.board[148], save.board[148], save.board[149], save.board[149],
            // 16th row of board
            save.board[150], save.board[150], save.board[151], save.board[151],
            save.board[152], save.board[152], save.board[153], save.board[153],
            save.board[154], save.board[154], save.board[155], save.board[155],
            save.board[156], save.board[156], save.board[157], save.board[157],
            save.board[158], save.board[158], save.board[159], save.board[159],
            // 17th row of board
            save.board[160], save.board[160], save.board[161], save.board[161],
            save.board[162], save.board[162], save.board[163], save.board[163],
            save.board[164], save.board[164], save.board[165], save.board[165],
            save.board[166], save.board[166], save.board[167], save.board[167],
            save.board[168], save.board[168], save.board[169], save.board[169],
            // 18th row of board
            save.board[170], save.board[170], save.board[171], save.board[171],
            save.board[172], save.board[172], save.board[173], save.board[173],
            save.board[174], save.board[174], save.board[175], save.board[175],
            save.board[176], save.board[176], save.board[177], save.board[177],
            save.board[178], save.board[178], save.board[179], save.board[179],
            // 19th row of board
            save.board[180], save.board[180], save.board[181], save.board[181],
            save.board[182], save.board[182], save.board[183], save.board[183],
            save.board[184], save.board[184], save.board[185], save.board[185],
            save.board[186], save.board[186], save.board[187], save.board[187],
            save.board[188], save.board[188], save.board[189], save.board[189],
            // 20th row of board
            save.board[190], save.board[190], save.board[191], save.board[191],
            save.board[192], save.board[192], save.board[193], save.board[193],
            save.board[194], save.board[194], save.board[195], save.board[195],
            save.board[196], save.board[196], save.board[197], save.board[197],
            save.board[198], save.board[198], save.board[199], save.board[199]);
      } else if (strcmp(args[0], "h") == 0 || strcmp(args[0], "he") == 0 ||
                 strcmp(args[0], "hel") == 0 || strcmp(args[0], "help") == 0) {
        if (argCount != 2) {
          fprintf(stderr, "Usage: help <command>\n");
          continue;
        }
        if (strcmp(args[1], "ra") == 0 || strcmp(args[1], "ran") == 0 ||
            strcmp(args[1], "rank") == 0) {
          printf(
              "This command calls the 'rank' program using a metric and a "
              "number "
              "of games to display to rank your quicksave against a central "
              "ranking database.\n");
        } else if (strcmp(args[1], "re") == 0 || strcmp(args[1], "rec") == 0 ||
                   strcmp(args[1], "reco") == 0 ||
                   strcmp(args[1], "recov") == 0 ||
                   strcmp(args[1], "recove") == 0 ||
                   strcmp(args[1], "recover") == 0) {
          printf(
              "This command calls the 'recover' program using a disk image "
              "path to retrieve accidentally deleted quicksaves.\n");
        } else if (strcmp(args[1], "m") == 0 || strcmp(args[1], "mo") == 0 ||
                   strcmp(args[1], "mod") == 0 ||
                   strcmp(args[1], "modi") == 0 ||
                   strcmp(args[1], "modif") == 0 ||
                   strcmp(args[1], "modify") == 0) {
          printf(
              "This command calls the 'modify' program using a field and a "
              "value "
              "to modify the state of your quicksave.\n");
        } else if (strcmp(args[1], "c") == 0 || strcmp(args[1], "ch") == 0 ||
                   strcmp(args[1], "che") == 0 ||
                   strcmp(args[1], "chec") == 0 ||
                   strcmp(args[1], "check") == 0) {
          printf(
              "This command call the 'check' program with your quicksave to "
              "verify it will pass legitimacy checks.\n");
        } else if (strcmp(args[1], "v") == 0 || strcmp(args[1], "vi") == 0 ||
                   strcmp(args[1], "vis") == 0 ||
                   strcmp(args[1], "visu") == 0 ||
                   strcmp(args[1], "visua") == 0 ||
                   strcmp(args[1], "visual") == 0 ||
                   strcmp(args[1], "visuali") == 0 ||
                   strcmp(args[1], "visualiz") == 0 ||
                   strcmp(args[1], "visualize") == 0) {
          printf(
              "This command visualizes the current state of the Tetris game "
              "board as it would be displayed in-game.\n");
        } else if (strcmp(args[1], "s") == 0 || strcmp(args[1], "sw") == 0 ||
                   strcmp(args[1], "swi") == 0 ||
                   strcmp(args[1], "swit") == 0 ||
                   strcmp(args[1], "switc") == 0 ||
                   strcmp(args[1], "switch") == 0) {
          printf(
              "This command switches the current quicksave with a new one.\n");
        } else if (strcmp(args[1], "e") == 0 || strcmp(args[1], "ex") == 0 ||
                   strcmp(args[1], "exi") == 0 ||
                   strcmp(args[1], "exit") == 0) {
          printf("This command exits the program.\n");
        } else if (strcmp(args[1], "i") == 0 || strcmp(args[1], "in") == 0 ||
                   strcmp(args[1], "inf") == 0 ||
                   strcmp(args[1], "info") == 0) {
          printf(
              "This command displays statistics of your current quicksave.\n");
        } else {
          fprintf(stderr,
                  "Possible commands for help: modify, rank, check, recover, "
                  "switch, visualize, exit\n");
        }
      } else if (strcmp(args[0], "i") == 0 || strcmp(args[0], "in") == 0 ||
                 strcmp(args[0], "inf") == 0 || strcmp(args[0], "info") == 0) {
        if (argCount != 1) {
          fprintf(stderr, "Usage: info\n");
          continue;
        }
        printf("Current quicksave file path: %s\nScore: %u\nLines: %u\n", path,
               score_print, lines_print);
      } else if (strcmp(args[0], "e") == 0 || strcmp(args[0], "ex") == 0 ||
                 strcmp(args[0], "exi") == 0 || strcmp(args[0], "exit") == 0) {
        if (argCount != 1) {
          fprintf(stderr, "Usage: exit\n");
          continue;
        }
        resetColor(color);
        return 0;
      } else {
        fprintf(stderr,
                "Possible commands: modify, rank, check, recover, switch, "
                "visualize, help, exit\n");
      }
    }
  }
}
