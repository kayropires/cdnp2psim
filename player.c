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
static void *schedulingWindow(TPeer *peer, void *video, void *listPeer,void **picked);
//static float playbackWindow(TPlayer *player,THashTable* hashTable, TPeer *peer,TSystemInfo* systemData);
static float playbackWindow(TCommunity *community, TPlayer *player,THashTable* hashTable, TPeer *peer,TSystemInfo* systemData);
static short stallPlayback(TPlayer *player);
static void setStatusPlayer(TPlayer *player, TStatusPlayer statusPlayer);
static TStatusPlayer getStatusPlayer(TPlayer *player);
static void setStorageWindow(TWindow *window, short storage);
static short swapStorageSwindow(TPlayer *player);
static short getLevelStorageWindow(TWindow *window);
//static float calcDownTimeChunk(TPeer *peer, TPlayer *player, float lengthBytes);
static float calcDownTimeChunk(TPeer *peer, TPlayer *player, float lengthBytes,TPeer *serverPeer);
static float calcDownTimeChunkFromServer(TPeer *peer, TPlayer *player, float lengthBytes);
static long int getCollectionLength(TDataSource *dataSource);


////prototipos de funcoes Window
static void *schedulingChunk(TPeer *peer, void *video, void *listPeer,void **picked);
static void setOccupancyWindow(TWindow *window, float lenghtObject);
static void setOccupBufferWindow(TWindow *window, float lenghtObject);
static void setPlaybackedTimeWindow(TWindow *window, float lenghtObject);
static void setLastPlaybackedObjWindow(TWindow *window, long int numberObject);
static void setLastChunkAvailableWindow(TWindow *window, long int objectNumber);
static void setLastRepresentationWindow(TWindow *window, int lastRepresentation);
static void setDownTimeLastChunkWindow(TWindow *window, float downTime);
static void setInstantFlowWindow(TWindow *window, float instantFlow);
static int getFreqRiWindow(TWindow *window, int lastRepresentation);
static float getDownTimeLastChunkWindow(TWindow *window);
static float getAverageDownTimeWindow(TWindow *window);
static TSizeWindow getSizeWindow(TWindow *window);
static long int getLastPlaybackedObjWindow(TWindow *window);
static long int getLastChunkAvailableWindow(TWindow *window);
static int getLastRepresentationWindow(TWindow *window);
static float getInstantFlowWindow(TWindow *window);
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


typedef struct _data_player TDataPlayer; //
struct _data_player{
	TListObject *buffer; // window objects
	THCache *storage;
	TWindow *window;
	TStatusPlayer status;

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
	int ri;//lastRepresentation
	int freqRi[7];
	float instantFlow;
	float downTime;
	float averageDownTime;
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
	data->ri = 0;
	data->freqRi[0] = 0;
	data->freqRi[1] = 0;
	data->freqRi[2] = 0;
	data->freqRi[3] = 0;
	data->freqRi[4] = 0;
	data->freqRi[5] = 0;
	data->freqRi[6] = 0;
	data->instantFlow = 0;
	data->downTime = 0;
	data->averageDownTime = 0;
	data->lStorage = 2;
	data->buffer = size;
	data->occupBuffer = 0;
	data->policy = policy;
	return data;
}

static void resetWindow(TWindow *window ){
	TDataWindow *data = window->data;

	data->occupancy = 0;
	data->playbackedTime=0;
	data->lastPlaybackedObj=-1;
	data->lastChunkAvailable=-1;
	data->ri = 0;
	data->freqRi[0] = 0;
	data->freqRi[1] = 0;
	data->freqRi[2] = 0;
	data->freqRi[3] = 0;
	data->freqRi[4] = 0;
	data->freqRi[5] = 0;
	data->freqRi[6] = 0;
	data->instantFlow = 0;
	data->downTime = 0;
	data->averageDownTime = 0;
	data->occupBuffer = 0;
}


