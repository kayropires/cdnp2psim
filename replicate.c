/*
 * replicate.c
 *
 *  Created on: Sep 18, 2011
 *      Author: cesar
 */

#include "time.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "peer.h"
#include "player.h"
#include "event.h"
#include "hash.h"
#include "system.h"
#include "object.h"
#include "datasource.h"
#include "randomic.h"
#include "replicate.h"
#include "cache.h"
#include "hierarchy.h"
#include "symtable.h"
#include "internals.h"
#include "topology.h"
#include "community.h"
#include "search.h"
#include "channel.h"
#include "dictionary.h"


//void runReplicate(TReplicate* replicate, THashTable* hashTable, TCommunity* community, TSystemInfo* systemData);



// minimum Replication Warranty  Policy
//RM Replication Manager
typedef struct RMMRWPolicy TRMMRWPolicy;
struct RMMRWPolicy{

	TRMReplicationGeneralPolicy replication; // Object Management Policy run cache(MRW/Popularity)

};

//typedef void TDATAMRWPolicy;

typedef struct _data_MRWPolicy TDATAMRWPolicy;
struct _data_MRWPolicy{
	TBfractionMRWPolicy bfraction;
};

struct MRWPolicy{
	TRMMRWPolicy *RM;
	TDATAMRWPolicy *data;
};



void runMinimumReplicationWarrantyPolicy(THashTable* hashTable, TCommunity* community, TSystemInfo* systemData){
	//short status = 0;
	TObject* video;
		TIdObject idVideo;
		TListObject *listEvicted;
		//TPeer* serverPeer;
		TPeer *peer;
		THCache *hc;
		TDataSource* dataSource;
		TItemHashTable *item;
		TPlayer *player;
		TWindow *window;
		//unsigned int idServerPeer;
		TChannel* channel;
		//	float prefetchRate;
		int version=0,lReplicate; //Level Replicate
		long int lastAvailableChunk,replicateChunk;


		int sizeComm = community->getSize(community); //obtem tamanho da comunidade
		unsigned int i;

		printf("Replicacao com garantia minima \n");

		for(i=0;i<sizeComm;i++){

			peer = community->getPeer(community, i);
			player = peer->getPlayer(peer);
			window = player->getWindow(player);
			lastAvailableChunk = window->getLastAvailableChunk(window);
			float availableWindowTime = 0, tempOccup = 0, tempTime = 0,replicateTime ;
			hc = peer->getHCache(peer);
			lReplicate = hc->getLevelReplicate(hc);
			//channel = peer->getChannel(peer);
			//TLink *downLink = channel->getDownLink(channel);
			//float currentDownRate = downLink->getCurrentRate(downLink);
			dataSource = peer->getDataSource(peer);
			availableWindowTime = window->getAvailability(window);
			//replicateTime = peer->getReplicateTime(peer);
			replicateTime = availableWindowTime * 0.5;
			//Enquanto durar o tempo de replicação
			while(tempOccup < availableWindowTime && tempTime <= replicateTime){
			video = dataSource->pickFromAdaptive(dataSource, version, lastAvailableChunk+=1); //escolher até Limite da metade da janela, entao baixar a 2 metade

			//somar o tempTime com os tempos gastos com a replicação, ( tempo de download)
			//Perguntar do par se o conteúdo a ser replicado eh menor que a janela deslizante

				if (video == NULL || !peer->hasDownlink(peer, video, 0))
				return;

	//		if (video == NULL )	return;

			getIdObject(video, idVideo);
			setReplicateObject(video,1);

			if(tempOccup > (availableWindowTime-replicateTime)){

			if (peer->isUp(peer)){

				if ( peer->insertCache( peer, cloneObject(video), systemData, lReplicate ) ){

					item = createItemHashTable();
					item->set(item, i, peer, idVideo, video);
					hashTable->insert(hashTable, item);
					item->dispose(item);

					// updating hash table due to evicting that made room for the cached video
					listEvicted = peer->getEvictedCache(peer);
					hashTable->removeEvictedItens(hashTable, i, listEvicted);
				}


			}

			}
			tempOccup+=getLengthObject(video);
			tempTime+=player->getDownTime(peer,player,((float)getLengthBytesObject(video)));
			//calcular tempo de download
			}//while
		}


	//return status;
}

