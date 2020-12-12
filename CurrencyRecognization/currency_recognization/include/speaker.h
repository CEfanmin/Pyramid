#ifndef SPEAKER_H_
#define SPEAKER_H_

#include "stdafx.h"
#include "speak_lib.h"
#include <unistd.h>

int speak(string text);

int speak_synchronously(string text);

void initSpeaker();

int AutoSpeak(string text);

#endif
