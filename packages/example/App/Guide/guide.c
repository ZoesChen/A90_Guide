#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "key.h"

#define MAX_MUSIC_INDEX 4 // 4bit num -> 1 music num

static pthread_t keyThread;
static int readKeyFlag = 0;
static int readCount = 100;

#define DEBUG_X86
//X86_ESC: stop read
//X86_ZERO: put music num into queue

int music_num[4] = {0};

#ifdef DEBUG_X86
#define X86_ESC   1
#define X86_KEY1 2
#define X86_KEY2 3
#define X86_KEY3 4
#define X86_KEY4 5
#define X86_KEY5 6
#define X86_KEY6 7
#define X86_KEY7 8
#define X86_KEY8 9
#define X86_KEY9 10
#define X86_ZERO 11
#define X86_KEY_A 30

static int oldKey = -1;
static int newKey = -1;
#endif


int Init()
{
	int res;
	res = OpenKeyDev();
	if(res) {
		return -1;
	}
	readKeyFlag = 1;
	return 0;
}

void *KeyThreadHandle(void *arg)
{
	printf("Enter into KeyThreadHandle\n");
	int keyCode;
	int musicNumIndex = 0;
#ifdef DEBUG_X86
	int x86StartFlag = 0;
#endif
	printf("readKeyFlag = %d, readCount = %d\n", readKeyFlag, readCount);
	while(readKeyFlag) {
		ReadKey(&keyCode);
#ifdef DEBUG_X86
		if (keyCode == X86_KEY_A) {
			x86StartFlag = 1;
			continue;
		}
		if (x86StartFlag != 1) {
			continue;
		}
			oldKey = newKey;
			newKey = keyCode;
			if (oldKey == newKey)
				continue;
#endif	

		if (keyCode == X86_ESC) {
			printf("Press esc\n");
			readKeyFlag = 0;
			continue;
		}
		
		if (keyCode >= X86_KEY1 && keyCode <= X86_ZERO) {
			//printf("musicNumIndex = %d\n",  musicNumIndex);
			
			if (musicNumIndex < MAX_MUSIC_INDEX) {
				if (keyCode == X86_ZERO)
					music_num[musicNumIndex] = 0;
				else
					music_num[musicNumIndex] =keyCode -1;
				//printf("music_num[%d] = %d", musicNumIndex, music_num[musicNumIndex]);
				musicNumIndex++;
			}

			if (musicNumIndex == MAX_MUSIC_INDEX) {
				musicNumIndex = 0;
				int i = 0;
				printf("MusicNum = %d\n", music_num[0]*1000 + music_num[1]*100 + music_num[2]*10 + music_num[3]);
				for(i = 0; i < MAX_MUSIC_INDEX; i++) {
					music_num[i] = 0;
				}
				//Enqueue music num into keyQueue
				//ToDo
			}
		}
	}
#ifdef DEBUG_X86
	x86StartFlag = 0;
#endif
	printf("Will return from keyThread\n");
	return NULL;
}

int main(int argv, char **argc)
{
	int res;
	printf("Begin......\n");
	if (Init() < 0) {
		return -1;
	}

	res = pthread_create(&keyThread, NULL, KeyThreadHandle, NULL);
	pthread_join(keyThread, NULL);
	return 0;
}



