#ifndef _GUIDE_KEY_H_
#define _GUIDE_KEY_H_

#define KEY_DEVICE "/dev/input/event3"

int OpenKeyDev();
int ReadKey(int *keyCode);
void CloseKeyDev();
#endif