#include <cv.h>
#include <queue>
#include <ctime>
#include <cmath>
#include <cstdio>
#include <string>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <iostream>
#include <highgui.h>
#include <algorithm>
#include <AudioToolbox/AudioToolbox.h>
using namespace std;

#define ceps 30
#define river_size 50
#define gap 100
#define MAX_H 1024
#define MAX_W 1024
#define pixR 10
#define firstKey 1000
#define keyGap 1000

string lo,la,z,sc;
char str[100];
int width,height;
int b[MAX_H][MAX_W],c[MAX_H][MAX_W],d[MAX_H][MAX_W];
int mv[4][2]={-1,0, 1,0, 0,-1, 0,1};
int ang[4][2];
int river_x=-1,river_y=-1;
int river_xs=-1,river_ys=-1;

struct PIX
{
    int x,y,frame,key,keyTime;
}pix[MAX_H*MAX_W];

int pl;

struct KEYPIX
{
    int x,y,t;
    CvScalar clr;
}kp[MAX_H*MAX_W];

int kpl;

bool chk(int x,int y)
{
    return x>-1&&x<height&&y>-1&&y<width&&b[x][y]&&!c[x][y];
}

int keyAdd=0,kb=0;

void getKey(int &key,int &keyTime)
{
    key=21+keyAdd;
    kb++;
    if(kb==3)
    {
        keyAdd++;
        kb=0;
    }
    keyTime=keyGap;
}

void getClr()
{
    kp[kpl].clr.val[0]=rand()%256;
    kp[kpl].clr.val[1]=rand()%256;
    kp[kpl].clr.val[2]=rand()%256;
}


#define PRINTERROR(LABEL)	printf("%s err %4.4s %ld\n", LABEL, (char *)&err, err)

const unsigned int kNumAQBufs = 3;			// number of audio queue buffers we allocate
const size_t kAQBufSize = 1024 * 1024;		// number of bytes in each audio queue buffer
const size_t kAQMaxPacketDescs = 512;		// number of packet descriptions in our array

struct MyData
{
	AudioFileStreamID audioFileStream;	// the audio file stream parser
    
	AudioQueueRef audioQueue;								// the audio queue
	AudioQueueBufferRef audioQueueBuffer[kNumAQBufs];		// audio queue buffers
	
	AudioStreamPacketDescription packetDescs[kAQMaxPacketDescs];	// packet descriptions for enqueuing audio
	
	unsigned int fillBufferIndex;	// the index of the audioQueueBuffer that is being filled
	size_t bytesFilled;				// how many bytes have been filled
	size_t packetsFilled;			// how many packets have been filled
    
	bool inuse[kNumAQBufs];			// flags to indicate that a buffer is still in use
	bool started;					// flag to indicate that the queue has been started
	bool failed;					// flag to indicate an error occurred
    
	pthread_mutex_t mutex;			// a mutex to protect the inuse flags
	pthread_cond_t cond;			// a condition varable for handling the inuse flags
	pthread_cond_t done;			// a condition varable for handling the inuse flags
};

typedef struct MyData MyData;

int  MyConnectSocket();

void MyAudioQueueOutputCallback(void* inClientData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer);
void MyAudioQueueIsRunningCallback(void *inUserData, AudioQueueRef inAQ, AudioQueuePropertyID inID);

void MyPropertyListenerProc(	void *							inClientData,
                            AudioFileStreamID				inAudioFileStream,
                            AudioFileStreamPropertyID		inPropertyID,
                            UInt32 *						ioFlags);

void MyPacketsProc(				void *							inClientData,
                   UInt32							inNumberBytes,
                   UInt32							inNumberPackets,
                   const void *					inInputData,
                   AudioStreamPacketDescription	*inPacketDescriptions);

const int bufSize=40000;
char buf[bufSize];

OSStatus MyEnqueueBuffer(MyData* myData);
void WaitForFreeBuffer(MyData* myData);


