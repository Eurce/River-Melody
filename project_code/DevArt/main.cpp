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
#define teps 0
#define river_size 40
#define MAX_H 1024
#define MAX_W 1024
//#define pixToUs 250
#define pixToUs (min(250,(int)(2000000/(DQ.size()+2))))
#define usToFrame 50000
#define usToFirstKey 10000
#define usToKey 800000
#define pixR 6
#define pianoSize 32
#define melodySize 96
#define randomD 50


int pixL=pixR*pixR/4;
string lo,la,z,sc;
char str[100];
int width,height;
int b[MAX_H][MAX_W],c[MAX_H][MAX_W],d[MAX_H][MAX_W];
int mv[4][2]={-1,0, 1,0, 0,-1, 0,1};
int ang[4][2],angl;
int river_x=-1,river_y=-1;
int river_xs=-1,river_ys=-1;
char keyA[MAX_H][MAX_W],keyB[MAX_H][MAX_W];
int keyC[MAX_H][MAX_W][2];
IplImage* img=NULL;

struct PIX
{
    int x,y,us,key,keyTime;
}pix[MAX_H*MAX_W];

int pl;

struct KEYPIX
{
    int x,y,t,pi;
    CvScalar clr;
}kp[MAX_H*MAX_W];

int kpi,kpl;

bool chk(int x,int y)
{
    return x>-1&&x<height&&y>-1&&y<width&&b[x][y]&&!c[x][y];
}

//12356530356323102224354031231760
//13265430354321701712364034532310
//34576420312321706712364032312710
//33455430334523203657643036532310
//24354650567564203213217072176560

int kV[5][pianoSize]=
{
    21,22,23,25,26,25,23,0,23,25,26,23,22,23,21,0,22,22,22,24,23,25,24,0,23,21,22,23,21,27,26,0,
    21,23,22,26,25,24,23,0,23,25,24,23,22,21,27,0,21,27,21,22,23,26,24,0,23,24,25,23,22,23,21,0,
    23,24,25,27,26,24,22,0,23,21,22,23,22,21,27,0,26,27,21,22,23,26,24,0,23,22,23,21,22,27,21,0,
    23,23,24,25,25,24,23,0,23,23,24,25,22,23,22,0,23,26,25,27,26,24,23,0,23,26,25,23,22,23,21,0,
    22,24,23,25,24,26,25,0,25,26,27,25,26,24,22,0,23,22,21,23,22,21,27,0,27,22,21,27,26,25,26,0
};

int keyValue[melodySize];
int kvl=1;

