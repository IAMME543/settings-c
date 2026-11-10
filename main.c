#include <spawn.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


extern char **environ;

int clamp(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

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

int volume(char *argv[]) {

    pid_t pid;
    char *args[] = {"wpctl", NULL, "@DEFAULT_AUDIO_SINK@", NULL};
    if (strcmp(argv[2],"get")==0) {
        int pipefd[2];
        pipe(pipefd);

        posix_spawn_file_actions_t actions;
        posix_spawn_file_actions_init(&actions);

        posix_spawn_file_actions_adddup2(&actions, pipefd[1], STDOUT_FILENO);
        posix_spawn_file_actions_addclose(&actions, pipefd[0]);
        posix_spawn_file_actions_addclose(&actions, pipefd[1]);

        char *args[] = {
            "wpctl",
            "get-volume",
            "@DEFAULT_AUDIO_SINK@",
            NULL
        };

        pid_t pid;

        int err = posix_spawnp(
            &pid, "wpctl", &actions, NULL, args, environ
        );

        close(pipefd[1]);
        posix_spawn_file_actions_destroy(&actions);

        if (err != 0) {
            close(pipefd[0]);
            return 3;
        }

        char buffer[128];
        ssize_t n = read(pipefd[0], buffer, sizeof(buffer) - 1);

        if (n > 0) {
            buffer[n] = '\0';
            // printf("Output: %s", buffer);
        }

        float volume;

        if (sscanf(buffer, "Volume: %f", &volume) == 1) {
            printf("Volume is %.0f%%\n", volume * 100);
        }

        close(pipefd[0]);

        int status;
        waitpid(pid, &status, 0);

        return WIFEXITED(status) ? WEXITSTATUS(status) : 1;

    } else if (strcmp(argv[2], "set")==0) {
        args[1] = "set-volume";

        int volume = clamp(atoi(argv[3]), 0, 100);
        char volume_arg[16];

        snprintf(volume_arg, sizeof(volume_arg), "%.2f", volume / 100.0f);

        args[3] = volume_arg;

    } else if (strcmp(argv[2], "mute")==0) {
        args[1] = "set-mute";
        args[3] = "toggle";
    }


    int err = posix_spawnp(&pid, "wpctl", NULL, NULL, args, environ);
    if (err != 0) {
        fprintf(stderr, "posix_spawnp failed");
      return 3;
    }
    int status;
    waitpid(pid, &status, 0);

    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}

int wifi(int argc, char *argv[]) {

    pid_t pid;
    char *args[] = {"nmcli", "radio", "wifi", NULL, NULL, NULL, NULL, NULL};
    if (argc < 3) {
        args[1] = "device";
        args[2] = "status";
    } else if (strcmp(argv[2],"on")==0) {
        args[3] = "on";
        printf("british wifi on");
    } else if (strcmp(argv[2],"off")==0) {
        args[3]="off";
    } else if (strcmp(argv[2], "list")==0) {
        args[1]="device";
    } else if (strcmp(argv[2], "connect")==0) {
        if (argc < 5) {
            fprintf(stderr, "Not enough arguments\n");
            return 1;
        }
        args[1] = "device";
        args[2] = "wifi";
        args[3] = "connect";
        args[4] = argv[3];
        args[5] = "password";
        args[6] = argv[4];
    }
    else {
        args[1] = "device";
        args[2] = "status";
    }

    int err = posix_spawnp(&pid, "nmcli", NULL, NULL, args, environ);
    if (err != 0) {
        fprintf(stderr, "posix_spawnp failed: %s\n", strerror(err));
      return 3;
    }
    int status;
    waitpid(pid, &status, 0);

    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}

int bluetooth(int argc, char*argv[]) {

    pid_t pid;
    char *args[] = {"bluetoothctl", NULL, NULL, NULL, NULL, NULL, NULL, NULL};
    if (argc < 3) {
        args[1] = "devices";
    } else if (strcmp(argv[2],"on")==0) {
        args[1] = "power";
        args[2]="on";
    } else if (strcmp(argv[2], "off")) {
            args[1]="power";
            args[2]="on";
    } else if (strcmp(argv[2], "list")) {
        args[1] = "devices";
    } else if (strcmp(argv[2], "list-paired")) {
        args[1] = "paired-devices";
    } else {
        args[1] = "devices";
    }

    int err = posix_spawnp(&pid, "bluetoothctl", NULL, NULL, args, environ);
    if (err != 0) {
        fprintf(stderr, "posix_spawnp failed: %s\n", strerror(err));
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
          bluetooth(argc, argv);
          printf("bluetooth\n");
      } else if (strcmp(argv[1], "wifi") == 0) {
          printf("Wifi\n");
          wifi(argc, argv);
      } else if (strcmp(argv[1], "volume") == 0) {
          printf("Volume\n");
          volume(argv);
      } else {
          fprintf(stderr, "%s is not a valid argument.", argv[1]);
          return 0;
      }
  }
  return 1;
}