int main(int argc, const char * argv[])
{
    int i,j,k,l,angl,x,y,kpB;
    srand(time(NULL));
    CvScalar cSW;
    cSW.val[0]=cSW.val[1]=cSW.val[2]=255;
    CvScalar cSB;
    cSB.val[0]=cSB.val[1]=cSB.val[2]=0;
    CvScalar cSL;
    cSL.val[0]=253; cSL.val[1]=242; cSL.val[2]=213;
    CvScalar cSD;
    cSD.val[0]=240; cSD.val[1]=208; cSD.val[2]=178;
    if(argc==4)
    {
        lo=argv[1];
        la=argv[2];
        z=argv[3];
    }
    else
    {
        lo="31.5057";
        la="121.3509";
        z="9";
    }
    sc="curl -o catch.png \"http://maps.googleapis.com/maps/api/staticmap?center="+lo+","+la+"&zoom="+z+"&size=640x640&maptype=roadmap&sensor=false\"";
//    system(sc.c_str());
    IplImage* img=NULL;
    while(!img)
        img=cvLoadImage( "catch.png" );
    
    width=img->width;
    height=img->height;
    ang[0][0]=0; ang[0][1]=0; ang[1][0]=height; ang[1][1]=width;
    ang[2][0]=height; ang[2][1]=0; ang[3][0]=0; ang[4][0]=width;
    
    cvShowImage( "River", img );
    cvWaitKey();
    
    for(i=0;i<height;i++)
    {
        for(j=0;j<width;j++)
        {
            CvScalar cS=cvGet2D(img, i, j);
            if(abs(cS.val[0]-230)<ceps&&abs(cS.val[1]-208)<ceps&&abs(cS.val[2]-158)<ceps)
            {
                cvSet2D(img, i, j, cSW);
            }
        }
    }
    
    cvShowImage( "River", img );
    cvWaitKey();
    cvDilate(img, img);
    cvDilate(img, img);
    cvDilate(img, img);
    cvShowImage("River", img);
    cvWaitKey();
    
    for(i=0;i<height;i++)
    {
        for(j=0;j<width;j++)
        {
            CvScalar cS=cvGet2D(img, i, j);
            if(cS.val[0]>250&&cS.val[1]>250&&cS.val[2]>250)
            {
                b[i][j]=1;
            }
        }
    }
    
    c[0][0]=b[0][0];
    for(i=1;i<height;i++)
    {
        c[i][0]=c[i-1][0]+b[i][0];
    }
    for(i=1;i<width;i++)
    {
        c[0][i]=c[0][i-1]+b[0][i];
    }
    for(i=1;i<height;i++)
    {
        for(j=1;j<width;j++)
        {
            c[i][j]=c[i-1][j]+c[i][j-1]-c[i-1][j-1]+b[i][j];
        }
    }
    
    river_x=river_y=-1;
    for(i=height-1,k=river_size*river_size;i>river_size&&river_x==-1;i--)
    {
        for(j=width-1;j>river_size;j--)
        {
            if(c[i][j]-c[i-river_size][j]-c[i][j-river_size]+c[i-river_size][j-river_size]==k)
            {
                river_x=i;
                river_y=j;
                break;
            }
        }
    }
    if(river_x==-1)
    {
        puts("No river found!");
        return 0;
    }
    printf("River found at (%d,%d).\n",river_x,river_y);
    
    for(i=angl=0,k=0x7fffffff;i<4;i++)
    {
        j=abs(river_x-ang[i][0])+abs(river_y-ang[i][1]);
        if(j<k)
        {
            k=j;
            angl=i^1;
        }
    }
    
    k=0x7fffffff;
    memset(c,0,sizeof c);
    c[river_x][river_y]=1;
    queue<int>Q;
    Q.push(river_x);
    Q.push(river_y);
    while(!Q.empty())
    {
        i=Q.front(); Q.pop();
        j=Q.front(); Q.pop();
        for(l=0;l<4;l++)
        {
            x=i+mv[l][0];
            y=j+mv[l][1];
            if(chk(x,y))
            {
                cvSet2D(img, x, y, cSL);
                if(abs(x-ang[angl][0])+abs(y-ang[angl][1])<k)
                {
                    k=abs(x-ang[angl][0])+abs(y-ang[angl][1]);
                    river_xs=x;
                    river_ys=y;
                }
                c[x][y]=1;
                Q.push(x);
                Q.push(y);
            }
        }
    }
    
    memcpy(b,c,sizeof b);
    
    for(i=0;i<height;i++)
    {
        for(j=0;j<width;j++)
        {
            if(!b[i][j])
                cvSet2D(img, i, j, cSW);
        }
    }
    
    cvShowImage("River", img);
    cvWaitKey(gap);
    
    printf("River starts at (%d,%d).\n",river_xs,river_ys);
    
    int logFrame=0,keyTime=firstKey;
    pl=0;
    memset(c,0,sizeof c);
    c[river_xs][river_ys]=1;
    pix[pl].x=river_xs;
    pix[pl].y=river_ys;
    pl++;
    Q.push(river_xs);
    Q.push(river_ys);
    while(!Q.empty())
    {
        i=Q.front(); Q.pop();
        j=Q.front(); Q.pop();
        for(l=0;l<8;l++)
        {
            if(l<4)
            {
                x=i+mv[l][0];
                y=j+mv[l][1];
            }
            else
            {
                x=i+rand()%50-25;
                y=j+rand()%50-25;
            }
            if(chk(x,y))
            {
                pix[pl].x=x;
                pix[pl].y=y;
                logFrame++;
                if(logFrame>max(200,int(Q.size()/12)))
                {
                    logFrame=0;
                    pix[pl].frame=1;
                    keyTime-=gap;
                    if(keyTime<=0)
                    {
                        getKey(pix[pl].key,pix[pl].keyTime);
                        keyTime=pix[pl].keyTime;
                    }
                }
                pl++;
                c[x][y]=1;
                Q.push(x);
                Q.push(y);
            }
        }
    }
    
    MyData* myData;
    size_t length;
    int time;
    FILE* file;
    
    memset(c,0,sizeof c);
    memset(d,-1,sizeof d);
    kpl=0;
    cvShowImage("River", img);
    cvWaitKey(gap);
    for(i=0;i<pl;i++)
    {
        kpB=0;
        x=pix[i].x;
        y=pix[i].y;
        if(d[x][y]!=-1&&!pix[i].key)
            continue;
        if(!pix[i].key)
        {
            cvSet2D(img, x, y, cSD);
        }
        else
        {
            kp[kpl].x=x;
            kp[kpl].y=y;
            kp[kpl].t=pix[i].keyTime/gap;
            getClr();
            kpl++;
            kpB=1;
        }
        if(pix[i].frame)
        {
            for(j=0;j<kpl;j++)
            {
                x=kp[j].x;
                y=kp[j].y;
                if(!kp[j].t)
                    continue;
                kp[j].t--;
                for(k=x-pixR;k<=x+pixR;k++)
                {
                    for(l=y-pixR;l<=y+pixR;l++)
                    {
                        if(sqrt((k-x)*(k-x)+(l-y)*(l-y))>=pixR)
                            continue;
                        if(!chk(k,l))
                            continue;
                        if(d[k][l]>j)
                            continue;
                        d[k][l]=j;
                        cvSet2D(img, k, l, kp[j].clr);
                    }
                }
                if(kp[j].t==1)
                {
                    kp[j].clr.val[0]=240;
                    kp[j].clr.val[1]=208;
                    kp[j].clr.val[2]=178;
                }
                else
                {
                    kp[j].clr.val[0]+=(240-kp[j].clr.val[0])/6;
                    kp[j].clr.val[1]+=(208-kp[j].clr.val[1])/6;
                    kp[j].clr.val[2]+=(178-kp[j].clr.val[2])/6;
                }
            }
            if(kpB)
            {
                sprintf(str,"%d.mp3",pix[i].key);
                puts(str);
                keyTime=pix[i].keyTime*1000;
                file=fopen(str,"r");
                if(!file)
                    file=fopen("0.mp3","r");
                memset(buf,0,bufSize);
                length=fread(buf, 1, bufSize, file);
                myData = (MyData*)calloc(1, sizeof(MyData));
                AudioFileStreamOpen(myData, MyPropertyListenerProc, MyPacketsProc,kAudioFileAAC_ADTSType,
                                    &myData->audioFileStream);
                AudioFileStreamParseBytes(myData->audioFileStream, length, buf, 0);
                MyEnqueueBuffer(myData);
            }
            cvWaitKey(gap);
            cvShowImage("River", img);
        }
    }
    
    for(i=0;i<height;i++)
    {
        for(j=0;j<width;j++)
        {
            if(b[i][j])
                cvSet2D(img, i, j, cSD);
        }
    }
    
    cvShowImage("River", img);
    cvWaitKey();
    return 0;
}