void *createMRWPolicy(void *entry){

	TMRWPolicy *policy = (TMRWPolicy *) malloc(sizeof( TMRWPolicy ) );
	policy->RM = (TRMMRWPolicy *) malloc(sizeof( TRMMRWPolicy ) );
	policy->data = NULL;
    //TParameters *lp = createParameters(entry, PARAMETERS_SEPARATOR);

   // lp->iterator(lp);

	// init dynamics

	policy->RM->replication = runMinimumReplicationWarrantyPolicy;
	//policy->data->bfraction = atof(lp->next(lp));

	//lp->dispose(lp);

	return policy;
}





//

float getBufferFractionReplicate(TReplicate *replicate){
	TDATAMRWPolicy *data =replicate->data;

	return data->bfraction ;
}







// REPLICATION Management


typedef struct _data_replication{


	void *policy;
	float bfraction;


} TDataReplicate;


static TDataReplicate *initDataReplicate(float bfraction, TRMGeneralPolicy *policy){

	TDataReplicate *data = malloc(sizeof(TDataReplicate));


	data->policy = policy;
	data->bfraction = bfraction;



	return data;
}
static void runReplicate(TReplicate* replicate, void* hashTable, void* community, void* systemData){


	TDataReplicate *data = replicate->data;
	TGeneralRMPolicy *policy = data->policy;


			policy->RM->replication(hashTable, community, systemData);

}

TReplicate *createDataReplicate(float bfraction, void *policy){

	TReplicate *replicate = malloc(sizeof(TReplicate));

	replicate->data = initDataReplicate(bfraction, policy);
	replicate->run = runReplicate;
	//replicate->getCurrentRI = getCurrentReplicationInterval;
	//replicate->getReplicationDuration = getReplicationDuration;



	return replicate;
}















































































































































//

