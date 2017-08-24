#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "system.h"
#include "randomic.h"
#include "object.h"
#include "hierarchy.h"
#include "cache.h"
#include "dictionary.h"
#include "internals.h"

static short insertHCache(THCache *hc, int levels, void *object, void* systemData); //@ parametro: levels=nivel a ser acessado
void putCacheInHCache(THCache *hc, int levels,void *vcache);
static short updateHCache(THCache *hc, int levels, void *object,void* systemData);
static short isCacheableHCache(THCache *hc, int levels,void *object, void* systemData);
static void *hasHCache(THCache *hc, void *object);
static short disposeHCache(THCache *hc, int levels);
static void showHCache(THCache *hc, int levels);
static void* searchObjectHCache(THCache* hc, void *vObject, int levelInit, int levelEnd);
static void* searchBiggerVersionHCache(THCache* hc, void *vObject, int levelInit, int levelEnd);



static TAvailabilityHCache getAvailabilityHCache(THCache* hc, int levels);
static TSizeCache getSizeHCache(THCache* hc, int levels);
static void showStatsHCache(THCache* hc, int levels);
static void* getDisposedObjectsHCache(THCache* hc, int levels);
static void* getObjectsHCache(THCache* hc, int level);
static int getLevelsHCache(THCache* hc);
static int getLevelPrincipalHCache(THCache* hc);
static int getLevelReplicateHCache(THCache* hc);
static int getLevelStorageHCache(THCache* hc);
static void* getCache(THCache *hc, int levels );
static void* getStatsHCache(THCache *hc, int levels );
static TOccupancyHCache getOccupancyHCache(THCache *hc, int levels);
static void addAvailabilityHCache(THCache* hc, int levels, TAvailabilityHCache amount);
static unsigned int getNumberOfStoredObjectHCache(THCache* hc, int levels);

typedef struct _data_hc TDataHCache;

struct _data_hc{
	TCache **hcache;
	int levels;
	int levelPrincipal;
	int levelReplicate;
	int levelStorage;

	TListObject *objects; // hcd objects
	TListObject *disposed; // disposed objects by OMPolicyReplacement

};

//segunda
static TDataHCache *initDataHCache(int levels){ //@
//static TDataHCache *initDataHCache(int levels, int levelPrincipal){ //@

	TDataHCache *data = malloc(sizeof(TDataHCache));

	data->hcache = malloc(levels * sizeof(TCache*));
	data->levels = levels;
	data->levelPrincipal=0;
	data->levelReplicate=1;
	data->levelStorage=2;

	return data;
}



THCache *createHCache(int levels){
	THCache *hc = (THCache *)malloc(sizeof(THCache));

	hc->data = initDataHCache(levels);

	hc->insert=insertHCache;
	hc->putCache=putCacheInHCache;
	hc->update=updateHCache;
	hc->isHCacheable=isCacheableHCache;

	//sets and gets
	hc->show = showHCache;
	hc->search=searchObjectHCache;
	hc->searchBiggerVersion=searchBiggerVersionHCache;
	hc->dispose = disposeHCache;
	hc->has = hasHCache;
	hc->getAvailability=getAvailabilityHCache;
	hc->getSize=getSizeHCache;
	hc->getEvictedObjects=getDisposedObjectsHCache;
	hc->getObjects=getObjectsHCache;
	hc->getLevels=getLevelsHCache;
	hc->getLevelPrincipal=getLevelPrincipalHCache;
	hc->getLevelReplicate=getLevelReplicateHCache;
	hc->getLevelStorage=getLevelStorageHCache;
	hc->getCache=getCache;
	hc->getStats = getStatsHCache;
	hc->addAvailability=addAvailabilityHCache;
	hc->getOccupancy = getOccupancyHCache;
	hc->showStats = showStatsHCache;
	hc->getNumberOfStoredObject = getNumberOfStoredObjectHCache;

	return hc;

}

static short insertHCache(THCache *hc, int levels, void *object, void *systemData){
	short status;
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];

	status=cache->insert(cache,object,systemData);

	return status;
}

//@ poe a cache na hierarquia
void putCacheInHCache(THCache *hc, int levels,void *vcache){

	TCache* cache=vcache;
	TDataHCache *data = hc->data;
	data->hcache[levels]=cache;
}

static short updateHCache(THCache *hc, int levels, void *object, void* systemData){
	short status;
	TDataHCache *data = hc->data;
	int lprincipal=hc->getLevelPrincipal(hc);
	TCache *cache=data->hcache[lprincipal];

	status = cache->update(cache, object,systemData);

	return status;
}

static short isCacheableHCache(THCache *hc, int levels, void *object, void* systemData){
	short status;
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];

	status = cache->isCacheable(cache, object,systemData);

	return status;
}

static void showHCache(THCache *hc, int levels ){//Modificar para mostrar o conteudo por nivel
	TDataHCache *data = hc->data;
	TCache *cache;
	TListObject *listObject;
	int i;
	//lprincipal=hc->getLevelPrincipal;

	if(levels!=-1){
		cache=data->hcache[levels];
		listObject = cache->getObjects(cache);

		listObject->show(listObject);

		printf("\n");

	}else{

		for(i=0;i<hc->getLevels(hc);i++){
			printf("\n");
			printf("Objects in level %d of the memory hierarchy	\n",i);
			cache=data->hcache[i];
			listObject = cache->getObjects(cache);
			listObject->show(listObject);
			printf("\n");
		}
	}

}