void getKey(int &key,int &keyTime)
{
    key=keyValue[kvl];
    kvl+=3;
    if(kvl>=melodySize)
        kvl=1;
    keyTime=usToKey;
}
void getClr(int key)
{
    kp[kpl].clr.val[0]=192+rand()%64;
    kp[kpl].clr.val[1]=192+rand()%64;
    kp[kpl].clr.val[2]=192+rand()%64;
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
char buf[88][bufSize];
int bufl[88];

OSStatus MyEnqueueBuffer(MyData* myData);
void WaitForFreeBuffer(MyData* myData);

int hs(const char s[])
{
    int i,j;
    for(i=j=0;s[i];i++)
        j=(j*131+s[i])%10007;
    return j;
}

int main(int argc, const char * argv[])
{
    if(argc==4)
    {
        lo=argv[1];
        la=argv[2];
        z=argv[3];
    }
    else
    {
        lo="22.6983427";
        la="113.7134584";
        z="11";
    }
    int i,j,k,l,x,y;
    srand(hs(lo.c_str())*hs(la.c_str())*hs(z.c_str()));
    CvScalar cSW;
    cSW.val[0]=cSW.val[1]=cSW.val[2]=255;
    CvScalar cSB;
    cSB.val[0]=cSB.val[1]=cSB.val[2]=0;
    CvScalar cSG;
    cSG.val[0]=cSG.val[1]=cSG.val[2]=240;
    CvScalar cSL;
    cSL.val[0]=253; cSL.val[1]=242; cSL.val[2]=213;
    CvScalar cSD;
    cSD.val[0]=217; cSD.val[1]=163; cSD.val[2]=99;
    for(i=0;i<88;i++)
    {
        char s[10];
        sprintf(s, "%d.mp3", i);
        FILE* file=fopen(s, "r");
        if(!file)
            file=fopen("0.mp3","r");
        bufl[i]=fread(buf[i], 1, bufSize, file);
    }
    for(i=j=0,k=rand()%5;i<melodySize;i++)
    {
        if(i%6==0)
        {
            keyValue[i]=1;
        }
        else if(i%6==3)
        {
            keyValue[i]=2;
        }
        else if(i%3==1)
        {
            keyValue[i]=kV[k][j++];
        }
        else if(i%3==2)
        {
            keyValue[i]=3;
        }
    }
    sc="curl -o catch.png \"http://maps.googleapis.com/maps/api/staticmap?center="+lo+","+la+"&zoom="+z+"&size=640x640&maptype=roadmap&sensor=false\"";
    system(sc.c_str());
    while(!img)
        img=cvLoadImage( "catch.png" );
    
    width=img->width;
    height=img->height;
    ang[0][0]=0; ang[0][1]=0; ang[1][0]=height; ang[1][1]=width;
    ang[2][0]=height; ang[2][1]=0; ang[3][0]=0; ang[4][0]=width;
    
    for(i=pixR;i<height-pixR-1;i+=2*pixR)
    {
        for(j=pixR;j<width-pixR-1;j+=2*pixR)
        {
            keyA[i][j]=1;
            for(k=i-pixR;k<=i+pixR;k++)
            {
                for(l=j-pixR;l<=j+pixR;l++)
                {
                    if(abs(sqrt((k-i)*(k-i)+(l-j)*(l-j)))<pixR)
                    {
                        keyC[k][l][0]=i;
                        keyC[k][l][1]=j;
                    }
                }
            }
        }
    }
    
    cvShowImage( "River-Melody", img );
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
    
    cvShowImage( "River-Melody", img );
    cvWaitKey();
    cvDilate(img, img);
    cvDilate(img, img);
    cvDilate(img, img);
    cvErode(img, img);
    cvErode(img, img);
    cvErode(img, img);
    cvErode(img, img);
    cvErode(img, img);
    cvDilate(img, img);
    cvDilate(img, img);
    cvShowImage("River-Melody", img);
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
        puts("No river found.");
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
    
    printf("River starts at (%d,%d).\n",river_xs,river_ys);
    cvShowImage("River-Melody", img);
    cvWaitKey();
    
    int nextFrame=usToFrame;
    int nextKey=usToFirstKey;
    int us=0;
    pl=0;
    memset(c,0,sizeof c);
    c[river_xs][river_ys]=1;
    deque<int>DQ;
    DQ.push_back(river_xs);
    DQ.push_back(river_ys);
    while(!DQ.empty())
    {
        i=DQ.front(); DQ.pop_front();
        j=DQ.front(); DQ.pop_front();
        pix[pl].x=i;
        pix[pl].y=j;
        pix[pl].us=pixToUs;
        us+=pix[pl].us;
        if(keyC[i][j][0])
        {
            keyB[keyC[i][j][0]][keyC[i][j][1]]++;
            if(us>=nextKey&&keyB[keyC[i][j][0]][keyC[i][j][1]]>=pixL)
            {
                keyB[keyC[i][j][0]][keyC[i][j][1]]=1;
                getKey(pix[pl].key,pix[pl].keyTime);
                nextKey+=pix[pl].keyTime;
            }
        }
        pl++;
        for(l=0;l<15;l++)
        {
            if(l<4)
            {
                x=i+mv[l][0];
                y=j+mv[l][1];
            }
            else
            {
                x=i+rand()%randomD-randomD/2;
                y=j+rand()%randomD-randomD/2;
            }
            if(chk(x,y))
            {
                c[x][y]=1;
                if(rand()%100)
                {
                    DQ.push_back(x);
                    DQ.push_back(y);
                }
                else
                {
                    DQ.push_front(y);
                    DQ.push_front(x);
                }
            }
        }
    }
    
    MyData* myData;
    
    memset(c,0,sizeof c);
    memset(d,-1,sizeof d);
    kpi=kpl=0;
    
    for(i=0;i<height;i++)
    {
        for(j=0;j<width;j++)
        {
            if(!keyC[i][j][0])
            {
                d[i][j]=0x7fffffff;
                cvSet2D(img, i, j, cSW);
            }
        }
    }
    
    cvShowImage("River-Melody", img);
    cvWaitKey();
    us=0;
    nextFrame=usToFrame;
    nextKey=usToFirstKey;
    for(i=0;i<pl;i++)
    {
        x=pix[i].x;
        y=pix[i].y;
        us+=pix[i].us;
        if(d[x][y]!=-1&&!pix[i].key)
            continue;
        if(!pix[i].key)
        {
            cvSet2D(img, x, y, cSD);
        }
        else
        {
            kp[kpl].x=keyC[x][y][0];
            kp[kpl].y=keyC[x][y][1];
            kp[kpl].pi=i;
            kp[kpl].t=pix[i].keyTime/usToFrame;
            getClr(pix[i].key);
            kpl++;
        }
        if(us>=nextKey&&kpi<kpl)
        {
            myData = (MyData*)calloc(1, sizeof(MyData));
            AudioFileStreamOpen(myData, MyPropertyListenerProc, MyPacketsProc,kAudioFileAAC_ADTSType,
                                &myData->audioFileStream);
            AudioFileStreamParseBytes(myData->audioFileStream, bufl[pix[kp[kpi].pi].key],
                                      buf[pix[i].key], 0);
            MyEnqueueBuffer(myData);
            nextKey+=pix[kp[kpi].pi].keyTime;
            kpi++;
        }
        if(us>=nextFrame)
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
                    kp[j].clr=cSD;
                }
                else
                {
                    kp[j].clr.val[0]+=(cSD.val[0]-kp[j].clr.val[0])/6;
                    kp[j].clr.val[1]+=(cSD.val[1]-kp[j].clr.val[1])/6;
                    kp[j].clr.val[2]+=(cSD.val[2]-kp[j].clr.val[2])/6;
                }
            }
            cvShowImage("River-Melody", img);
            cvWaitKey(usToFrame/1000-teps);
            nextFrame+=usToFrame;
        }
    }
    
    for(i=0;i<height;i++)
    {
        for(j=0;j<width;j++)
        {
            if(b[i][j]&&d[i][j]<0x7fffffff)
                cvSet2D(img, i, j, cSD);
        }
    }
    
    cvShowImage("River-Melody", img);
    cvWaitKey();
    return 0;
}