/*

void *getCycleReplicate(TReplicate *replicate){


	return replicate->cycle;
}

float getBufferFractionReplicate(TReplicate *replicate){


	return replicate->bfraction;
}

TPolicyReplicate getPolicyReplicate(TReplicate *replicate){

	return replicate->policy;
}

static void setDATAReplicate(TReplicate *replicate, TDATAReplicate *data){

	replicate->data = data;
}

static TDATAReplicate *getDATAReplicate(TReplicate *replicate){

	return replicate->data;
}


short RunReplicate(TReplicate *replicate, void *cache, void* systemData, void* hashTable, void** peers){
    short status=0;

    TPolicyReplicate policy = getPolicyReplicate(replicate);

    status = policy(replicate, cache, systemData, hashTable, peers);

    // if (status)
    //    printf("Well done!\n");
    //else
    //    printf("Got problems to insert\n");


    return status;
}


typedef struct DATAReplicateRandomic TDATAReplicateRandomic;

struct DATAReplicateRandomic{
    TListNeighborsReplicate listNeighbors;
    int numNeighbors;
    int maxReplicas;
    int topK;
};

static TDATAReplicateRandomic *initDataReplicateRandomic(int numNeighbors, int *listNeighbors, int maxReplicas, int topK){
	TDATAReplicateRandomic * data = malloc(sizeof(TDATAReplicateRandomic));

	data->listNeighbors=listNeighbors;
	data->maxReplicas=maxReplicas;
	data->numNeighbors=numNeighbors;
	data->topK=topK;

	return data;
}

static void destroyDataReplicateRandomic(TDATAReplicateRandomic *data){
	free(data);
}

static short policyReplicateRandomic(TReplicate* replicate, void* cache, void* systemData, void* hashTable, void **peers){
	return 1;
}


void *initReplicateRandomic(short peer, int numNeighbors, int range, void *cycle, int maxReplicas, int topK){
	int i,j;
	int *listNeighbors;
	short status;
	short neighbor;
	TReplicate *replicate;
	char entry[20];

	sprintf(entry,"%d;",range);
    TRandomic *neighbors = NULL; // initPickRandomic("UNIFORM",entry);

	listNeighbors = (int *)malloc(sizeof(int)*numNeighbors);

	// get a list of neighbors peers
	for(i=0;i<numNeighbors;i++){
		do{
			neighbor = neighbors->pick(neighbors) - 1;
			if (neighbor == peer)
				status = 1;
			else{
				j=0;
				while ( (j<i) && (listNeighbors[j] != neighbor) ){
					j++;
				}
				status = (j<i?1:0); // picked peer is in neighbor list
			}

		}while( status );
		listNeighbors[i] = neighbor;
	}

	replicate = (TReplicate *)malloc(sizeof(TReplicate));

	replicate->policy = policyReplicateRandomic;

	replicate->cycle = cycle;

	replicate->data = initDataReplicateRandomic(numNeighbors, listNeighbors, maxReplicas, topK);

	// dispose TRandomic
	neighbors->dispose(neighbors);

	return replicate;
}


void destroyReplicateRandomic(TReplicate* replicate){

	destroyDataReplicateRandomic(replicate->data);
	disposeRandomic(replicate->cycle);
	free(replicate);
}



int getTopKReplicateRandomic(TReplicate *replicate){
	TDATAReplicateRandomic *data = (TDATAReplicateRandomic*)replicate->data;

	return data->topK;

}

int getNumNeighborsReplicateRandomic(TReplicate *replicate){
	TDATAReplicateRandomic *data = (TDATAReplicateRandomic*)replicate->data;

	return data->numNeighbors;

}

TListNeighborsReplicate getListNeighborsReplicateRandomic(TReplicate *replicate){
	TDATAReplicateRandomic *data = (TDATAReplicateRandomic*)replicate->data;

	return data->listNeighbors;

}

int getMaxReplicasReplicateRandomic(TReplicate *replicate){
	TDATAReplicateRandomic *data = (TDATAReplicateRandomic*)replicate->data;

	return data->maxReplicas;
}

typedef struct DATAReplicateTOPK TDATAReplicateTOPK;

struct DATAReplicateTOPK{
    TListNeighborsReplicate listNeighbors;
    int numNeighbors;
    int topK;
};

int getNumNeighborsReplicateTOPK(TReplicate *replicate){
	TDATAReplicateTOPK *data = (TDATAReplicateTOPK*)replicate->data;

	return data->numNeighbors;
}

TListNeighborsReplicate getListNeighborsReplicateTOPK(TReplicate *replicate){
	TDATAReplicateTOPK *data = (TDATAReplicateTOPK*)replicate->data;

	return data->listNeighbors;
}

int getTopKReplicateTOPK(TReplicate *replicate){
	TDATAReplicateTOPK *data = (TDATAReplicateTOPK*)replicate->data;

	return data->topK;
}

void *initReplicateTopK(void *cycle, void *placement, int topK){

	return NULL;
}

// TOP-K policy
short policyReplicateTOPK(TReplicate* replicate, void* cache, void* systemData, void* vHashTable, void **peers) {
	int i, status;
	TPeer *peer;
	unsigned int idPeerNeighbor;
	TObject *object, *cloneObj;
	TCache *replicateCache;
	TIdObject idObject;
	TListObject *listDisposed;
	TItemHashTable *item;
	THashTable *hashTable = vHashTable;

	item = createItemHashTable();

	int numNeighbors = getNumNeighborsReplicateTOPK(replicate);
	TListNeighborsReplicate listNeighbors = getListNeighborsReplicateTOPK(replicate);
	int topK = getTopKReplicateTOPK(replicate);

	TListObject *candidateslistObject = ((TCache *)cache)->firstK(cache, topK); // K = 2

	while (!candidateslistObject->isEmpty(candidateslistObject)) {
		i = 0;

		object = candidateslistObject->getHead(candidateslistObject);
		cloneObj = cloneObject(object);

		status = 0;
		while ((i < numNeighbors)) { //&& (!status)) {

			while ((i < numNeighbors) && (!peer->isUp(peer))){
				i++;
				if (i< numNeighbors)
					peer = peers[listNeighbors[i]];
			}

			if (i < numNeighbors) {

				idPeerNeighbor = listNeighbors[i];
				i++;

				replicateCache = peer->getHCache(peer);

				if (replicateCache->isCacheable(replicateCache, cloneObj, systemData)) {

					if (!replicateCache->has(replicateCache, cloneObj)) { // not found

						if (replicateCache->insert(replicateCache, cloneObj, systemData)) {
							status = 1;

							getIdObject(cloneObj, idObject);
							item->set(item, idPeerNeighbor,peers[idPeerNeighbor], idObject,
									cloneObj);

							hashTable->insert(hashTable, item);

							// updating hash table due to evicting that made room for the cached video
							listDisposed = replicateCache->getEvictedObjects(replicateCache);

							hashTable->removeEvictedItens(hashTable, idPeerNeighbor, listDisposed);

						}
					}
				}
			}

			if (status == 1) // a replica was accepted, get new replica and try to put it on other peer
				cloneObj = cloneObject(object);

		}
		if (status == 0) // object was not accepted
			disposeObject(cloneObj);

		candidateslistObject->removeHead(candidateslistObject);

	}

	candidateslistObject->destroy(candidateslistObject);
	item->dispose(item);

	return (short) 1;
}*/

