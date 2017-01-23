/*
 * player.c
 *
 *  Created on: Nov 10, 2016
 *      Author: kratos
 */
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "system.h"
#include "player.h"
#include "object.h"
#include "hierarchy.h"
#include "community.h"
#include "time.h"
#include "peer.h"
#include "event.h"
#include "hash.h"
#include "system.h"

#include "datasource.h"
#include "cache.h"
#include "symtable.h"
#include "internals.h"
#include "topology.h"

#include "search.h"
#include "channel.h"




//prototipos de funcoes Player
static void processBuffering(unsigned int idPeer, THashTable* hashTable, TCommunity* community, TSystemInfo* systemData);
static void playbackWindow(TPlayer *player);
static short stallPlayback(TPlayer *player);
//static short storageBuffered(TPlayer *player, void *object);
static void setStorageWindow(TWindow *window, short storage);
//static void SetOccupancyWindow(TPlayer *player, float lenghtObject);
static short swapStorageSwindow(TPlayer *player);
//static THCache *getLevelStoragePlayer(TPlayer *player);
static short getLevelStorageWindow(TWindow *window);

//static TSizeWindow getSizeWindow(TPlayer *player);

////prototipos de funcoes Window
static short schedulingChunk(TWindow *window, void *object);
static short isFitInWindow(TWindow *window, TObject *object);
static void SetOccupancyWindow(TWindow *window, float lenghtObject);
static TSizeWindow getSizeWindow(TWindow *window);
static TWindow *getWindow(TPlayer *player);
static TOccupancyWindow getOccupancyWindow(TWindow *window);
static unsigned int getNumberOfStoredObjectWindow(TWindow *window);
static void showWindow(TWindow *window);
static short hasWindow(TWindow *window, void *object);
static TAvailabilityWindow getAvailabilityWindow(TWindow *window);
//static void buffering(unsigned int idPeer, void* hashTable, void* community, void* systemData);


//static TPlayback *createPlayback(TSizeWindow size);



typedef struct _data_player TDataPlayer; //
struct _data_player{
	TListObject *buffer; // window objects
	THCache *storage;
	//TPlayback *playback;
	TWindow *window;


	void *policy;
};

static TDataPlayer *initDataPlayer(TSizeWindow size, void *policy ){
	TDataPlayer *data = malloc(sizeof(TDataPlayer));

	data->buffer = createListObject();
	data->window = createWindow(size,policy);
	data->policy = policy;



	return data;
}

// CREATE WINDOW
typedef struct _data_window TDataWindow; //
struct _data_window{
	TSizeWindow size; // in seconds)
	float occupancy;
	short lStorage;
	TArrayDynamic **scheduledChunks;//alterar para tipo array dinamico que aponta para lista circular
	void *policy;
};

static TDataWindow *initDataWindow(TSizeWindow size, void *policy ){
	TDataWindow *data = malloc(sizeof(TDataWindow));

	data->size = size;
	data->occupancy = 0.0;
	data->lStorage = 2;
	data->policy = policy;
	return data;
}
TWindow *createWindow(TSizeWindow size, TSchedulingPolicy *policy ){
	TWindow *window = (TWindow *)malloc(sizeof(TWindow));

	window->data = initDataWindow(size, policy);




	//window->request = requestChunk;		 // dispara requição/requisições
	window->scheluding = schedulingChunk;		 // dispara requição/requisições
	window->isFitInWindow = isFitInWindow; 		//verifica se item cabe no tempo da janela
	window->show = showWindow;
	window->has = hasWindow;

	//gets
	window->getAvailability = getAvailabilityWindow;
	window->getSize = getSizeWindow;

	window->getLevelStorage = getLevelStorageWindow;
	window->getOccupancy = getOccupancyWindow;
	window->getNumberOfStoredObject = getNumberOfStoredObjectWindow;
	//sets
	window->setStorage = setStorageWindow;
	window->setOccupancy = SetOccupancyWindow;


	return window;

}

