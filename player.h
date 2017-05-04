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
typedef float TBufferWindow;
typedef float TOccupBufferWindow;

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
typedef void (* TSetOccupBufferWindow)(TWindow *window, float lenghtObject);



typedef void (* TSetPlaybackedTime)(TWindow *window, float lenghtObject);
typedef void (* TSetLastPlaybackedObj)(TWindow *window, long int numberObject);
typedef void (* TSetLastChunkAvailable)(TWindow *window, long int numberObject);

typedef void (* TResetWindow)(TWindow *window);





typedef void (* TBufferingPlayer)(unsigned int idPeer, void* hashTable, void* community, void* systemData);
typedef void* (* TSchedulingPlayer)(void *peer, void *video, void *listPeer,void *picked);
typedef float (* TPlaybackPlayer)(TPlayer *player,void *hashTable, void *peer, void *systemData);

typedef short (* TStallPlayer)(TPlayer *player);
typedef void (* TSetStorageWindow)(TWindow *window, void *storage);


typedef short (* TSwapStoragePlayer)(TPlayer *player);

//typedef short (* TIsFitInWindow)(TPlayer *player, void *object);
typedef void* (* TSchedulingWindow)(void *peer, void *video, void *listPeer,void *picked);

typedef short (* THasWindow)(TPlayer *player, void *object);
typedef short (* TDisposeWindow)(TPlayer* player);
typedef void (* TShowWindow)(TPlayer* player);
typedef short (* TGetLevelStorageWindow)(TWindow* window);
typedef TAvailabilityWindow (* TGetAvailabilityWindow)(TWindow* window);
typedef TSizeWindow (* TGetSizeWindow)(TWindow* window);
typedef TWindow *(* TGetWindow)(TPlayer* player);
typedef float (* TGetDownTimeChunk)(void *peer, TPlayer *player, float lenghtObject);

typedef long int (* TGetCollectionLength)(void *datasource);

typedef TOccupancyWindow(* TGetOccupancyWindow)(TWindow *window);
typedef TBufferWindow(* TGetBufferWindow)(TWindow *window);
typedef TOccupBufferWindow(* TGetOccupBufferWindow)(TWindow *window);

typedef long int (* TGetLastPlaybackedObj)(TWindow *window);
typedef long int (* TGetLastChunkAvailable)(TWindow *window);

typedef float (* TGetPlaybackedTime)(TWindow *window);
typedef float (* TGetRemainingPlayingTime)(TWindow *window);



typedef unsigned int (* TGetNumberOfStoredObjectWindow)(TPlayer* player);

struct player{
	//private data
	void *data;

	//public methods
//1 player
//2 janela

	TBufferingPlayer buffering; //1
	TSchedulingPlayer scheluding; //1
	TPlaybackPlayer playback; //1
	TStallPlayer stall; //1//sugestao estatisticas
	//TSetStoragePlayer setStorage;

	TSwapStoragePlayer swapStorage;
	TGetWindow getWindow;
	TGetDownTimeChunk	getDownTime;
	TGetCollectionLength getCollectionLength;



};





struct playback{

	//a definir
	void *availableChunks;
	void *scheduleChunks;


};


//Window
struct window{

	void *data;

	TSchedulingWindow scheluding;
	//TIsFitInWindow isFitInWindow;
	THasWindow has;
	TShowWindow show;

	TGetLevelStorageWindow getLevelStorage;
	TGetAvailabilityWindow getAvailability;
	TGetSizeWindow getSize;

	TGetOccupancyWindow getOccupancy;
	TGetBufferWindow getBuffer;
	TGetOccupBufferWindow getOccupBuffer;
	TGetLastPlaybackedObj getLastPlaybackedObj;
	TGetLastChunkAvailable getLastChunkAvailable;
	TGetPlaybackedTime getPlaybackedTime;
	TGetRemainingPlayingTime getRemainingPlayingTime;

	TGetNumberOfStoredObjectWindow getNumberOfStoredObject;

	TSetStorageWindow setStorage;
	TSetOccupancyWindow setOccupancy;
	TSetOccupBufferWindow setOccupBuffer;

	TSetPlaybackedTime setPlaybackedTime;
	TSetLastPlaybackedObj setLastPlaybackedObj;
	TSetLastChunkAvailable setLastChunkAvailable;

	TResetWindow resetWindow;
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



// Politica generica
// Policy related declaration
//
typedef void TSWMPolicy;

// General Policy stuff declaration
//typedef short (* TSWMReplaceGeneralPolicy)(void* systemData, void* cache, void* object);
typedef void *(* TSWMReplaceGeneralPolicy)(void* peer, void* video, void* listPeers, void** picked, void* policy);
typedef short (* TSWMUpdateGeneralPolicy)(void* systemData, void* cache, void* object);
typedef short (* TSWMisFitInSwindowGeneralPolicy)(void* player, void** object);


//
typedef struct GeneralPolicySwm TGeneralPolicySwm;

// Policy related data/function definition
//
typedef struct SWMGeneralPolicy TSWMGeneralPolicy;

struct SWMGeneralPolicy{
	TSWMReplaceGeneralPolicy Replace; // Object Management Policy Replacement(LRU/Popularity)
	TSWMUpdateGeneralPolicy Update; // Object Management Policy Update cache(LRU/Popularity)
	TSWMisFitInSwindowGeneralPolicy FitsInSwindow; // Eligibility criteria

};

typedef void TDATAGeneralPolicySwm;

struct GeneralPolicySwm{
	TSWMGeneralPolicy *SWM;
	TDATAGeneralPolicySwm *data;
};
//fim politica generica


//Greedy
typedef struct GreedyPolicy TGreedyPolicy;
void *createGreedyPolicy(void *entry);
//void *replaceGreedyPolicy(void *peer, void *video, void *listPeers, void **picked);


//short fitsInSwindowGreedyPolicy(void *player, void **object);
//short fitsInSwindowGreedyPolicy(void* player, void** object);

short updateGreedyPolicy(void* systemData,void* cache, void* object);







#endif /* PLAYER_H_ */