void MyPropertyListenerProc(	void *							inClientData,
                            AudioFileStreamID				inAudioFileStream,
                            AudioFileStreamPropertyID		inPropertyID,
                            UInt32 *						ioFlags)
{
	// this is called by audio file stream when it finds property values
	MyData* myData = (MyData*)inClientData;
	OSStatus err = noErr;
    
	switch (inPropertyID) {
		case kAudioFileStreamProperty_ReadyToProducePackets :
		{
			// the file stream parser is now ready to produce audio packets.
			// get the stream format.
			AudioStreamBasicDescription asbd;
			UInt32 asbdSize = sizeof(asbd);
			err = AudioFileStreamGetProperty(inAudioFileStream, kAudioFileStreamProperty_DataFormat, &asbdSize, &asbd);
			if (err) { PRINTERROR("get kAudioFileStreamProperty_DataFormat"); myData->failed = true; break; }
			
			// create the audio queue
			err = AudioQueueNewOutput(&asbd, MyAudioQueueOutputCallback, myData, NULL, NULL, 0, &myData->audioQueue);
			if (err) { PRINTERROR("AudioQueueNewOutput"); myData->failed = true; break; }
			
			// allocate audio queue buffers
			for (unsigned int i = 0; i < kNumAQBufs; ++i) {
				err = AudioQueueAllocateBuffer(myData->audioQueue, kAQBufSize, &myData->audioQueueBuffer[i]);
				if (err) { PRINTERROR("AudioQueueAllocateBuffer"); myData->failed = true; break; }
			}
            
			// get the cookie size
			UInt32 cookieSize;
			Boolean writable;
			err = AudioFileStreamGetPropertyInfo(inAudioFileStream, kAudioFileStreamProperty_MagicCookieData, &cookieSize, &writable);
			if (err) { PRINTERROR("info kAudioFileStreamProperty_MagicCookieData"); break; }
			printf("cookieSize %d\n", (unsigned int)cookieSize);
            
			// get the cookie data
			void* cookieData = calloc(1, cookieSize);
			err = AudioFileStreamGetProperty(inAudioFileStream, kAudioFileStreamProperty_MagicCookieData, &cookieSize, cookieData);
			if (err) { PRINTERROR("get kAudioFileStreamProperty_MagicCookieData"); free(cookieData); break; }
            
			// set the cookie on the queue.
			err = AudioQueueSetProperty(myData->audioQueue, kAudioQueueProperty_MagicCookie, cookieData, cookieSize);
			free(cookieData);
			if (err) { PRINTERROR("set kAudioQueueProperty_MagicCookie"); break; }
            
			// listen for kAudioQueueProperty_IsRunning
			err = AudioQueueAddPropertyListener(myData->audioQueue, kAudioQueueProperty_IsRunning, MyAudioQueueIsRunningCallback, myData);
			if (err) { PRINTERROR("AudioQueueAddPropertyListener"); myData->failed = true; break; }
			
			break;
		}
	}
}