//
TPlayer *createPlayer(TSizeWindow size, TSchedulingPolicy *policy ){
	TPlayer *player = (TPlayer *)malloc(sizeof(TPlayer));

	player->data = initDataPlayer(size, policy);

	player->buffering = processBuffering;
	//player->scheluding = schedulingChunk;		 // dispara requição/requisições
	//player->isFitInWindow = isFitInWindow; 		//verifica se item cabe no tempo da janela
	player->playback = playbackWindow;	 	//reproduz janela ( o que houver disponivel )
	player->stall = stallPlayback;		// parada por falta de segmentos a reproduzir
	player->swapStorage = swapStorageSwindow; 		//troca os objetos de nivel de cache apos reproducao
	player->getWindow = getWindow;
	//player->show = showWindow;
	//player->has = hasWindow;
	//gets
	//player->getAvailability = getAvailabilityWindow;
	//player->getSize = getSizeWindow;

	//player->getLevelStorage = getLevelStoragePlayer;
	//player->getOccupancy = getOccupancyWindow;
	//player->getNumberOfStoredObject = getNumberOfStoredObjectWindow;
	//sets
	//player->setStorage = setStoragePlayer;


	return player;

}

//Functions
static void processBuffering(unsigned int idPeer, THashTable* hashTable, TCommunity* community, TSystemInfo* systemData){
	TObject *video, *cloneVideo;
	TListObject *listEvicted;
	//TPeer *serverPeer;
	//TArrayDynamic *listPeers;
	TDataSource *dataSource;
	TItemHashTable *item;
	TIdObject idVideo;
	TPlayer *player;
	TWindow *window;
	int version=6;
	long int segment=0;
	//float sizeWindow;
	TPeer *peer = community->getPeer(community, idPeer);
	player = peer->getPlayer(peer);
	window = player->getWindow(player);

	short lStorage=window->getLevelStorage(window);
	dataSource = peer->getDataSource(peer);
	//sizeWindow = window->getSize(window);

	video = dataSource->pickFromAdaptive(dataSource,version,segment);

	while(window->getAvailability(window) >= getLengthObject(video)){

		window->setOccupancy(window, getLengthObject(video));
		cloneVideo=cloneObject(video);

		if ( peer->insertCache( peer, cloneVideo , systemData, lStorage ) ){
			//insert array
			getIdObject(video, idVideo);

			item = createItemHashTable();
			item->set(item, idPeer, peer, idVideo, video);
			hashTable->insert(hashTable, item);
			item->dispose(item);

			// updating hash table due to evicting that made room for the cached video
			listEvicted = peer->getEvictedCache(peer);
			hashTable->removeEvictedItens(hashTable, idPeer, listEvicted);

		}

		segment++;
		video = dataSource->pickFromAdaptive(dataSource,version,segment);

	}
}

static short schedulingChunk(TWindow *window, void *object){
	short status=0;
/*	TDataCache *data = cache->data;
	TGeneralPolicy *policy = data->policy;

	TListObject *disposed = data->disposed;
	disposed->cleanup(disposed);*/

	//verificar se objeto já não está disponivel

	//status = policy->SWM->request(systemData, cache, object);

	return status;
}


static short isFitInWindow(TWindow *window, TObject *object){
	short status=0;
	TDataWindow *dataWindow=window->data;
	//TDataPlayer *data = player->data;
	//TGeneralPolicy *policy = data->policy;

	//status = policy->WM->isFitInSwindow(systemData, cache, object);

	return status;
}

static void playbackWindow(TPlayer *player){
	TDataPlayer *data = player->data;

	//player->resetOccupancy(player, 0.0);

}



static short stallPlayback(TPlayer *player){
	short status=0;


	return status;
}
static void setStorageWindow(TWindow *window, short storage){
	//TDataPlayer *data = player->data;
	TDataWindow *data=window->data;

	data->lStorage = storage;
}

