/*
  Simple program to get data from event triggered by librespot, and store TRACK_ID into env vars
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

#define TRACK_ID_LENGTH         22

char* currentTrackId = NULL;
key_t shm_key = 5678; // shared memory key

/* -- MAIN function -- */

int
main(int argc, char *argv[])
{
    if (getenv("TRACK_ID")) {
        int shmid;
        if ((shmid = shmget(shm_key, TRACK_ID_LENGTH, IPC_CREAT | 0666)) < 0) {
            perror("shmget");
            exit(1);
        }
        if ((currentTrackId = shmat(shmid, NULL, 0)) == (char *) -1) {
            perror("shmat");
            exit(1);
        }

        memcpy(currentTrackId, getenv("TRACK_ID"), TRACK_ID_LENGTH);
    }

    return 0; /* to prevent compiler warning */
}


