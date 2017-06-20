/*
 * hierarchy.h
 *
 *  Created on: Sep 7, 2016
 *      Author: kratos
 */

#ifndef HIERARCHY_H_
#define HIERARCHY_H_

//HCache related implementation
typedef unsigned int TAvailabilityHCache;
typedef unsigned int TSizeHCache;
typedef unsigned int TOccupancyHCache;
typedef struct hcache THCache;

THCache *createHCache(int levels);

typedef void* (* TFirstKHCache)(THCache *hc, int k);
typedef short (* TUpdateHCache)(THCache *hc, int levels, void *object, void *SystemData);
typedef short (* TInsertHCache)(THCache *hc, int levels, void *object, void *SystemData);
typedef void (* TPutCacheInHCache)(THCache *hc, int levels,void* cache);

typedef short (* TIsHCacheableHCache)(THCache *hc, void *object, void *SystemData);
typedef short (* THasHCache)(THCache *hc, void *object);
typedef short (* TDisposeHCache)(THCache* hc);
typedef void (* TShowHCache)(THCache* hc);
typedef void* (*TSearchHCache)(THCache* hc, void *vObject, int levelInit, int levelEnd);
typedef void* (*TSearchBiggerVersionHCache)(THCache* hc, void *vObject, int levelInit, int levelEnd);

typedef TAvailabilityHCache (* TGetAvailabilityHCache)(THCache* hc);
typedef TSizeHCache (* TGetSizeHCache)(THCache* hc);
typedef void* (* TGetStatsHCache)(THCache* hc, int levels);//
typedef void (* TShowStatsHCache)(THCache* hc, int levels);
typedef void* (* TGetDisposedObjectsHCache)(THCache* hc, int levels);
typedef void* (* TGetObjectsHCache)(THCache* hc, int levels);
typedef void* (* TGetCache)(THCache* hc, int levels);
typedef int (* TGetLevelsHCache)(THCache* hc);
typedef int (* TGetLevelPrincipalHCache)(THCache* hc);
typedef int (* TGetLevelReplicateHCache)(THCache* hc);
typedef int (* TGetLevelStorageHCache)(THCache* hc);

typedef TOccupancyHCache (* TGetOccupancyHCache)(THCache *hc);
typedef void (* TAddAvailabilityHCache)(THCache* hc, TAvailabilityHCache amount);

typedef unsigned int (* TGetNumberOfStoredObjectHCache)(THCache* hc, int levels);

struct hcache{
	//private data
	void *data;

	//public methods
	TFirstKHCache firstK;
	TUpdateHCache update;
	TInsertHCache insert;
	TPutCacheInHCache putCache;
	TIsHCacheableHCache isHCacheable;
	THasHCache has;
	TDisposeHCache dispose;
	TShowHCache show;
	TSearchHCache search;
	TSearchBiggerVersionHCache searchBiggerVersion;

	//gets and sets
	TGetAvailabilityHCache getAvailability;
	TGetSizeHCache getSize;
	TGetStatsHCache getStats;
	TShowStatsHCache showStats;
	TGetDisposedObjectsHCache getEvictedObjects;
	TGetObjectsHCache getObjects;
	TGetLevelsHCache getLevels;
	TGetLevelPrincipalHCache getLevelPrincipal;
	TGetLevelReplicateHCache getLevelReplicate;
	TGetLevelStorageHCache getLevelStorage;
	TGetCache getCache;
	TGetOccupancyHCache getOccupancy;
	TAddAvailabilityHCache addAvailability;

	TGetNumberOfStoredObjectHCache getNumberOfStoredObject;


};


#endif /* HIERARCHY_H_ */
