/*
 * player.h
 *
 *  Created on: Nov 10, 2016
 *      Author: kratos
 */

#ifndef PLAYER_H_
#define PLAYER_H_


//Swindow related implementation
typedef float TAvailabilityWindow;
typedef float TSizeWindow;
typedef float TOccupancyWindow;

typedef struct player TPlayer;
typedef struct playback TPlayback;
typedef struct chunk TChunk;
typedef struct window TWindow;

typedef void TSchedulingPolicy;

//Player
TPlayer *createPlayer(TSizeWindow size, TSchedulingPolicy *policy );
TWindow *createWindow(TSizeWindow size, TSchedulingPolicy *policy );

//Window
typedef void (* TSetOccupancyWindow)(TWindow *window, float lenghtObject);

typedef void (* TBufferingPlayer)(unsigned int idPeer, void* hashTable, void* community, void* systemData);
typedef short (* TSchedulingPlayer)(TPlayer *player, void *object);
typedef void (* TPlaybackPlayer)(TPlayer *player);
typedef short (* TStallPlayer)(TPlayer *player);
typedef void (* TSetStorageWindow)(TWindow *window, void *storage);


typedef short (* TSwapStoragePlayer)(TPlayer *player);

typedef short (* TIsFitInWindow)(TPlayer *player, void *object);
typedef short (* THasWindow)(TPlayer *player, void *object);
typedef short (* TDisposeWindow)(TPlayer* player);
typedef void (* TShowWindow)(TPlayer* player);
typedef short (* TGetLevelStorageWindow)(TWindow* window);
typedef TAvailabilityWindow (* TGetAvailabilityWindow)(TWindow* window);
typedef TSizeWindow (* TGetSizeWindow)(TWindow* window);
typedef TWindow *(* TGetWindow)(TPlayer* player);
typedef TOccupancyWindow(* TGetOccupancyWindow)(TWindow *window);
typedef unsigned int (* TGetNumberOfStoredObjectWindow)(TPlayer* player);

struct player{
	//private data
	void *data;

	//public methods
//1 player
//2 janela

	TBufferingPlayer buffering; //1
	//TSchedulingPlayer scheluding; //1
	TPlaybackPlayer playback; //1
	TStallPlayer stall; //1//sugestao estatisticas
	//TSetStoragePlayer setStorage;

	TSwapStoragePlayer swapStorage;
	TGetWindow getWindow;
	//TIsFitInWindow isFitInWindow;
	//THasWindow has;
	//TShowWindow show;

	//gets and sets
/*	TGetLevelStorageWindow getLevelStorage;
	TGetAvailabilityWindow getAvailability;
	TGetSizeWindow getSize;
	TGetWindow getWindow;
	TGetOccupancyWindow getOccupancy;
	TGetNumberOfStoredObjectWindow getNumberOfStoredObject;*/


};





struct playback{

	//a definir
	void *availableChunks;
	void *scheduleChunks;


};


//Window
struct window{

	void *data;

	TSchedulingPlayer scheluding;
	TIsFitInWindow isFitInWindow;
	THasWindow has;
	TShowWindow show;

	TGetLevelStorageWindow getLevelStorage;
	TGetAvailabilityWindow getAvailability;
	TGetSizeWindow getSize;

	TGetOccupancyWindow getOccupancy;
	TGetNumberOfStoredObjectWindow getNumberOfStoredObject;

	TSetStorageWindow setStorage;
	TSetOccupancyWindow setOccupancy;
};


//chunk
struct chunk{

	char ID;
	float initFlow;
	float endFlow;
	float lengthBytes;
	float duration;
	int available;  // 0 | 1
	int reproduced; // 0 | 1

};

//
//Greedy
typedef struct GreedyPolicy TGreedyPolicy;
void *createGreedyPolicy(void *entry);
short replaceGreedyPolicy(void* vSysInfo, void* cache, void* object);
short updateGreedyPolicy(void* systemData,void* cache, void* object);

// Politica generica
// Policy related declaration
//
typedef void TSWMPolicy;

// General Policy stuff declaration
typedef short (* TSWMReplaceGeneralPolicy)(void* systemData, void* cache, void* object);
typedef short (* TSWMUpdateGeneralPolicy)(void* systemData, void* cache, void* object);
typedef short (* TSWMisFitInSwindowGeneralPolicy)(void* player, void* object);


//
typedef struct GeneralPolicySwm TGeneralPolicySwm;

// Policy related data/function definition
//
typedef struct SWMGeneralPolicy TSWMGeneralPolicy;

struct SWMGeneralPolicy{
	TSWMReplaceGeneralPolicy Replace; // Object Management Policy Replacement(LRU/Popularity)
	TSWMUpdateGeneralPolicy Update; // Object Management Policy Update cache(LRU/Popularity)
	TSWMisFitInSwindowGeneralPolicy IsFitInSwindow; // Eligibility criteria

};

typedef void TDATAGeneralPolicySwm;

struct GeneralPolicySwm{
	TSWMGeneralPolicy *SWM;
	TDATAGeneralPolicySwm *data;
};
//fim politica generica


//short isFitInSwindowGreedyPolicy(void* player, void* object);











#endif /* PLAYER_H_ */