// Politicas de Replicacao 11/08/16

//RANDOM POLICY REPLICATE
/*

//static void RandomReplicate(THashTable* hashTable, TCommunity* community, TSystemInfo* systemData){
// retirado o static para efeito de teste, construir estruturas posteriormente
void RandomReplicate(THashTable* hashTable, TCommunity* community, TSystemInfo* systemData){
	TObject* video;
	TIdObject idVideo;
	TListObject *listEvicted;
	//TPeer* serverPeer;
	TPeer *peer;
	THCache *hc;
	TDataSource* dataSource;
	TItemHashTable *item;
	//unsigned int idServerPeer;
	//TChannel* channel;
	//	float prefetchRate;
	int lReplicate; //Level Replicate

	int sizeComm = community->getSize(community); //obtem tamanho da comunidade
	unsigned int i;

	printf("Replicacao---------------------------------------------------\n");

	for(i=0;i<sizeComm;i++){

		peer = community->getPeer(community, i);
		hc=peer->getHCache(peer);
		lReplicate=hc->getLevelReplicate(hc);
		//channel = peer->getChannel(peer);
		dataSource = peer->getDataSource(peer);
		video = dataSource->pick(dataSource);
		//prefetchRate = dataSource->getPrefetchRate(dataSource);//@ identificar

		//Perguntar do par se o conteúdo a ser replicado eh menor que a janela deslizante

				if (video == NULL || !peer->hasDownlink(peer, video, prefetchRate))
			return;

		if (video == NULL )	return;

		getIdObject(video, idVideo);
		setReplicateObject(video,1);

		if (peer->isUp(peer)){

			if ( peer->insertCache( peer, cloneObject(video), systemData, lReplicate ) ){

				item = createItemHashTable();
				item->set(item, i, peer, idVideo, video);
				hashTable->insert(hashTable, item);
				item->dispose(item);

				// updating hash table due to evicting that made room for the cached video
				listEvicted = peer->getEvictedCache(peer);
				hashTable->removeEvictedItens(hashTable, i, listEvicted);
			}


		}
	}

}
*/



/*

// NONE POLICY REPLICATE
static void NoneReplicate(TPeer* peer, unsigned int idPeer, THashTable* hashTable, TCommunity* community, TSystemInfo* systemData){

		return;

}
*/



