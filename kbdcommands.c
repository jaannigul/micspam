#include "kbdcommands.h"

#include "audioplayer.h"

#include <Windows.h>
#include <pthread.h>

int KEYBOARD_PLAY_OR_STOP_AUDIO_BIND = VK_OEM_PERIOD;
// int KEYBOARD_NEXT_AUDIO_BIND;
// int KEYBOARD_PREV_AUDIO_BIND;
// int KEYBOARD_RELOAD_AUDIO_FILES_BIND;


int currentlySelectedSong = 0;

void keyboardCommandListener(void** threadArgs) { // actual arguments: char** fileList, int numFiles
	char** fileList = threadArgs[0];
	int numFiles = (int*)threadArgs[1];

	while (1) {
		if (GetAsyncKeyState(KEYBOARD_PLAY_OR_STOP_AUDIO_BIND) & 1) {

			int res = togglePlayingAudio(fileList[currentlySelectedSong]);
			switch (res) {
			case PLAYER_NO_ERROR:
				printf("[KBD CMD] Toggling audio for sample %s", fileList[currentlySelectedSong]);
				break;
			case PLAYER_THREAD_KILLED:
				printf("[KBD CMD] Audio player halted.");
				break;			
			case PLAYER_THREAD_FAILED_TO_KILL:
				printf("[KBD CMD] Failed to stop audio player thread.");
				break; 
			case PLAYER_COULDNT_FIND_AUDIO:
				printf("[KBD CMD] Audio file '%s' has been deleted and can no longer be played.");
				break;
			}
		}
	}
}