void MyPropertyListenerProc(	void *							inClientData,
                            AudioFileStreamID				inAudioFileStream,
                            AudioFileStreamPropertyID		inPropertyID,
                            UInt32 *						ioFlags)
{
	MyData* myData = (MyData*)inClientData;
	switch (inPropertyID) {
		case kAudioFileStreamProperty_ReadyToProducePackets :
		{
			AudioStreamBasicDescription asbd;
			UInt32 asbdSize = sizeof(asbd);
			AudioFileStreamGetProperty(inAudioFileStream, kAudioFileStreamProperty_DataFormat, &asbdSize, &asbd);
			AudioQueueNewOutput(&asbd, MyAudioQueueOutputCallback, myData, NULL, NULL, 0, &myData->audioQueue);
			for (unsigned int i = 0; i < kNumAQBufs; ++i) {
				AudioQueueAllocateBuffer(myData->audioQueue, kAQBufSize,
                                         &myData->audioQueueBuffer[i]);
			}
			return;
		}
	}
}

void MyPacketsProc(				void *							inClientData,
                   UInt32							inNumberBytes,
                   UInt32							inNumberPackets,
                   const void *					inInputData,
                   AudioStreamPacketDescription	*inPacketDescriptions)
{
	MyData* myData = (MyData*)inClientData;
	for (int i = 0; i < inNumberPackets; ++i) {
		SInt64 packetOffset = inPacketDescriptions[i].mStartOffset;
		SInt64 packetSize   = inPacketDescriptions[i].mDataByteSize;
		size_t bufSpaceRemaining = kAQBufSize - myData->bytesFilled;
		if (bufSpaceRemaining < packetSize) {
			MyEnqueueBuffer(myData);
			WaitForFreeBuffer(myData);
		}
		AudioQueueBufferRef fillBuf = myData->audioQueueBuffer[myData->fillBufferIndex];
		memcpy((char*)fillBuf->mAudioData + myData->bytesFilled, (const char*)inInputData + packetOffset, packetSize);
		myData->packetDescs[myData->packetsFilled] = inPacketDescriptions[i];
		myData->packetDescs[myData->packetsFilled].mStartOffset = myData->bytesFilled;
		myData->bytesFilled += packetSize;
		myData->packetsFilled += 1;
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
	if (!myData->started) {
		AudioQueueStart(myData->audioQueue, NULL);
		myData->started = true;
	}
	return err;
}

OSStatus MyEnqueueBuffer(MyData* myData)
{
	OSStatus err = noErr;
	myData->inuse[myData->fillBufferIndex] = true;
	AudioQueueBufferRef fillBuf = myData->audioQueueBuffer[myData->fillBufferIndex];
	fillBuf->mAudioDataByteSize = myData->bytesFilled;
	AudioQueueEnqueueBuffer(myData->audioQueue, fillBuf, myData->packetsFilled,
                            myData->packetDescs);
	StartQueueIfNeeded(myData);
	
	return err;
}


void WaitForFreeBuffer(MyData* myData)
{
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
	MyData* myData = (MyData*)inClientData;
	unsigned int bufIndex = MyFindQueueBuffer(myData, inBuffer);
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
	AudioQueueGetProperty(inAQ, kAudioQueueProperty_IsRunning, &running, &size);
	if (!running) {
		pthread_mutex_lock(&myData->mutex);
		pthread_cond_signal(&myData->done);
		pthread_mutex_unlock(&myData->mutex);
	}
}
