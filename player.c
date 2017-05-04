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
//static short schedulingWindow(long int chunkNumber, TPlayer *player, void *listPeer);
static void *schedulingWindow(TPeer *peer, void *video, void *listPeer,void **picked);
//static float playbackWindow(TPlayer *player, TPeer *peer,TSystemInfo* systemData);
static float playbackWindow(TPlayer *player,THashTable* hashTable, TPeer *peer,TSystemInfo* systemData);
static short stallPlayback(TPlayer *player);
//static short storageBuffered(TPlayer *player, void *object);
static void setStorageWindow(TWindow *window, short storage);
//static void SetOccupancyWindow(TPlayer *player, float lenghtObject);
static short swapStorageSwindow(TPlayer *player);
//static THCache *getLevelStoragePlayer(TPlayer *player);
static short getLevelStorageWindow(TWindow *window);
//static TSizeWindow getSizeWindow(TPlayer *player);
static float getDownTimeChunk(TPeer *peer, TPlayer *player, float lengthBytes);
static long int getCollectionLength(TDataSource *dataSource);


////prototipos de funcoes Window
//static short schedulingChunk(TWindow *window, void *listPeer);
//static short schedulingChunk(long int chunkNumber, TWindow *window, void *listPeer);
static void *schedulingChunk(TPeer *peer, void *video, void *listPeer,void **picked);

//static short isFitInWindow(TWindow *window, TObject *object);
static void setOccupancyWindow(TWindow *window, float lenghtObject);
static void setOccupBufferWindow(TWindow *window, float lenghtObject);

static void setPlaybackedTimeWindow(TWindow *window, float lenghtObject);
static void setLastPlaybackedObjWindow(TWindow *window, long int numberObject);
static void setLastChunkAvailableWindow(TWindow *window, long int objectNumber);
static TSizeWindow getSizeWindow(TWindow *window);
static long int getLastPlaybackedObjWindow(TWindow *window);
static long int getLastChunkAvailableWindow(TWindow *window);
static float getPlaybackedTimeWindow(TWindow *window);
static float getRemainingPlayingTimeWindow(TWindow *window);
static TWindow *getWindowPlayer(TPlayer *player);
static TOccupancyWindow getOccupancyWindow(TWindow *window);
static TBufferWindow getBufferWindow(TWindow *window);
static TOccupBufferWindow getOccupBufferWindow(TWindow *window);
static unsigned int getNumberOfStoredObjectWindow(TWindow *window);
static void showWindow(TWindow *window);
static short hasWindow(TWindow *window, void *object);
static TAvailabilityWindow getAvailabilityWindow(TWindow *window);
static void resetWindow(TWindow *window );


//static void buffering(unsigned int idPeer, void* hashTable, void* community, void* systemData);
//static TPlayback *createPlayback(TSizeWindow size);

//
void *replaceGreedyPolicy(TPeer *peer, TObject *video, TArrayDynamic *listPeers, TObject **picked,TGeneralPolicySwm *policy);
//short fitsInSwindowGreedyPolicy(void *player, void **object);
short fitsInSwindowGreedyPolicy(TPlayer* player, TObject** object);
//

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
	float playbackedTime;
	long int lastPlaybackedObj;
	long int lastChunkAvailable;
	short lStorage;
	float buffer;
	float occupBuffer;

	TArrayDynamic **scheduledChunks;//alterar para tipo array dinamico que aponta para lista circular
	void *policy;
};

static TDataWindow *initDataWindow(TSizeWindow size, void *policy ){
	TDataWindow *data = malloc(sizeof(TDataWindow));

	data->size = size;
	data->occupancy = 0;
	data->playbackedTime=0;
	data->lastPlaybackedObj=-1;
	data->lastChunkAvailable=-1;
	data->lStorage = 2;
	data->buffer = size;
	data->occupBuffer = 0;
	data->policy = policy;
	return data;
}

static void resetWindow(TWindow *window ){
	TDataWindow *data = window->data;

	//data->size = size;
	data->occupancy = 0;
	data->playbackedTime=0;
	data->lastPlaybackedObj=-1;
	data->lastChunkAvailable=-1;
	//data->lStorage = 2;
	//data->buffer = size;
	data->occupBuffer = 0;
	//data->policy = policy;
	//return data;
}