TWindow *createWindow(TSizeWindow size, TSchedulingPolicy *policy ){
	TWindow *window = (TWindow *)malloc(sizeof(TWindow));

	window->data = initDataWindow(size, policy);


	window->scheluding = schedulingChunk;
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
	window->getLastAvailableChunk = getLastChunkAvailableWindow;
	window->getLastRepresentation = getLastRepresentationWindow;
	window->getFreqRi = getFreqRiWindow;
	window->getInstantFlow = getInstantFlowWindow;
	window->getDownTimeLastChunk = getDownTimeLastChunkWindow;
	window->getAverageDownTime = getAverageDownTimeWindow;
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
	window->setLastRepresentation = setLastRepresentationWindow;
	window->setInstantFlow = setInstantFlowWindow;
	window->setDownTimeLastChunk = setDownTimeLastChunkWindow;

	window->resetWindow = resetWindow;

	return window;
}

//
TPlayer *createPlayer(TSizeWindow size, TSchedulingPolicy *policy ){
	TPlayer *player = (TPlayer *)malloc(sizeof(TPlayer));

	player->data = initDataPlayer(size, policy);

	player->buffering = processBuffering;
	player->scheluding = schedulingWindow;
	player->playback = playbackWindow;
	player->setStatusPlayer = setStatusPlayer;
	player->getStatusPlayer = getStatusPlayer;
	player->stall = stallPlayback;
	player->swapStorage = swapStorageSwindow;
	player->getWindow = getWindowPlayer;
	player->calcDownTime = calcDownTimeChunk;
	player->calcDownTimeFromServer = calcDownTimeChunkFromServer;
	player->getCollectionLength = getCollectionLength;

	return player;

}

//Functions Player