/*
//
void minimumWarrantyReplicate(THashTable* hashTable, TCommunity* community, TSystemInfo* systemData){
	TObject* video;
	TIdObject idVideo;
	TListObject *listEvicted;
	//TPeer* serverPeer;
	TPeer *peer;
	THCache *hc;
	TDataSource* dataSource;
	TItemHashTable *item;
	TPlayer *player;
	TWindow *window;
	//unsigned int idServerPeer;
	TChannel* channel;
	//	float prefetchRate;
	int version=0,lReplicate; //Level Replicate
	long int lastAvailableChunk,replicateChunk;


	int sizeComm = community->getSize(community); //obtem tamanho da comunidade
	unsigned int i;

	printf("Replicacao com garantia minima \n");

	for(i=0;i<sizeComm;i++){

		peer = community->getPeer(community, i);
		player = peer->getPlayer(peer);
		window = player->getWindow(player);
		lastAvailableChunk = window->getLastAvailableChunk(window);
		float availableWindowTime = 0, tempOccup = 0, tempTime = 0,replicateTime ;
		hc = peer->getHCache(peer);
		lReplicate = hc->getLevelReplicate(hc);
		//channel = peer->getChannel(peer);
		//TLink *downLink = channel->getDownLink(channel);
		//float currentDownRate = downLink->getCurrentRate(downLink);
		dataSource = peer->getDataSource(peer);
		availableWindowTime = window->getAvailability(window);
		//replicateTime = peer->getReplicateTime(peer);
		replicateTime = availableWindowTime * 0.5;
		//Enquanto durar o tempo de replicação
		while(tempOccup < availableWindowTime && tempTime <= replicateTime){
		video = dataSource->pickFromAdaptive(dataSource, version, lastAvailableChunk+=1); //escolher até Limite da metade da janela, entao baixar a 2 metade

		//somar o tempTime com os tempos gastos com a replicação, ( tempo de download)
		//Perguntar do par se o conteúdo a ser replicado eh menor que a janela deslizante

				if (video == NULL || !peer->hasDownlink(peer, video, prefetchRate))
			return;

		if (video == NULL )	return;

		getIdObject(video, idVideo);
		setReplicateObject(video,1);

		if(tempOccup > (availableWindowTime-replicateTime)){

		if (peer->isUp(peer)){

			if ( peer->insertCache( peer, cloneObject(video), systemData, lReplicate ) ){

				item = createItemHashTable();
				item->set(item, i, peer, idVideo, video);
				hashTable->insert(hashTable, item);
				item->dispose(item);

				// updating hash table due to evicting that made room for the cached video
				listEvicted = peer->getEvictedCache(peer);
				hashTable->removeEvictedItens(hashTable, i, listEvicted);
			}


		}

		}
		tempOccup+=getLengthObject(video);
		tempTime+=player->getDownTime(peer,player,((float)getLengthBytesObject(video)));
		//calcular tempo de download
		}//while
	}

}
*/


//End Policy Replication



/*
//createReplicateMinimumWarranty
void *createReplicateMinimumWarranty(char *pars) {
	TReplicate *replicate;
	TParameters *lp = createParameters(pars, PARAMETERS_SEPARATOR);

	lp->iterator(lp);

	replicate = (TReplicate*) malloc(sizeof(TReplicate));

	replicate->dynamic = minimumWarrantyReplicate;
	replicate->bfraction = atof(lp->next(lp));
	replicate->swindow = atoi(lp->next(lp));
	lp->dispose(lp);

	return replicate;
}
//


void *createReplicateRandom(char *pars) {
	TReplicate *replicate;
	TParameters *lp = createParameters(pars, PARAMETERS_SEPARATOR);

	lp->iterator(lp);

	replicate = (TReplicate*) malloc(sizeof(TReplicate));

	replicate->dynamic = RandomReplicate;
	replicate->bfraction = atof(lp->next(lp));
	replicate->swindow = atoi(lp->next(lp));
	lp->dispose(lp);

	return replicate;
}

void *createReplicateNone(char *pars) {
	TReplicate *replicate;
	TParameters *lp = createParameters(pars, PARAMETERS_SEPARATOR);

	lp->iterator(lp);

	replicate = (TReplicate*) malloc(sizeof(TReplicate));

	replicate->dynamic = NoneReplicate;
	//replicate->bfraction = atof(lp->next(lp));
	//replicate->swindow = atoi(lp->next(lp));
	lp->dispose(lp);

	return replicate;
}
*/