TWindow *createWindow(TSizeWindow size, TSchedulingPolicy *policy ){
	TWindow *window = (TWindow *)malloc(sizeof(TWindow));

	window->data = initDataWindow(size, policy);

	//window->request = requestChunk;		 // dispara requição/requisições
	window->scheluding = schedulingChunk;		 // escalona segmentos
	//window->isFitInWindow = isFitInWindow; 		//verifica se item cabe no tempo da janela
	window->show = showWindow;
	window->has = hasWindow;

	//gets
	window->getAvailability = getAvailabilityWindow;
	window->getSize = getSizeWindow;
	window->getLevelStorage = getLevelStorageWindow;
	window->getOccupancy = getOccupancyWindow;
	window->getBuffer = getBufferWindow;
	window->getOccupBuffer = getOccupBufferWindow;
	window->getLastPlaybackedObj = getLastPlaybackedObjWindow;
	window->getLastChunkAvailable = getLastChunkAvailableWindow;
	window->getPlaybackedTime = getPlaybackedTimeWindow;
	window->getRemainingPlayingTime = getRemainingPlayingTimeWindow;


	window->getNumberOfStoredObject = getNumberOfStoredObjectWindow;
	//sets
	window->setStorage = setStorageWindow;
	window->setOccupancy = setOccupancyWindow;
	window->setOccupBuffer = setOccupBufferWindow;
	window->setPlaybackedTime = setPlaybackedTimeWindow;
	window->setLastPlaybackedObj = setLastPlaybackedObjWindow;
	window->setLastChunkAvailable = setLastChunkAvailableWindow;

	window->resetWindow = resetWindow;

	return window;
}