void MyPacketsProc(				void *							inClientData,
                   UInt32							inNumberBytes,
                   UInt32							inNumberPackets,
                   const void *					inInputData,
                   AudioStreamPacketDescription	*inPacketDescriptions)
{
	// this is called by audio file stream when it finds packets of audio
	MyData* myData = (MyData*)inClientData;
	printf("got data.  bytes: %d  packets: %d\n", (unsigned int)inNumberBytes, (unsigned int)inNumberPackets);
    
	// the following code assumes we're streaming VBR data. for CBR data, you'd need another code branch here.
    
	for (int i = 0; i < inNumberPackets; ++i) {
		SInt64 packetOffset = inPacketDescriptions[i].mStartOffset;
		SInt64 packetSize   = inPacketDescriptions[i].mDataByteSize;
		
		// if the space remaining in the buffer is not enough for this packet, then enqueue the buffer.
		size_t bufSpaceRemaining = kAQBufSize - myData->bytesFilled;
		if (bufSpaceRemaining < packetSize) {
			MyEnqueueBuffer(myData);
			WaitForFreeBuffer(myData);
		}
		
		// copy data to the audio queue buffer
		AudioQueueBufferRef fillBuf = myData->audioQueueBuffer[myData->fillBufferIndex];
		memcpy((char*)fillBuf->mAudioData + myData->bytesFilled, (const char*)inInputData + packetOffset, packetSize);
		// fill out packet description
		myData->packetDescs[myData->packetsFilled] = inPacketDescriptions[i];
		myData->packetDescs[myData->packetsFilled].mStartOffset = myData->bytesFilled;
		// keep track of bytes filled and packets filled
		myData->bytesFilled += packetSize;
		myData->packetsFilled += 1;
		
		// if that was the last free packet description, then enqueue the buffer.
		size_t packetsDescsRemaining = kAQMaxPacketDescs - myData->packetsFilled;
		if (packetsDescsRemaining == 0) {
			MyEnqueueBuffer(myData);
			WaitForFreeBuffer(myData);
		}
	}
}