//Returns a status that points out whether or not
//objects were disposed from passed hc
static short disposeHCache(THCache* hc, int levels ){

	short status = 0;
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];

		status = cache->dispose(cache);

	return status;
}


static void *hasHCache(THCache *hc, void *object){
	TDataHCache *data = hc->data;
	int slevel,elevel;
	 int levels=hc->getLevels(hc);
	TCache *cache;
	TObject *videoFound=NULL;
	//short found=0;

		slevel=0;
		elevel=levels;
		while(slevel<elevel && videoFound==NULL){
			cache=data->hcache[slevel];
			videoFound = cache->has(cache, object);
			slevel++;
		}

	return videoFound;
}

static TOccupancyHCache getOccupancyHCache(THCache *hc, int levels){
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];
	return cache->getOccupancy(cache);
}

static TAvailabilityHCache getAvailabilityHCache(THCache* hc, int levels){
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];
	return cache->getAvailability(cache);
}

static TSizeCache getSizeHCache(THCache* hc, int levels){
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];
	return cache->getSize(cache);
}

static unsigned int getNumberOfStoredObjectHCache(THCache* hc, int levels){
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];
	return cache->getNumberOfStoredObject (cache);
}


static void* getDisposedObjectsHCache(THCache* hc, int levels){
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];
	return cache->getEvictedObjects(cache);
}

static void* getObjectsHCache(THCache* hc, int level){ //@ Tratar com urgencia 02_11_16,existe a necessidade do getObject?
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[level];
	return cache->getObjects(cache);
}

static int getLevelsHCache(THCache* hc){
	TDataHCache *data = hc->data;
	return data->levels;
}

static int getLevelPrincipalHCache(THCache* hc){
	TDataHCache *data = hc->data;
	return data->levelPrincipal;
}

static int getLevelReplicateHCache(THCache* hc){
	TDataHCache *data = hc->data;
	return data->levelReplicate;
}

static int getLevelStorageHCache(THCache* hc){
	TDataHCache *data = hc->data;
	return data->levelStorage;
}

static void* getCache(THCache *hc, int levels ){
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];
	return cache;
}

static void* getStatsHCache(THCache *hc, int levels ){
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];
	return cache->getStats(cache);
}

static void* searchObjectHCache(THCache* hc, void *vObject, int levelInit, int levelEnd){ //@

	TCache *cache;
	TObject *object = vObject;
	int i;
	TObject *storedObject=NULL;
	TListObject *listObject;

	i=levelInit;
	if (levelInit < 0 || levelInit >  levelEnd) {
		printf("hierarchy.c:searchObjectHCache,Erro: Incorrect search range \n");
		exit (0);
	}else{
		while(i<levelEnd && storedObject == NULL){

			cache=hc->getCache(hc,i);
			listObject = cache->getObjects(cache);
			if(listObject!=NULL){
				//storedObject = listObject->getObjectSegment(listObject, object);//
				storedObject = listObject->getObject(listObject, object);//
				if(storedObject != NULL){
					setFoundLevelObject(storedObject,i);
				}
			}
			i++;
		}
	}
	return storedObject;
}

static void *searchBiggerVersionHCache(THCache *hc, void *vObject , int levelInit, int levelEnd){ //@

	TCache *cache;
	int i,bigger=0;
	TObject *storedObject=NULL, *auxObject;
	TListObject *listObject;
	TObject *object=vObject;
	auxObject=object;

	i=levelInit;
	if (levelInit < 0 || levelInit >  levelEnd) {
		printf("hierarchy.c:searchObjectHCache,Erro: Incorrect search range, #line 373 \n");
		exit (0);
	}else{
		while(i<levelEnd){


			cache=hc->getCache(hc,i);
			listObject = cache->getObjects(cache);
			if(listObject!=NULL){
				auxObject = listObject->getBiggerVersion(listObject, auxObject);//
				if(auxObject!=NULL){
					if(getLengthBytesObject(auxObject) > bigger){
						bigger=getLengthBytesObject(auxObject);
						storedObject=auxObject;
						setFoundLevelObject(storedObject,i);
					}

				}else{
					if(storedObject!=NULL){
					auxObject=storedObject;
					}
				}
			}

			i++;
		}
	}
	return storedObject;
}



static void addAvailabilityHCache(THCache* hc, int levels, TAvailabilityHCache amount){
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];
	cache=cache->data;

		cache->addAvailability(cache,amount);
}

static void showStatsHCache(THCache* hc, int levels){
	TDataHCache *data = hc->data;
	TCache *cache=data->hcache[levels];

	cache->showStats(cache);
}

/*
int main(){
    int i;
    THCache *hc, int levels, int levels;
    TObject *object;
    char idVideo[400], time[6], colon;
    int views, ratings, min, sec;
    float stars;

 // hc with 2000 seconds And LRU Object MaNaGement policy

    hc = initHCache(200000, LRUHCache);

    while(!feof(stdin)){

        scanf("%s", idVideo);
        scanf("%d%d",&min,&sec);
        scanf("%d", &views);
        scanf("%d", &ratings);
        scanf("%f", &stars);

        object = initObject(idVideo, (min*60+sec), views, 0);
        insertHCache(hc, object);

        printf("%d %d\n", getAvailabilityHCache(hc), getSizeHCache(hc));

    };

}

 */