//
TPlayer *createPlayer(TSizeWindow size, TSchedulingPolicy *policy ){
	TPlayer *player = (TPlayer *)malloc(sizeof(TPlayer));

	player->data = initDataPlayer(size, policy);

	player->buffering = processBuffering;
	player->scheluding = schedulingWindow;		 // dispara requição/requisições
	//player->isFitInWindow = isFitInWindow; 		//verifica se item cabe no tempo da janela
	player->playback = playbackWindow;	 	//reproduz janela ( o que houver disponivel )
	player->stall = stallPlayback;		// parada por falta de segmentos a reproduzir
	player->swapStorage = swapStorageSwindow; 		//troca os objetos de nivel de cache apos reproducao
	player->getWindow = getWindowPlayer;
	player->getDownTime = getDownTimeChunk;
	player->getCollectionLength = getCollectionLength;

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


//############################################################################
//Functions Player

static void processBuffering(unsigned int idPeer, THashTable* hashTable, TCommunity* community, TSystemInfo* systemData){
	TObject *video;
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
	float buffer=0, occupBuffer;
	short lStorage;

	//float sizeWindow;
	TPeer *peer = community->getPeer(community, idPeer);
	player = peer->getPlayer(peer);
	window = player->getWindow(player);
	buffer = window->getBuffer(window);


	lStorage=window->getLevelStorage(window);
	dataSource = peer->getDataSource(peer);
	//sizeWindow = window->getSize(window);

	video = dataSource->pickFromAdaptive(dataSource,version,segment);

	while((buffer-(window->getOccupBuffer(window))) >= getLengthObject(video)){


		//cloneVideo=cloneObject(video);

		//############# verificar se a hash já é atualizada no insert ########
		if ( peer->insertCache( peer, cloneObject(video) , systemData, lStorage ) ){
			//insert array
			getIdObject(video, idVideo);

			item = createItemHashTable();
			item->set(item, idPeer, peer, idVideo, video);
			hashTable->insert(hashTable, item);
			item->dispose(item);

			// updating hash table due to evicting that made room for the cached video
			listEvicted = peer->getEvictedCache(peer);
			hashTable->removeEvictedItens(hashTable, idPeer, listEvicted);

			//
			//window->setOccupancy(window, getLengthObject(video));
			window->setOccupBuffer(window, getLengthObject(video));
			window->setLastChunkAvailable(window,segment);

		}else{
			printf("PANIC: Error on Buffering procedure \n");
		}

		segment++;
		video = dataSource->pickFromAdaptive(dataSource,version,segment);

	}
}

static void *schedulingWindow(TPeer *peer, void *video, void *listPeer,void **picked){
	TPlayer *player;
	TWindow *window;
	TPeer *ServerPeer;

	player=peer->getPlayer(peer);
	window=player->getWindow(player);
	ServerPeer=window->scheluding(peer,video,listPeer,picked);




	return ServerPeer;
}



static float playbackWindow(TPlayer *player,THashTable* hashTable, TPeer *peer,TSystemInfo* systemData){


	TDataSource *dataSource;
	THCache *hc;
	TCache *cache;
	TWindow *window=player->getWindow(player);
	int i,levelInit,levelEnd,levelStorage,bigger=0;
	long int lastPlaybackedObject;
	TObject *storedObject=NULL, *auxObject,*aux2;
	TListObject *listObject;
	float lengthObject=0;

	/*Initiator*/
	hc=peer->getHCache(peer);
	levelInit=0;
	levelEnd=hc->getLevels(hc);
	levelStorage=hc->getLevelStorage(hc);
	lastPlaybackedObject = window->getLastPlaybackedObj(window);
	dataSource = peer->getDataSource(peer);

	i=levelInit;


	if(lastPlaybackedObject <= (dataSource->getCollectionLength(dataSource)-1)){
		auxObject = dataSource->pickFromAdaptive(dataSource,0,lastPlaybackedObject+1);
		aux2 = auxObject;

		while(i<levelEnd){


			cache=hc->getCache(hc,i);
			listObject = cache->getObjects(cache);
			if(listObject!=NULL){
				auxObject = listObject->getBiggerVersion(listObject, auxObject);//
				if(auxObject!=NULL){
					if(getLengthBytesObject(auxObject) > bigger){
						bigger=getLengthBytesObject(auxObject);
						storedObject=auxObject;
						aux2=storedObject;
						setFoundLevelObject(storedObject,i);
					}

				}else{
					if(aux2!=NULL){
						auxObject=aux2;
					}
				}
			}

			i++;
		}
	}else{

		printf(" Video played completely!\n");
	}

	if(storedObject!=NULL){
		lengthObject = getLengthObject(storedObject);
		window->setPlaybackedTime(window,lengthObject);
		if(window->getPlaybackedTime(window) > window->getSize(window))
		window->setOccupancy(window,-lengthObject);
		window->setLastPlaybackedObj(window,lastPlaybackedObject+1);

	}


	return lengthObject;
}

static short stallPlayback(TPlayer *player){
	short status=0;


	return status;
}

static short swapStorageSwindow(TPlayer *player){
	short status=0;


	return status;
}

static TWindow *getWindowPlayer(TPlayer *player){
	TDataPlayer *data = player->data;

		return data->window;
}


static float getDownTimeChunk(TPeer *peer, TPlayer *player, float lengthBytes){
	//TDataPlayer *data = player->data;
	TChannel *channel;
	TLink *downLink;
	float CurrentDownRate,downTime ;


		channel = peer->getChannel(peer);
		downLink = channel->getDownLink(channel);
		CurrentDownRate = downLink->getCurrentRate(downLink);

		downTime = (lengthBytes/1000)/CurrentDownRate;

		return downTime;
}

static long int getCollectionLength(TDataSource *dataSource){
	//getCollectionLengthDataSource(dataSource);

		return getCollectionLengthDataSource(dataSource);
}



//############################################################################
//funcoes window

static void *schedulingChunk(TPeer *peer, void *video, void *listPeer,void **picked){
	TPeer *p;
	TPlayer *player;
	TWindow *window;

	player=peer->getPlayer(peer);
	window=player->getWindow(player);
		TDataWindow *data = window->data;
		TGeneralPolicySwm *policy = data->policy;


		p = policy->SWM->Replace(peer,video,listPeer, picked,policy);



	return p;
}


/*static short isFitInWindow(TWindow *window, TObject *object){
	short status=0;
	TDataWindow *dataWindow=window->data;
	//TDataPlayer *data = player->data;
	//TGeneralPolicy *policy = data->policy;

	//status = policy->WM->isFitInSwindow(systemData, cache, object);

	return status;
}*/


static void setStorageWindow(TWindow *window, short storage){
	//TDataPlayer *data = player->data;
	TDataWindow *data=window->data;

	data->lStorage = storage;
}

static void setOccupancyWindow(TWindow *window, float lenghtObject){
/*	TDataPlayer *data = player->data;
	TWindow *window = data->window;*/
	TDataWindow *dataWindow=window->data;
	//window->setOccupancy(lenghtObject);
	float occupWindow = dataWindow->occupancy;
	if ((occupWindow +lenghtObject) < 0){
		dataWindow->occupancy = 0;
	}else{
		dataWindow->occupancy+=lenghtObject;
	}

}

//
static void setOccupBufferWindow(TWindow *window, float lenghtObject){
/*	TDataPlayer *data = player->data;
	TWindow *window = data->window;*/
	TDataWindow *dataWindow=window->data;
	//window->setOccupancy(lenghtObject);

	dataWindow->occupBuffer+=lenghtObject;
}

//
static void setPlaybackedTimeWindow(TWindow *window, float lenghtObject){
/*	TDataPlayer *data = player->data;
	TWindow *window = data->window;*/
	TDataWindow *dataWindow=window->data;
	//window->setOccupancy(lenghtObject);

	dataWindow->playbackedTime+=lenghtObject;
}
//

static void setLastPlaybackedObjWindow(TWindow *window, long int objectNumber){
/*	TDataPlayer *data = player->data;
	TWindow *window = data->window;*/
	TDataWindow *dataWindow=window->data;
	//window->setOccupancy(lenghtObject);

	dataWindow->lastPlaybackedObj=objectNumber;
}
//
static void setLastChunkAvailableWindow(TWindow *window, long int objectNumber){
/*	TDataPlayer *data = player->data;
	TWindow *window = data->window;*/
	TDataWindow *dataWindow=window->data;
	//window->setOccupancy(lenghtObject);

	dataWindow->lastChunkAvailable=objectNumber;
}

//

static void showWindow(TWindow *window){



}

static short hasWindow(TWindow *window, void *object){
	short status=0;


	return status;
}

static TAvailabilityWindow getAvailabilityWindow(TWindow *window){

	float sizeWindow = window->getSize(window);
	float occupWindow = window->getOccupancy(window);

	return sizeWindow-occupWindow;
}

static TSizeWindow getSizeWindow(TWindow *window){
	TDataWindow *data = window->data;
	//TDataPlayer *data = player->data;

		return data->size;
}

static short getLevelStorageWindow(TWindow *window){
	TDataWindow *data = window->data;
	//TDataPlayer *data = player->data;

		return data->lStorage;
}

static TOccupancyWindow getOccupancyWindow(TWindow *window){

	//TWindow *window = player->getWindowPlayer(player);
	TDataWindow *data = window->data;
	return data->occupancy;

}

static TBufferWindow getBufferWindow(TWindow *window){

	//TWindow *window = player->getWindowPlayer(player);
	TDataWindow *data = window->data;
	return data->buffer;

}
static TOccupBufferWindow getOccupBufferWindow(TWindow *window){

	//TWindow *window = player->getWindowPlayer(player);
	TDataWindow *data = window->data;
	return data->occupBuffer;

}



static long int getLastPlaybackedObjWindow(TWindow *window){

	//TWindow *window = player->getWindowPlayer(player);
	TDataWindow *data = window->data;
	return data->lastPlaybackedObj;

}
//

static long int getLastChunkAvailableWindow(TWindow *window){

	//TWindow *window = player->getWindowPlayer(player);
	TDataWindow *data = window->data;
	return data->lastChunkAvailable;

}

//
static float getPlaybackedTimeWindow(TWindow *window){
	TDataWindow *data = window->data;
	return data->playbackedTime;

}
//
static float getRemainingPlayingTimeWindow(TWindow *window){
	TDataWindow *data = window->data;
/*	float remainingPlayingTime;

	//return data->occupancy - data->playbackedTime;
	if(data->occupancy <= data->size){
		remainingPlayingTime=data->size;
	}else{
		remainingPlayingTime=data->occupBuffer;
	}*/
	return data->size-data->occupancy;

}

//
static unsigned int getNumberOfStoredObjectWindow(TWindow *window){
	TDataWindow *data = window->data;

	unsigned int numberOfStoredObjectSwindow;

	return numberOfStoredObjectSwindow;
}


// Políticas de escalonamento

// Greedy Policy
typedef struct SWMGreedyPolicy TSWMGreedyPolicy;
struct SWMGreedyPolicy{
	//
	TSWMReplaceGeneralPolicy Replace; // Object Management Policy Replacement(Greedy/Popularity)
	TSWMUpdateGeneralPolicy Update; // Object Management Policy Update cache(Greedy/Popularity)
	TSWMisFitInSwindowGeneralPolicy FitsInSwindow; // Object Management Policy Eligibility

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
	policy->SWM->FitsInSwindow = fitsInSwindowGreedyPolicy;

	return policy;
}



//Returns a status that points out whether or not
void *replaceGreedyPolicy(TPeer *peer, TObject *video, TArrayDynamic *listPeers, TObject **picked, TGeneralPolicySwm *policy){
	TPeer *serverPeer,*candidateServer;
	TPlayer *player;
	TWindow *window;
	TChannel *channel;
	TLink *downLink;
	TObject **auxPicked;
	float CurrentDownRate, lengthBytes,lengthBytesAux,downTime,remainingPlayingTime;

	//TArrayDynamic *listPeers;

	//listPeers=listofPeers;
	int occup,i;
	short fitsInWindow=0;
	//unsigned int IDPeer;

	occup=listPeers->getOccupancy(listPeers);
	//if(occup==0){
	serverPeer=NULL;
	//}else{

	//colocar aux para picked e candidateServer para comparar com os seguintes
	candidateServer = listPeers->getElement(listPeers,0);
	*picked = candidateServer->hasCachedBiggerVersion(candidateServer,video);
	player=peer->getPlayer(peer);
	window=player->getWindow(player);
	fitsInWindow = policy->SWM->FitsInSwindow(player,picked);
	channel = peer->getChannel(peer);
	downLink = channel->getDownLink(channel);
	CurrentDownRate = downLink->getCurrentRate(downLink);
	lengthBytes = getLengthBytesObject(*picked);
	downTime = (lengthBytes/1000)/CurrentDownRate;
	remainingPlayingTime = window->getRemainingPlayingTime(window);

	if(fitsInWindow == 1 && (remainingPlayingTime > downTime)){

		serverPeer=candidateServer;

	}else{
		*picked=NULL;
		lengthBytes=0;
	}

	for (i=1;i<occup;i++){
		candidateServer = listPeers->getElement(listPeers,i);//lista de pares candidatos zerada
		*auxPicked=candidateServer->hasCachedBiggerVersion(candidateServer,video);
		//player=peer->getPlayer(peer);
		//window=player->getWindowPlayer(player);
		fitsInWindow = policy->SWM->FitsInSwindow(player,auxPicked);
		//channel = peer->getChannel(peer);
		//downLink = channel->getDownLink(channel);
		//CurrentDownRate = downLink->getCurrentRate(downLink);
		lengthBytesAux = getLengthBytesObject(*auxPicked);
		//downTime = (lengthBytesAux/1000)/CurrentDownRate;
		downTime = player->getDownTime(peer,player,lengthBytesAux);
		remainingPlayingTime = window->getRemainingPlayingTime(window);

		if((fitsInWindow == 1) && (remainingPlayingTime > downTime) && (lengthBytesAux > lengthBytes)){

			serverPeer=candidateServer;

			float lengthBytes3 = getLengthBytesObject(*picked);
			float lengthBytes4 = getLengthBytesObject(*auxPicked);

			*picked = auxPicked;
			lengthBytes = lengthBytesAux;

			 lengthBytes3 = getLengthBytesObject(*auxPicked);
			 lengthBytes4 = getLengthBytesObject(*auxPicked);

		}
		/*if((fitsInWindow == 1) && (remainingPlayingTime > downTime) && (lengthBytesAux >= lengthBytes) && candidateServer==peer){

			serverPeer=candidateServer;
			*picked = auxPicked;
			lengthBytes = lengthBytesAux;

		}*/

		//status = policy->SWM->hasChannel = isFitInSwindowGreedyPolicy(peer,candidate, *picked);

		//testar o tamanho da janela e canal

		//criar ArrayDinamic com server e picked para futuras analises

		//}

	}

	return serverPeer;
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

short fitsInSwindowGreedyPolicy(TPlayer* player, TObject** object){
	short status = 0;
	float availabilityWindow;
	TWindow *window = player->getWindow(player);

	availabilityWindow = window->getAvailability(window);
	if(*object!=NULL){
		if(availabilityWindow >= getLengthObject(*object)){
			status=1;
		}
	}


	return status;
}




//