OSStatus StartQueueIfNeeded(MyData* myData)
{
	OSStatus err = noErr;
	if (!myData->started) {		// start the queue if it has not been started already
		err = AudioQueueStart(myData->audioQueue, NULL);
		if (err) { PRINTERROR("AudioQueueStart"); myData->failed = true; return err; }
		myData->started = true;
	}
	return err;
}

OSStatus MyEnqueueBuffer(MyData* myData)
{
	OSStatus err = noErr;
	myData->inuse[myData->fillBufferIndex] = true;		// set in use flag
	
	// enqueue buffer
	AudioQueueBufferRef fillBuf = myData->audioQueueBuffer[myData->fillBufferIndex];
	fillBuf->mAudioDataByteSize = myData->bytesFilled;
	err = AudioQueueEnqueueBuffer(myData->audioQueue, fillBuf, myData->packetsFilled, myData->packetDescs);
	if (err) { PRINTERROR("AudioQueueEnqueueBuffer"); myData->failed = true; return err; }
	
	StartQueueIfNeeded(myData);
	
	return err;
}


void WaitForFreeBuffer(MyData* myData)
{
	// go to next buffer
	if (++myData->fillBufferIndex >= kNumAQBufs) myData->fillBufferIndex = 0;
	myData->bytesFilled = 0;		// reset bytes filled
	myData->packetsFilled = 0;		// reset packets filled
    
	// wait until next buffer is not in use
	printf("->lock\n");
	pthread_mutex_lock(&myData->mutex);
	while (myData->inuse[myData->fillBufferIndex]) {
		printf("... WAITING ...\n");
		pthread_cond_wait(&myData->cond, &myData->mutex);
	}
	pthread_mutex_unlock(&myData->mutex);
	printf("<-unlock\n");
}

int MyFindQueueBuffer(MyData* myData, AudioQueueBufferRef inBuffer)
{
	for (unsigned int i = 0; i < kNumAQBufs; ++i) {
		if (inBuffer == myData->audioQueueBuffer[i])
			return i;
	}
	return -1;
}


void MyAudioQueueOutputCallback(	void*					inClientData,
                                AudioQueueRef			inAQ,
                                AudioQueueBufferRef		inBuffer)
{
	// this is called by the audio queue when it has finished decoding our data.
	// The buffer is now free to be reused.
	MyData* myData = (MyData*)inClientData;
    
	unsigned int bufIndex = MyFindQueueBuffer(myData, inBuffer);
	
	// signal waiting thread that the buffer is free.
	pthread_mutex_lock(&myData->mutex);
	myData->inuse[bufIndex] = false;
	pthread_cond_signal(&myData->cond);
	pthread_mutex_unlock(&myData->mutex);
}

void MyAudioQueueIsRunningCallback(		void*					inClientData,
                                   AudioQueueRef			inAQ,
                                   AudioQueuePropertyID	inID)
{
	MyData* myData = (MyData*)inClientData;
	
	UInt32 running;
	UInt32 size;
	OSStatus err = AudioQueueGetProperty(inAQ, kAudioQueueProperty_IsRunning, &running, &size);
	if (err) { PRINTERROR("get kAudioQueueProperty_IsRunning"); return; }
	if (!running) {
		pthread_mutex_lock(&myData->mutex);
		pthread_cond_signal(&myData->done);
		pthread_mutex_unlock(&myData->mutex);
	}
}