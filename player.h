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
typedef void (* TSetLastRepresentation)(TWindow *window, int lastRepresentation);
typedef void (* TSetInstantFlow)(TWindow *window, float instantFlow);
typedef void (* TSetDownTimeLastChunk)(TWindow *window, float downTime);
typedef void (* TResetWindow)(TWindow *window);

typedef void (* TBufferingPlayer)(unsigned int idPeer, void* hashTable, void* community, void* systemData);
typedef void* (* TSchedulingPlayer)(void *peer, void *video, void *listPeer,void *picked);
typedef float (* TPlaybackPlayer)(void *community, TPlayer *player,void *hashTable, void *peer, void *systemData);
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
//typedef float (* TCalcDownTimeChunk)(void *peer, TPlayer *player, float lenghtObject);
typedef float (* TCalcDownTimeChunk)(void *peer, TPlayer *player, float lengthBytes, void *serverPeer);
typedef float (* TCalcDownTimeFromServer)(void *peer, TPlayer *player, float lengthBytes);



typedef long int (* TGetCollectionLength)(void *datasource);

typedef TOccupancyWindow(* TGetOccupancyWindow)(TWindow *window);
typedef TBufferWindow(* TGetBufferWindow)(TWindow *window);
typedef TOccupBufferWindow(* TGetOccupBufferWindow)(TWindow *window);

typedef long int (* TGetLastPlaybackedObj)(TWindow *window);
typedef long int (* TGetLastChunkAvailable)(TWindow *window);

typedef int (* TGetLastRepresentation)(TWindow *window);
typedef int (* TGetFreqRi)(TWindow *window, int lastRepresentation);
typedef float (* TGetInstantFlow)(TWindow *window);
typedef float (* TGetDownTimeLastChunk)(TWindow *window);
typedef float (* TGetAverageDownTime)(TWindow *window);


typedef float (* TGetPlaybackedTime)(TWindow *window);
typedef float (* TGetRemainingPlayingTime)(TWindow *window);

typedef unsigned int (* TGetNumberOfStoredObjectWindow)(TPlayer* player);

struct player{
	//private data
	void *data;

	//public methods

	TBufferingPlayer buffering; //1
	TSchedulingPlayer scheluding; //1
	TPlaybackPlayer playback; //1
	TStallPlayer stall; //1//sugestao estatisticas
	TSwapStoragePlayer swapStorage;
	TGetWindow getWindow;
	TCalcDownTimeChunk	calcDownTime;
	TCalcDownTimeFromServer calcDownTimeFromServer;
	TGetCollectionLength getCollectionLength;



};

//Window
struct window{

	void *data;

	TSchedulingWindow scheluding;
	THasWindow has;
	TShowWindow show;
	TGetLevelStorageWindow getLevelStorage;
	TGetAvailabilityWindow getAvailability;
	TGetSizeWindow getSize;
	TGetOccupancyWindow getOccupancy;
	TGetBufferWindow getBuffer;
	TGetOccupBufferWindow getOccupBuffer;
	TGetLastPlaybackedObj getLastPlaybackedObj;
	TGetLastChunkAvailable getLastAvailableChunk;
	TGetLastRepresentation getLastRepresentation;
	TGetFreqRi getFreqRi;
	TGetInstantFlow getInstantFlow;
	TGetDownTimeLastChunk getDownTimeLastChunk;
	TGetAverageDownTime getAverageDownTime;
	TGetPlaybackedTime getPlaybackedTime;
	TGetRemainingPlayingTime getRemainingPlayingTime;
	TGetNumberOfStoredObjectWindow getNumberOfStoredObject;
	TSetStorageWindow setStorage;
	TSetOccupancyWindow setOccupancy;
	TSetOccupBufferWindow setOccupBuffer;
	TSetPlaybackedTime setPlaybackedTime;
	TSetLastPlaybackedObj setLastPlaybackedObj;
	TSetLastChunkAvailable setLastChunkAvailable;
	TSetLastRepresentation setLastRepresentation;
	TSetInstantFlow setInstantFlow;
	TSetDownTimeLastChunk setDownTimeLastChunk;

	TResetWindow resetWindow;
};


// Politica generica
// Policy related declaration
//
typedef void TSWMPolicy;

// General Policy stuff declaration
//typedef short (* TSWMReplaceGeneralPolicy)(void* systemData, void* cache, void* object);
typedef void *(* TSWMReplaceGeneralPolicy)(void* peer, void* video, void* listPeers, void** picked, void* policy);
typedef short (* TSWMUpdateGeneralPolicy)(void* systemData, void* cache, void* object);
typedef short (* TSWMisFitInSwindowGeneralPolicy)(void *player, void *object);


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

//Greedy
typedef struct GreedyPolicy TGreedyPolicy;
void *createGreedyPolicy(void *entry);
void *replaceGreedyPolicy(void *peer, void *video, void *listPeers, void **picked, void *policy);
short fitsInSwindowGreedyPolicy(void *vplayer, void *vobject);
short updateGreedyPolicy(void* systemData,void* cache, void* object);


//Adapt
typedef struct queue TQueue;
typedef struct queueElem TQueueElem;
typedef struct AdaptPolicy TAdaptPolicy;
typedef struct _data_AdaptPolicy TDATAAdaptPolicy;
void *createAdaptPolicy(void *entry);
void *replaceAdaptPolicy(void *peer, void *video, void *listPeers, void **picked, void *policy);
short fitsInSwindowAdaptPolicy(void *vplayer, void *vobject);
short updateAdaptPolicy(void* systemData,void* cache, void* object);





#endif /* PLAYER_H_ */
