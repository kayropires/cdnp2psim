
typedef enum EVENTS {JOIN=1,BUFFERING=2,REQUEST=3, PLAYBACK=4,STALL_PLAY=5,STALL_REQ=6, TOPOLOGY=7,SHOW_TOPOLOGY=8, SHOW_MAPQUERY=9,LEAVE=10,STORE=11,REPLICATE=12,FLUCTUATION=13, FINISHED_VIEWING=14, SHOW_CHANNELS=15} TTypeEvent;

//typedef unsigned int TTimeEvent;
typedef float TTimeEvent;
typedef unsigned int TOwnerEvent;

typedef struct event TEvent;
typedef TTimeEvent (*TGetTimeEvent)(TEvent* event);
typedef TTypeEvent (*TGetTypeEvent)(TEvent* event);
typedef TOwnerEvent (*TGetOwnerEvent)(TEvent* event);
typedef void (*TUFreeEvent)(TEvent* event);

struct event{
	void *data;
	TGetTimeEvent getTime;
	TGetTypeEvent getType;
	TGetOwnerEvent getOwner;
	TUFreeEvent ufree;
};

TEvent *createEvent(TTimeEvent time, TTypeEvent type, TOwnerEvent owner);


