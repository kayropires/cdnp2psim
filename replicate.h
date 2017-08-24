/*
 * replicate.h
 *
 *  Created on: Sep 20, 2011
 *      Author: cesar
 */
#ifndef REPLICATE_H_
#define REPLICATE_H_

float getBufferFractionReplicate(void *replicate);
/*
//Policy Replicate
void *createReplicateRandom(char *pars);
void *createReplicateMinimumWarranty(char *pars);
void *createReplicateNone(char *pars);



typedef struct replicate TReplicate;

struct replicate{

    void *data;
};
*/

/*



//Object related definitions
//
//Stats related definition
typedef unsigned int THitStatsCache;
typedef unsigned int TMissStatsCache;
typedef unsigned int TMaxOccupancyStatsCache;
typedef unsigned int TCommunityHitStatsCache;
typedef double TByteCommunityHitStatsCache;
typedef double TByteHitStatsCache;
typedef double TByteMissStatsCache;

//Stats related definition
typedef struct statsCache TStatsCache;

TStatsCache *createStatsCache();

typedef THitStatsCache (* TGetHitStatsCache)(TStatsCache *stats);
typedef TMissStatsCache (* TGetMissStatsCache)(TStatsCache *stats);
typedef TMaxOccupancyStatsCache (* TGetMaxOccupancyStatsCache)(TStatsCache *stats);
typedef TCommunityHitStatsCache (* TGetCommunityHitStatsCache)(TStatsCache *stats);
typedef TByteCommunityHitStatsCache (* TGetByteCommunityHitStatsCache)(TStatsCache *stats);
typedef TByteHitStatsCache (* TGetByteHitStatsCache)(TStatsCache *stats);
typedef TByteMissStatsCache (* TGetByteMissStatsCache)(TStatsCache *stats);

typedef void (* TSetHitStatsCache)(TStatsCache *stats, THitStatsCache hit);
typedef void (* TSetMissStatsCache)(TStatsCache *stats, TMissStatsCache miss);
typedef void (* TSetMaxOccupancyStatsCache)(TStatsCache *stats, TMaxOccupancyStatsCache ocuppancy);
typedef void (* TSetCommunityHitStatsCache)(TStatsCache *stats, TCommunityHitStatsCache communityHits);
typedef void (* TSetByteCommunityHitStatsCache)(TStatsCache *stats, TByteCommunityHitStatsCache ByteCommunityHit);
typedef void (* TSetByteHitStatsCache)(TStatsCache *stats, THitStatsCache hit);
typedef void (* TSetByteMissStatsCache)(TStatsCache *stats, TMissStatsCache miss);

typedef void (* TAddHitStatsCache)(TStatsCache *stats, THitStatsCache amount);
typedef void (* TAddMissStatsCache)(TStatsCache *stats, TMissStatsCache amount);
typedef void (* TAddMaxOccupancyStatsCache)(TStatsCache *stats, TMaxOccupancyStatsCache amount);
typedef void (* TAddCommunityHitStatsCache)(TStatsCache *stats, TCommunityHitStatsCache amount);
typedef void (* TAddByteCommunityHitStatsCache)(TStatsCache *stats, TByteCommunityHitStatsCache amount);
typedef void (* TAddByteHitStatsCache)(TStatsCache *stats, TByteHitStatsCache amount);
typedef void (* TAddByteMissStatsCache)(TStatsCache *stats, TByteMissStatsCache amount);


struct statsCache{
	//private data
	void *data;

	//public methods (adds, gets, sets)
	TGetHitStatsCache getHit;
	TGetMissStatsCache getMiss;
	TGetMaxOccupancyStatsCache getMaxOccupancy;
	TGetCommunityHitStatsCache getCommunityHit;
	TGetByteCommunityHitStatsCache getByteCommunityHit;
	TGetByteHitStatsCache getByteHit;
	TGetByteMissStatsCache getByteMiss;

	TSetHitStatsCache setHit;
	TSetMissStatsCache setMiss;
	TSetMaxOccupancyStatsCache setMaxOccupancy;
	TSetCommunityHitStatsCache setCommunityHit;
	TSetByteCommunityHitStatsCache setByteCommunityHit;
	TSetByteHitStatsCache setByteHit;
	TSetByteMissStatsCache setByteMiss;

	TAddHitStatsCache addHit;
	TAddMissStatsCache addMiss;
	TAddMaxOccupancyStatsCache addMaxOccupancy;
	TAddCommunityHitStatsCache addCommunityHit;
	TAddByteCommunityHitStatsCache addByteCommunityHit;
	TAddByteHitStatsCache addByteHit;
	TAddByteMissStatsCache addByteMiss;

};
*/




//Cache related implementation


typedef struct replicate TReplicate;


//
//Replicate Related functions
//
TReplicate *createReplicate(void *policy );
TReplicate *createDataReplicate(float bfraction, void *policy);


typedef void (* TRunReplicate)(TReplicate* replicate, void* hashTable, void* community, void* systemData);


struct replicate{
	//private data
	void *data;

	//public methods

	TRunReplicate run;

	//TGetStatsReplicate getStats;
	//TShowStatsReplicate showStats;
};

//void runReplicate(TReplicate* replicate, THashTable* hashTable, TCommunity* community, TSystemInfo* systemData);

// Policy related declaration

typedef void TRMPolicy;

// General Policy stuff declaration


typedef void (* TRMReplicationGeneralPolicy)(void* hashTable, void* community, void* systemData);



typedef struct GeneralRMPolicy TGeneralRMPolicy;

// Policy related data/function definition

typedef struct RMGeneralPolicy TRMGeneralPolicy;

struct RMGeneralPolicy{

	TRMReplicationGeneralPolicy replication;

};

typedef void TDATAGeneralRMPolicy;

struct GeneralRMPolicy{
	TRMGeneralPolicy *RM;
	TDATAGeneralRMPolicy *data;
};



//Replicate

//MRW
typedef struct MRWPolicy TMRWPolicy;
typedef float TBfractionMRWPolicy;

//Minimum Replication warranty policy
void *createMRWPolicy(void *entry);
//void runMinimumReplicationWarrantyPolicy(void* hashTable, void* community, void* systemData);



typedef struct replicationBasedNetworkStatePolicy TRBNSPolicy;
typedef float TBfractionRBNSPolicy;

void *createReplicationBasedNetworkStatePolicy(void *entry);

//
typedef struct replicationGreedyPolicy TRGreedyPolicy;
typedef float TBfractionRGreedyPolicy;

void *createReplicationGreedyPolicy(void *entry);
//














































































































































/*typedef void TDATAReplicate;
typedef int * TListNeighborsReplicate;


typedef short (* TPolicyReplicate)(TReplicate* replicate, void* cache, void* systemData, void* hashTable, void** peers);

TPolicyReplicate getPolicyReplicate(TReplicate *replicate);
void *getCycleReplicate(TReplicate *replicate);
short RunReplicate(TReplicate *replicate, void *cache, void* systemData, void *hastTable, void** peers);



typedef struct DATAReplicateRandomic TRandomicDATAReplicate;
void *initReplicateRandomic(short peer, int neighbors, int range, void *cycle, int maxReplicas, int topK);
int getNumNeighborsReplicateRandomic(TReplicate *replicate);
TListNeighborsReplicate getListNeighborsReplicateRandomic(TReplicate *replicate);
int getMaxReplicasReplicateRandomic(TReplicate *replicate);
int getTopKReplicateRandomic(TReplicate *replicate);*/




#endif /* REPLICATE_H_ */
