#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "../headers/config.h"
#include "../headers/control.h"
#include "../headers/definitions.h"
#include "../headers/globals.h"
#include "../headers/input.h"
#include "../headers/logic.h"
#include "../headers/screen.h"
#include "../headers/system_logic.h"
#include "../headers/doubly_linked_rom_list.h"


void initializeGlobals() {
	running=1;
	currentSectionNumber=0;
	gamesInPage=0;
	CURRENT_SECTION.totalPages=0;
	MAX_GAMES_IN_SECTION=50000;
	favoritesSectionNumber=0;
	favoritesSize=0;
	currentCPU=OC_NO;
	favoritesSectionSelected=0;
	favoritesChanged=0;
	ITEMS_PER_PAGE=10;
	isPicModeMenuHidden=1;
	srand(time(0));
}

void resetFrameBuffer () {
	int ret = system("./scripts/reset_fb");
	if (ret==-1) {
		generateError("FATAL ERROR", 1);
	}
}

void sig_term_handler(int signum)
{
	running=0;
}


//void *checkClock(void *x_void_ptr) {
//	while (1) {
//		currRawtime = time (NULL);
//		currTime = localtime(&currRawtime);
//		if(currTime->tm_sec>lastSec) {
//			lastSec=currTime->tm_sec;
//			if(!isSuspended&&!leftOrRightPressed&&!isUSBMode) {
//				printf("iupdate last\n");
//				lastChargeLevel = getBatteryLevel();
//				updateScreen();
//			}
//		}
//		free (currTime);
//	}
//	printf("pepe\n");
//}

int main(int argc, char* argv[]) {
	//	testSectionLoad();
	//	exit(0);
	//	lastChargeLevel = getBatteryLevel();
	//	currRawtime = time(NULL);
	//	currTime = localtime(&currRawtime);
	//	lastSec=currTime->tm_sec;
	//	pthread_mutex_init(&lock, NULL);
	//	pthread_create(&clockThread, NULL, checkClock,NULL);
	signal(SIGTERM, &sig_term_handler);
	#if defined(TARGET_NPG) || defined(TARGET_RG350)
	resetFrameBuffer();
	#endif
	createConfigFilesInHomeIfTheyDontExist();
	loadConfig();
	#if defined(TARGET_BITTBOY) || defined(TARGET_RG300) || defined(TARGET_RG350) || defined(TARGET_NPG)
	HW_Init();
	setCPU(OC_NO);
	#endif
	initializeGlobals();
	setupDisplayAndKeys();
	int sectionCount=loadSections();
	loadFavorites();
	if (argv[1]!=NULL) {
		setSectionsState(argv[1]);
		currentSectionNumber=atoi(argv[2]);
		returnTo=atoi(argv[3]);
		fullscreenMode=atoi(argv[4]);
	}
	loadLastState();
	if(fullscreenMode) {
		ITEMS_PER_PAGE=12;
	}
	#if defined(TARGET_BITTBOY) || defined(TARGET_RG300) || defined(TARGET_RG350)
	initSuspendTimer();
	#endif
	determineStartingScreen(sectionCount);
	if (CURRENT_SECTION.currentGameNode!=NULL) {
		updateScreen(CURRENT_SECTION.currentGameNode->data);
	} else {
		updateScreen(NULL);
	}
	enableKeyRepeat();
	while (running) {
		while(pollEvent()){
			if(getEventType()==getKeyDown()){
				if (!isSuspended) {
					if (!currentlyChoosingEmulator) {
						if (CURRENT_SECTION.currentGameNode!=NULL) {
							performAction(CURRENT_SECTION.currentGameNode->data);
						} else {
							performAction(NULL);
						}
					} else {
						performChoosingAction();
					}
				}
				#ifndef TARGET_PC
				resetScreenOffTimer();
				#endif
				if (CURRENT_SECTION.currentGameNode!=NULL) {
					updateScreen(CURRENT_SECTION.currentGameNode->data);
				} else {
					updateScreen(NULL);
				}
			} else if (getEventType()==getKeyUp()) {
				if(getPressedKey()==BTN_B) {
					hotKeyPressed=0;
					if(fullscreenMode) {
						if(currentlySectionSwitching) {
							hideFullScreenModeMenu();
						} else if (CURRENT_SECTION.alphabeticalPaging) {
							resetPicModeHideMenuTimer();
						}
					}
					CURRENT_SECTION.alphabeticalPaging=0;
					currentlySectionSwitching=0;
					if (CURRENT_SECTION.currentGameNode!=NULL) {
						updateScreen(CURRENT_SECTION.currentGameNode->data);
					} else {
						updateScreen(NULL);
					}
				}
				if(getPressedKey()==BTN_SELECT&&!hotKeyPressed) {
					if (stripGames) {
						stripGames=0;
					} else {
						stripGames=1;
					}
					if (CURRENT_SECTION.currentGameNode!=NULL) {
						updateScreen(CURRENT_SECTION.currentGameNode->data);
					} else {
						updateScreen(NULL);
					}
				}
			}
		}
	}
//	PrintDoublyLinkedRomList(CURRENT_SECTION.head);
	quit();
}