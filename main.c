#include <spawn.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


extern char **environ;

int powerprofiles(char *argv[]) {
  pid_t pid;

  char *args[] = {"powerprofilesctl", "set", "balanced", NULL};

  switch (atoi(argv[2])) {
  case 0:
    args[2] = "power-saver";
    break;
  case 1:
    args[2] = "balanced";
    break;
  case 2:
    args[2] = "performance";
    break;
  default:
      fprintf(stderr, "input profile not in range 0-2");
    return 2;
  }

  int err = posix_spawnp(&pid, "powerprofilesctl", NULL, NULL, args, environ);
  if (err != 0) {
      fprintf(stderr, "posix_spawnp failed");
    return 3;
  }
  int status;
  waitpid(pid, &status, 0);

  return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}


int main(int argc, char *argv[]) {

  if (argc >=2) {
      if (strcmp(argv[1], "power") == 0) {
          powerprofiles( argv);
      } else if (strcmp(argv[1], "bluetooth") ==0) {
          printf("\nbluetooth\n");
      } else if (strcmp(argv[1], "wifi") == 0) {
          printf("\nwifi\n");
      } else if (strcmp(argv[1], "volume") == 0) {
          printf("\nvolume\n");
      } else {
          fprintf(stderr, "%s is not a valid argument", argv[1]);
          return 0;
      }

  }


  return 1;
}
