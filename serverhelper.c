#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "serverhelper.h"

//  Returns 1 if the channel exists, 0 if not
int checkChannel(channel *chann, int num, int id) {
  int i;
  for (i = 0; i < num; i++) {
    if (chann[i].id == id) {
      return 1;
    }
  }

  return 0;
}
