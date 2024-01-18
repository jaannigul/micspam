#include "kbdcommands.h"

#include "audioplayer.h"
#include "globals.h"

#include <Windows.h>
#include <pthread.h>
#include <math.h>

int KEYBOARD_PLAY_OR_STOP_AUDIO_BIND = VK_OEM_PERIOD;
int KEYBOARD_NEXT_AUDIO_BIND = VK_DOWN;
int KEYBOARD_PREV_AUDIO_BIND = VK_UP;
int KEYBOARD_INCREASE_VOLUME_BIND = VK_OEM_PLUS;
int KEYBOARD_DECREASE_VOLUME_BIND = VK_OEM_MINUS;
// int KEYBOARD_RELOAD_AUDIO_FILES_BIND;

// a

int currentlySelectedSong = 0;

void keyboardCommandListener(void** threadArgs) { // actual arguments: char** fileList, int numFiles
	char** fileList = threadArgs[0];
	int numFiles = *(int*)threadArgs[1];

	while (1) {
		if (GetAsyncKeyState(KEYBOARD_PLAY_OR_STOP_AUDIO_BIND) & 1) {

			int res = togglePlayingAudio(fileList[currentlySelectedSong]);
			switch (res) {
			case PLAYER_NO_ERROR:
				printf("[KBD CMD] Toggling audio for sample %s\n", fileList[currentlySelectedSong]);
				break;
			case PLAYER_THREAD_KILLED:
				printf("[KBD CMD] Audio player halted.\n");
				break;			
			case PLAYER_THREAD_FAILED_TO_KILL:
				printf("[KBD CMD] Failed to stop audio player thread.\n");
				break; 
			case PLAYER_COULDNT_FIND_AUDIO:
				printf("[KBD CMD] Audio file '%s' has been deleted and can no longer be played.\n");
				break;
			}
		}

		if (GetAsyncKeyState(KEYBOARD_NEXT_AUDIO_BIND) & 1) {
			currentlySelectedSong = (currentlySelectedSong + 1) % numFiles;
			printf("[KBD CMD] Audio file '%s' selected as active song.\n", fileList[currentlySelectedSong]);
		}

		if (GetAsyncKeyState(KEYBOARD_PREV_AUDIO_BIND) & 1) {
			currentlySelectedSong--;
			if (currentlySelectedSong < 0) currentlySelectedSong = numFiles - 1;

			printf("[KBD CMD] Audio file '%s' selected as active song.\n", fileList[currentlySelectedSong]);
		}

		if (GetAsyncKeyState(KEYBOARD_INCREASE_VOLUME_BIND) & 1) {
			soundVolume += 0.01f;
			soundVolume = min(ceilf(soundVolume * 100.0f) / 100.0f, 1.0f);

			printf("[KBD CMD] Audio volume increased to %d %%.\n", (int)ceilf(soundVolume * 100.0f));
		}

		if (GetAsyncKeyState(KEYBOARD_DECREASE_VOLUME_BIND) & 1) {
			soundVolume -= 0.01f;
			soundVolume = max(0.0f, floorf(soundVolume * 100.0f) / 100.0f);

			printf("[KBD CMD] Audio volume decreased to %d %%.\n", (int)floorf(soundVolume * 100.0f));
		}
	}
}