static void processBuffering(unsigned int idPeer, THashTable* hashTable, TCommunity* community, TSystemInfo* systemData){
	TObject *video;
	TListObject *listEvicted;
	TDataSource *dataSource;
	TItemHashTable *item;
	TIdObject idVideo;
	TPlayer *player;
	TWindow *window;
	int version=6;
	long int segment=0;
	float buffer=0, occupBuffer;
	short lStorage;

	TPeer *peer = community->getPeer(community, idPeer);
	player = peer->getPlayer(peer);
	window = player->getWindow(player);
	buffer = window->getBuffer(window);

	lStorage=window->getLevelStorage(window);
	dataSource = peer->getDataSource(peer);

	video = dataSource->pickFromAdaptive(dataSource,version,segment);

	while((buffer-(window->getOccupBuffer(window))) >= getLengthObject(video)){

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



static float playbackWindow(TCommunity *community, TPlayer *player,THashTable* hashTable, TPeer *peer,TSystemInfo* systemData){


	TDataSource *dataSource;
	THCache *hc;
	TCache *cache;
	TWindow *window=player->getWindow(player);
	int i,levelInit,levelEnd,levelReplicate,bigger=0;
	long int lastPlaybackedObject;
	TObject *storedObject=NULL, *auxObject,*aux2;
	TListObject *listObject;
	float lengthObject=0;

	/*Initiator*/
	hc=peer->getHCache(peer);
	levelInit=0;
	levelEnd=hc->getLevels(hc);
	levelReplicate=hc->getLevelReplicate(hc);
	lastPlaybackedObject = window->getLastPlaybackedObj(window);
	dataSource = peer->getDataSource(peer);

	i=levelInit;


	if((lastPlaybackedObject <= (dataSource->getCollectionLength(dataSource)-1)) && ((lastPlaybackedObject+1) <= window->getLastAvailableChunk(window))){
		auxObject = dataSource->pickFromAdaptive(dataSource,0,lastPlaybackedObject+1);
		aux2 = auxObject;

		while(i<levelEnd){

			if( i != levelReplicate){

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
			}
			i++;


		}
	}

	if(storedObject!=NULL){
		lengthObject = getLengthObject(storedObject);
		window->setPlaybackedTime(window,lengthObject);
		if(window->getPlaybackedTime(window) > window->getSize(window))
			window->setOccupancy(window,-lengthObject);
		window->setLastPlaybackedObj(window,lastPlaybackedObject+1);
		char str[200];
		sprintf(str, "PLAYBACK %u %li %d %f \n",peer->getId(peer),getChunkNumber(storedObject), getRepresentationObject(storedObject), systemData->getTime(systemData));
		community->logRecord(community,str);

	}else{
		char str[200];
		sprintf(str, "PLAYBACK %u %li STALL_INIT %f \n",peer->getId(peer),window->getLastPlaybackedObj(window)+1, systemData->getTime(systemData));
		community->logRecord(community,str);

	}


	return lengthObject;
}

static short stallPlayback(TPlayer *player){
	short status=0;


	return status;
}


static void setStatusPlayer(TPlayer *player, TStatusPlayer statusPlayer){
	TDataPlayer *data = player->data;

	data->status = statusPlayer;
}

static TStatusPlayer getStatusPlayer(TPlayer *player){
	TDataPlayer *data = player->data;

	return data->status;
}

static short swapStorageSwindow(TPlayer *player){
	short status=0;


	return status;
}

static TWindow *getWindowPlayer(TPlayer *player){
	TDataPlayer *data = player->data;

		return data->window;
}



/*static float getDownTimeChunk(TPeer *peer, TPlayer *player, float lengthBytes){

	TWindow *window = player->getWindow(player);

	TChannel *channel;
	TLink *downLink;
	float CurrentDownRate,downTime ;


		channel = peer->getChannel(peer);
		downLink = channel->getDownLink(channel);
		CurrentDownRate = downLink->getCurrentRate(downLink);

		downTime = (lengthBytes/1000)/CurrentDownRate;
		window->setDownTimeLastChunk(window,downTime);

		return downTime;
}*/


static float calcDownTimeChunk(TPeer *peer, TPlayer *player, float lengthBytes,TPeer *serverPeer){

	TWindow *window = player->getWindow(player);

	TChannel *channel, *channelServer;
	TLink *downLink, *upLinkServerPeer;
	float currentDownRatePeer,currentUpRateServerPeer ,downRate, downTime ;


		channel = peer->getChannel(peer);
		downLink = channel->getDownLink(channel);

		channelServer = serverPeer->getChannel(serverPeer);
		upLinkServerPeer = channelServer->getUpLink(channelServer);

		currentUpRateServerPeer = upLinkServerPeer->getCurrentRate(upLinkServerPeer);
		currentDownRatePeer = downLink->getCurrentRate(downLink);

		if(currentUpRateServerPeer <= currentDownRatePeer){

			downRate = currentUpRateServerPeer;
		}else{
			downRate = currentDownRatePeer;
		}

		downTime = (lengthBytes/1000)/downRate;


		return downTime;
}
//
static float calcDownTimeChunkFromServer(TPeer *peer, TPlayer *player, float lengthBytes){

	TWindow *window = player->getWindow(player);

	TChannel *channel;
	TLink *downLink;
	float currentDownRatePeer, downTime ;


		channel = peer->getChannel(peer);
		downLink = channel->getDownLink(channel);
		currentDownRatePeer = downLink->getCurrentRate(downLink);

		downTime = (lengthBytes/1000)/currentDownRatePeer;

		return downTime;
}










static long int getCollectionLength(TDataSource *dataSource){
	//getCollectionLengthDataSource(dataSource);

		return getCollectionLengthDataSource(dataSource);
}



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



static void setStorageWindow(TWindow *window, short storage){
	//TDataPlayer *data = player->data;
	TDataWindow *data=window->data;

	data->lStorage = storage;
}

static void setOccupancyWindow(TWindow *window, float lenghtObject){

	TDataWindow *dataWindow=window->data;

	float occupWindow = dataWindow->occupancy;
	if ((occupWindow +lenghtObject) < 0){
		dataWindow->occupancy = 0;
	}else{
		dataWindow->occupancy+=lenghtObject;
	}

}


static void setOccupBufferWindow(TWindow *window, float lenghtObject){

	TDataWindow *dataWindow=window->data;

	dataWindow->occupBuffer+=lenghtObject;
}


static void setPlaybackedTimeWindow(TWindow *window, float lenghtObject){

	TDataWindow *dataWindow=window->data;

	dataWindow->playbackedTime+=lenghtObject;
}
//

static void setLastPlaybackedObjWindow(TWindow *window, long int objectNumber){

	TDataWindow *dataWindow=window->data;


	dataWindow->lastPlaybackedObj=objectNumber;
}

static void setLastChunkAvailableWindow(TWindow *window, long int objectNumber){

	TDataWindow *dataWindow=window->data;
	dataWindow->lastChunkAvailable=objectNumber;
}


static void setLastRepresentationWindow(TWindow *window, int lastRepresentation){

	TDataWindow *dataWindow=window->data;
	dataWindow->ri = lastRepresentation;
	dataWindow->freqRi[lastRepresentation]+=1;
}

//
static void setInstantFlowWindow(TWindow *window, float instantFlow){

	TDataWindow *dataWindow=window->data;
	dataWindow->instantFlow = instantFlow;
}

static void setDownTimeLastChunkWindow(TWindow *window, float downTime){

	TDataWindow *dataWindow=window->data;
	if (dataWindow->averageDownTime > 0) {
		dataWindow->averageDownTime = (0.25 * window->getDownTimeLastChunk(window) + ((1 - 0.25) * downTime));
		}else{
			dataWindow->averageDownTime = downTime;
	}
	dataWindow->downTime = downTime;

}


static void showWindow(TWindow *window){



}

static short hasWindow(TWindow *window, void *object){
	short status=0;


	return status;
}

static TAvailabilityWindow getAvailabilityWindow(TWindow *window){

	float sizeWindow = window->getSize(window);
	float occupWindow = window->getOccupancy(window);

	if((sizeWindow-occupWindow) > 0){
	return sizeWindow-occupWindow;
	}else{
		return 0;
	}
}

static TSizeWindow getSizeWindow(TWindow *window){

	TDataWindow *data = window->data;

	return data->size;
}

static short getLevelStorageWindow(TWindow *window){
	TDataWindow *data = window->data;

		return data->lStorage;
}

static TOccupancyWindow getOccupancyWindow(TWindow *window){

	TDataWindow *data = window->data;
	return data->occupancy;

}

static TBufferWindow getBufferWindow(TWindow *window){

	TDataWindow *data = window->data;
	return data->buffer;

}
static TOccupBufferWindow getOccupBufferWindow(TWindow *window){


	TDataWindow *data = window->data;
	return data->occupBuffer;

}



static long int getLastPlaybackedObjWindow(TWindow *window){

	TDataWindow *data = window->data;
	return data->lastPlaybackedObj;

}


static long int getLastChunkAvailableWindow(TWindow *window){

	TDataWindow *data = window->data;
	return data->lastChunkAvailable;

}

static int getLastRepresentationWindow(TWindow *window){


	TDataWindow *data = window->data;
	return data->ri;

}

static float getInstantFlowWindow(TWindow *window){


	TDataWindow *data = window->data;
	return data->instantFlow;

}

static float getDownTimeLastChunkWindow(TWindow *window){

	TDataWindow *data = window->data;
	return data->downTime;

}
static float getAverageDownTimeWindow(TWindow *window){

	TDataWindow *data = window->data;
	return data->averageDownTime;

}


static int getFreqRiWindow(TWindow *window, int lastRepresentation){


	TDataWindow *data = window->data;
	return data->freqRi[lastRepresentation];

}


//
static float getPlaybackedTimeWindow(TWindow *window){
	TDataWindow *data = window->data;
	return data->playbackedTime;

}
//
static float getRemainingPlayingTimeWindow(TWindow *window){
	TDataWindow *data = window->data;

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

//void *replaceGreedyPolicy(TPeer *peer, TObject *video, TArrayDynamic *listPeers, TObject **picked, TGeneralPolicySwm *policy){
void *replaceGreedyPolicy(void *vpeer, void *video, void *vlistPeers, void **vpicked, void *vpolicy){
	TPeer *peer = vpeer;
	TArrayDynamic *listPeers = vlistPeers;
	TObject *picked = *vpicked;
	TGeneralPolicySwm *policy = vpolicy;

	TPeer *serverPeer,*candidateServer;
	TPlayer *player;
	TWindow *window;
	TChannel *channel;
	TLink *downLink;
	void *auxPicked = NULL;
	float CurrentDownRate, lengthBytes,lengthBytesAux,downTime,remainingPlayingTime;

	int occup,i;
	short fitsInWindow=0;

	occup=listPeers->getOccupancy(listPeers);

	serverPeer=NULL;

	//colocar aux para picked e candidateServer para comparar com os seguintes
	candidateServer = listPeers->getElement(listPeers,0);
	*vpicked = candidateServer->hasCachedBiggerVersion(candidateServer,video);
	lengthBytes = getLengthBytesObject(*vpicked);
	TIdObject id ;
	getIdObject(*vpicked,id);
	player=peer->getPlayer(peer);
	window=player->getWindow(player);
	fitsInWindow = policy->SWM->FitsInSwindow(player,*vpicked);
	channel = peer->getChannel(peer);
	downLink = channel->getDownLink(channel);
	CurrentDownRate = downLink->getCurrentRate(downLink);
	lengthBytes = getLengthBytesObject(*vpicked);
	downTime = (lengthBytes/1000)/CurrentDownRate;
	remainingPlayingTime = window->getRemainingPlayingTime(window);

	if(fitsInWindow == 1 && (remainingPlayingTime > downTime)){

		serverPeer=candidateServer;

	}else{
		*vpicked=NULL;
		lengthBytes=0;
	}

	for (i=1;i<occup;i++){
		candidateServer = listPeers->getElement(listPeers,i);
		auxPicked = candidateServer->hasCachedBiggerVersion(candidateServer,video);
		fitsInWindow = policy->SWM->FitsInSwindow(player,auxPicked);
		lengthBytesAux = getLengthBytesObject(auxPicked);
		downTime = player->calcDownTime(peer,player,lengthBytesAux,candidateServer);
		remainingPlayingTime = window->getRemainingPlayingTime(window);

		if((fitsInWindow == 1) && (remainingPlayingTime > downTime) && (lengthBytesAux > lengthBytes)){

			serverPeer=candidateServer;
			*vpicked = auxPicked;
			lengthBytes = lengthBytesAux;

		}

	}

	return serverPeer;
}

//Returns a status that points out whether or not
short updateGreedyPolicy(void* xSysInfo,void* cache, void* object){
	TSystemInfo *sysInfo = xSysInfo;
	short status = 0;

		setLastAccessObject(object,sysInfo->getTime(sysInfo));

	status = 1;

	return status;
}

short fitsInSwindowGreedyPolicy(void *vplayer, void *object){
	short status = 0;
	float availabilityWindow;
	TPlayer *player = vplayer;
	TWindow *window = player->getWindow(player);

	float lengthBytes = getLengthBytesObject(object);
		TIdObject id ;
		getIdObject(object,id);

	availabilityWindow = window->getAvailability(window);
	if(object!=NULL){
		if(availabilityWindow >= getLengthObject(object)){
			status=1;
		}
	}


	return status;
}//End Greedy policy



// Adapt Policy
typedef struct SWMAdaptPolicy TSWMAdaptPolicy;
struct SWMAdaptPolicy{

	TSWMReplaceGeneralPolicy Replace; // Object Management Policy Replacement(Adapt/Popularity)
	TSWMUpdateGeneralPolicy Update; // Object Management Policy Update cache(Adapt/Popularity)
	TSWMisFitInSwindowGeneralPolicy FitsInSwindow;

};

struct queueElem{
	float elem;
	struct queueElem *next;
};

struct queue{
	struct queueElem *begin;
	struct queueElem *end;
};

TQueue *createQueueAdaptPolicy(){
	TQueue *q = (TQueue*)malloc(sizeof(TQueue));
	if(!q)
		exit(1);
	else{
		q->begin = NULL;
		q->end = NULL;
	}
	return q;
}

int emptyQueueAdaptPolicy(TQueue*q){
	if(q==NULL) return 1;
	if(q->begin==NULL) return 1;
	else return 0;
}

void enqueueAdaptPolicy(TQueue *q, float downTime){
	TQueueElem *node = (TQueueElem*)malloc(sizeof(TQueueElem));
	if(!node)
		exit(1);
	else{
		node->elem = downTime;
		node->next = NULL;
	}

	if(q->end == NULL){
		q->begin = node;
		q->end = node;
	}else{
		q->end->next = node;
		q->end = node;
	}
}
float dequeueAdaptPolicy(TQueue *q){
	if(emptyQueueAdaptPolicy(q)) return 0;
	float firstElemQueueDownTime;

	TQueueElem*node = q->begin;
	firstElemQueueDownTime = node->elem;
	q->begin = q->begin->next;
	if(q->begin==NULL)
		q->end = NULL;
	free(node);
	return firstElemQueueDownTime;
}

struct _data_AdaptPolicy{


	float bLow;
	float bMin;
	float bMax;
	float factorC;
	int rMin;
	int rMax;

	float instantFlow;
	float averageFlow;

	float deltaTime;
	float occupDeltaTime;
	TQueue *queueDownTime;
	TQueue *queueInstantFlow;


	int occupObjectsDeltaTime;


};

struct AdaptPolicy{
	TSWMAdaptPolicy *SWM;
	TDATAAdaptPolicy *data;
};

void *createAdaptPolicy(void *entry){

	TAdaptPolicy *policy = (TAdaptPolicy *) malloc(sizeof( TAdaptPolicy ) );
	policy->SWM = (TSWMAdaptPolicy *) malloc(sizeof( TSWMAdaptPolicy ) );
	policy->data = (TDATAAdaptPolicy *) malloc(sizeof( TDATAAdaptPolicy ) );


	policy->SWM->Replace = replaceAdaptPolicy; // Object Management Policy Replacement(Adapt/Popularity)
	policy->SWM->Update = updateAdaptPolicy; // Object Management Policy Update cache(Adapt/Popularity)
	policy->SWM->FitsInSwindow = fitsInSwindowAdaptPolicy;
	policy->data->queueDownTime = createQueueAdaptPolicy();
	policy->data->queueInstantFlow = createQueueAdaptPolicy();

	TParameters *lp = createParameters(entry, PARAMETERS_SEPARATOR);

	lp->iterator(lp);

	policy->data->deltaTime = atof(lp->next(lp));
	policy->data->factorC = atof(lp->next(lp));
	policy->data->bMin = atof(lp->next(lp));
	policy->data->bLow = atof(lp->next(lp));
	policy->data->bMax = atof(lp->next(lp));
	policy->data->rMax = (atoi(lp->next(lp))-1);
	policy->data->rMin = 0;
	policy->data->averageFlow = 0;
	policy->data->instantFlow = 0;
	policy->data->occupObjectsDeltaTime = 0;
	policy->data->occupDeltaTime = 0;

	lp->dispose(lp);


	return policy;
}


float calcAverageFlowinDeltaTimeAdaptPolicy(TDATAAdaptPolicy *data){

	TQueue *q = data->queueInstantFlow;
	if(emptyQueueAdaptPolicy(q)){
		return 0;
	}

	TQueueElem *aux = q->begin;

	float firstInstantFlow = aux->elem;

	while(aux!=NULL){
		data->averageFlow = (0.25 * firstInstantFlow + ((1 - 0.25) * aux->elem));
		firstInstantFlow = aux->elem;
		aux = aux->next;
	}
	return data->averageFlow;
}

void setOccupDeltaTimeAdaptPolicy(float downTimeLastChunk,float instantFlowLastChunk, TDATAAdaptPolicy *data){

	if(downTimeLastChunk <= (data->deltaTime - data->occupDeltaTime)){
		data->occupDeltaTime+=downTimeLastChunk;
		if(downTimeLastChunk > 0){
		data->occupObjectsDeltaTime+=1;
		enqueueAdaptPolicy(data->queueDownTime,downTimeLastChunk);
		enqueueAdaptPolicy(data->queueInstantFlow,instantFlowLastChunk);
		}

	}else{
		while(downTimeLastChunk > (data->deltaTime - data->occupDeltaTime)){
			data->occupDeltaTime = (data->occupDeltaTime - dequeueAdaptPolicy(data->queueDownTime));
			float firstInstantFlow = dequeueAdaptPolicy(data->queueInstantFlow);
			data->occupObjectsDeltaTime-=1;
		}

	}

}



void updateAverageFlowAdaptPolicy(float instantFlow,TDATAAdaptPolicy *data){
	float averageFlow;

	data->instantFlow = instantFlow;
	if( data->averageFlow > 0){
		data->averageFlow = calcAverageFlowinDeltaTimeAdaptPolicy(data);
	}else{
		data->averageFlow = instantFlow;
	}
}

int moderateAdaptPolicy(float currentBufferLevel, int ri, int fRi, TDATAAdaptPolicy *data){

	int rProx;

	float instantFlow, averageFlow;

	//freqRi in window ok
	//window->getFreqRi ok
	//criar campos para guardar frequencia de representações na janela ok
	//falta obter a vazão media e vazão instantanea


	if(currentBufferLevel > data->bLow && currentBufferLevel < data->bMax){
		if(data->averageFlow*data->factorC > fRi && ri < data->rMax){
			rProx = ri+1;
		}
	}else{
		if(currentBufferLevel > data->bMin && currentBufferLevel < data->bLow ){
			if( data->instantFlow*data->factorC < fRi && ri > data->rMin){
				rProx = ri-1;
			}else{
				if( data->instantFlow*data->factorC > fRi && ri < data->rMax){
					rProx = ri+1;
				}

			}

		}else{
			if( currentBufferLevel <= data->bMin){
				rProx = data->rMin;
			}

		}

	}

	return rProx;
}


void *replaceAdaptPolicy(void *vpeer, void *video, void *vlistPeers, void **vpicked, void *vpolicy){
	TPeer *peer = vpeer;
	TArrayDynamic *listPeers = vlistPeers;
	//TObject *picked = *vpicked;
	TGeneralPolicySwm *policy = vpolicy;
	TDATAAdaptPolicy *dataPolicy = policy->data;
	TDataSource *dataSource;
	dataSource = peer->getDataSource(peer);


	TPeer *serverPeer,*candidateServer;
	TPlayer *player = peer->getPlayer(peer);
	TWindow *window = player->getWindow(player);
	TChannel *channel;
	TLink *downLink;
	void *auxPicked = NULL;
	TObject *videoFound=NULL;
	int occup,i;
	short fitsInWindow=0;
	float CurrentDownRate, lengthBytes,lengthBytesAux,downTime,remainingPlayingTime;
	float currentBufferLevel;
	float occupDeltaTime;
	float deltaTime;
	int lastRepresentation;
	int freqRi;
	//short hasCached=0;
	i=0;

	currentBufferLevel = window->getOccupancy(window);
	lastRepresentation = window->getLastRepresentation(window);
	freqRi = window->getFreqRi(window,lastRepresentation );
	setOccupDeltaTimeAdaptPolicy(window->getDownTimeLastChunk(window),window->getInstantFlow(window),dataPolicy);
	updateAverageFlowAdaptPolicy(window->getInstantFlow(window),dataPolicy);
	occup=listPeers->getOccupancy(listPeers);
	serverPeer=NULL;

	int rProx = moderateAdaptPolicy(currentBufferLevel,lastRepresentation,freqRi,dataPolicy);
	video = dataSource->pickFromAdaptive(dataSource,rProx,getChunkNumber(video));

	while (i < occup && auxPicked == NULL){

		candidateServer = listPeers->getElement(listPeers,i);

		if ((videoFound = candidateServer->hasCached(candidateServer,video))!=NULL){

			fitsInWindow = policy->SWM->FitsInSwindow(player,videoFound);
			lengthBytesAux = getLengthBytesObject(videoFound);
			downTime = player->calcDownTime(peer,player,lengthBytesAux,candidateServer);
			remainingPlayingTime = window->getRemainingPlayingTime(window);

			if(fitsInWindow == 1 && (remainingPlayingTime > downTime) ){
				serverPeer=candidateServer;
				*vpicked = cloneObject(videoFound);
				auxPicked = *vpicked;
				window->setInstantFlow(window,(lengthBytesAux/1000)/downTime);
				window->setDownTimeLastChunk(window,downTime);
			}
		}
		i++;
	}
	if(*vpicked == NULL)
		*vpicked = video;

	return serverPeer;
}

//Returns a status that points out whether or not
short updateAdaptPolicy(void* xSysInfo,void* cache, void* object){
	TSystemInfo *sysInfo = xSysInfo;
	short status = 0;

	setLastAccessObject(object,sysInfo->getTime(sysInfo));
	status = 1;

	//    printf("Call Adapt\n");

	return status;
}

short fitsInSwindowAdaptPolicy(void *vplayer, void *object){
	short status = 0;
	float availabilityWindow;
	TPlayer *player = vplayer;
	TWindow *window = player->getWindow(player);
	float lengthBytes = getLengthBytesObject(object);
	TIdObject id ;
	getIdObject(object,id);

	availabilityWindow = window->getAvailability(window);
	if(object!=NULL){
		if(availabilityWindow >= getLengthObject(object)){
			status=1;
		}
	}


	return status;
}

