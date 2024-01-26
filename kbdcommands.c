#include "kbdcommands.h"

#include "audioplayer.h"
#include "globals.h"
#include "gui/gui_main.h"
#include "consts.h"

#include <Windows.h>
#include <pthread.h>
#include <math.h>
#include <stdio.h>

int KEYBOARD_PLAY_OR_STOP_AUDIO_BIND = VK_OEM_PERIOD;
int KEYBOARD_NEXT_AUDIO_BIND = VK_DOWN;
int KEYBOARD_PREV_AUDIO_BIND = VK_UP;
int KEYBOARD_INCREASE_VOLUME_BIND = VK_OEM_PLUS;
int KEYBOARD_DECREASE_VOLUME_BIND = VK_OEM_MINUS;
// int KEYBOARD_RELOAD_AUDIO_FILES_BIND;

int currentlySelectedSong = 0;

void keyboardCommandListener(void** threadArgs) { // actual arguments: char** fileList, int numFiles
	char** fileList = threadArgs[0];
	int numFiles = *(int*)threadArgs[1];

	char popupText[MAX_CHAR_BUF_LEN] = { 0 };

	if(numFiles == 0)
		printf("[KBD CMD] Warning: you have no audio samples, some commands will not work\n" );

	while (1) {
		char* songName = fileList[currentlySelectedSong] + strlen(USER_AUDIO_FILES_PATH) + 1; // remove the prefix

		if (GetAsyncKeyState(KEYBOARD_PLAY_OR_STOP_AUDIO_BIND) & 1 && numFiles > 0) {

			int res = togglePlayingAudio(fileList[currentlySelectedSong]);
			switch (res) {
			case PLAYER_NO_ERROR:
				printf("[KBD CMD] Toggling audio for sample %s\n", songName);
				snprintf(popupText, MAX_CHAR_BUF_LEN, "Toggling audio for sample %s", songName);
				break;
			case PLAYER_THREAD_KILLED:
				printf("[KBD CMD] Audio player halted.\n");
				snprintf(popupText, MAX_CHAR_BUF_LEN, "Audio player halted");
				break;			
			case PLAYER_THREAD_FAILED_TO_KILL:
				printf("[KBD CMD] Failed to stop audio player thread.\n");
				snprintf(popupText, MAX_CHAR_BUF_LEN, "Failed to stop audio player thread");
				break; 
			case PLAYER_COULDNT_FIND_AUDIO:
				printf("[KBD CMD] Audio file '%s' has been deleted and can no longer be played.\n", songName);
				snprintf(popupText, MAX_CHAR_BUF_LEN, "Audio file '%s' has been deleted and can no longer be played.", songName);
				break;
			}

			sendPopupNotification(POPUP_TEXT, popupText, 0, 0, DT_WORDBREAK | DT_CENTER);
		}

		if (GetAsyncKeyState(KEYBOARD_NEXT_AUDIO_BIND) & 1 && numFiles > 0) {

			currentlySelectedSong = (currentlySelectedSong + 1) % numFiles;
			printf("[KBD CMD] Audio file '%s' selected as active song.\n", songName);

			sendPopupNotification(POPUP_SONGS, fileList, numFiles, currentlySelectedSong, 0);
		}

		if (GetAsyncKeyState(KEYBOARD_PREV_AUDIO_BIND) & 1 && numFiles > 0) {
			currentlySelectedSong--;
			if (currentlySelectedSong < 0) currentlySelectedSong = numFiles - 1;

			printf("[KBD CMD] Audio file '%s' selected as active song.\n", songName);

			sendPopupNotification(POPUP_SONGS, fileList, numFiles, currentlySelectedSong, 0);
		}

		if (GetAsyncKeyState(KEYBOARD_INCREASE_VOLUME_BIND) & 1) {
			soundVolume += 1;
			soundVolume = min(100, soundVolume);

			printf("[KBD CMD] Audio volume increased to %d %%.\n", soundVolume);

			snprintf(popupText, MAX_CHAR_BUF_LEN, "Volume set to %d%%", soundVolume);
			sendPopupNotification(POPUP_TEXT, popupText, 0, 0, DT_SINGLELINE | DT_VCENTER);
		}

		if (GetAsyncKeyState(KEYBOARD_DECREASE_VOLUME_BIND) & 1) {
			soundVolume -= 1;
			soundVolume = max(soundVolume, 0);

			printf("[KBD CMD] Audio volume decreased to %d %%.\n", soundVolume);

			snprintf(popupText, MAX_CHAR_BUF_LEN, "Volume set to %d%%", soundVolume);
			sendPopupNotification(POPUP_TEXT, popupText, 0, 0, DT_SINGLELINE | DT_VCENTER);
		}
	}
}