static void SetOccupancyWindow(TWindow *window, float lenghtObject){
/*	TDataPlayer *data = player->data;
	TWindow *window = data->window;*/
	TDataWindow *dataWindow=window->data;
	//window->setOccupancy(lenghtObject);

	dataWindow->occupancy+=lenghtObject;
}



static short swapStorageSwindow(TPlayer *player){
	short status=0;


	return status;
}

static void showWindow(TWindow *window){



}

static short hasWindow(TWindow *window, void *object){
	short status=0;


	return status;
}

static TAvailabilityWindow getAvailabilityWindow(TWindow *window){

	float sizeWindow = window->getSize(window),occupWindow = window->getOccupancy(window);

	return sizeWindow-occupWindow;
}

static TSizeWindow getSizeWindow(TWindow *window){
	TDataWindow *data = window->data;
	//TDataPlayer *data = player->data;

		return data->size;
}

static TWindow *getWindow(TPlayer *player){
	TDataPlayer *data = player->data;

		return data->window;
}

static short getLevelStorageWindow(TWindow *window){
	TDataWindow *data = window->data;
	//TDataPlayer *data = player->data;

		return data->lStorage;
}

static TOccupancyWindow getOccupancyWindow(TWindow *window){

	//TWindow *window = player->getWindow(player);
	TDataWindow *data = window->data;
	return data->occupancy;

}
static unsigned int getNumberOfStoredObjectWindow(TWindow *window){
	TDataWindow *data = window->data;

	unsigned int numberOfStoredObjectSwindow;

	return numberOfStoredObjectSwindow;
}
/*static TPlayback *createPlayback(TSizeWindow size){
	TPlayback *playback;

		return playback;
}*/


// Políticas de escalonamento



// Greedy Policy

// Greedy Policy
typedef struct SWMGreedyPolicy TSWMGreedyPolicy;
struct SWMGreedyPolicy{
	//
	TSWMReplaceGeneralPolicy Replace; // Object Management Policy Replacement(Greedy/Popularity)
	TSWMUpdateGeneralPolicy Update; // Object Management Policy Update cache(Greedy/Popularity)
	TSWMisFitInSwindowGeneralPolicy IsFitInSwindow; // Object Management Policy Eligibility

};

typedef void TDATAGreedyPolicy;

struct GreedyPolicy{
	TSWMGreedyPolicy *SWM;
	TDATAGreedyPolicy *data;
};


void *createGreedyPolicy(void *entry){

	TGreedyPolicy *policy = (TGreedyPolicy *) malloc(sizeof( TGreedyPolicy ) );

	policy->SWM = (TSWMGreedyPolicy *) malloc(sizeof( TSWMGreedyPolicy ) );

	policy->data = NULL;

	// init dynamics
	policy->SWM->Replace = replaceGreedyPolicy; // Object Management Policy Replacement(Greedy/Popularity)
	policy->SWM->Update = updateGreedyPolicy; // Object Management Policy Update cache(Greedy/Popularity)
	//policy->SWM->IsFitInSwindow = isFitInSwindowGreedyPolicy;

	return policy;
}



//Returns a status that points out whether or not
short replaceGreedyPolicy(void* vSysInfo, void* cache, void* object){
	TSystemInfo *sysInfo = vSysInfo;
	short status = 0;


		setLastAccessObject(object,sysInfo->getTime(sysInfo));



	return status;
}

//Returns a status that points out whether or not
short updateGreedyPolicy(void* xSysInfo,void* cache, void* object){
	TSystemInfo *sysInfo = xSysInfo;
	short status = 0;

		setLastAccessObject(object,sysInfo->getTime(sysInfo));

	status = 1;

	//    printf("Call Greedy\n");

	return status;
}

short isFitInSwindowGreedyPolicy(TPlayer* player, TObject* object){
	short status = 0;
	float availabilityWindow;
	TWindow *window = player->getWindow(player);

	availabilityWindow = window->getAvailability(window);


	if(availabilityWindow >= getLengthObject(object)){
		status=1;
	}


	return status;
}




//
