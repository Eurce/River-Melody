While simulating the river flow, it's necessary to control the flow speed. The requirement for this could be concluded into three points:

1. The number of pixels being flowed over within a certain time gap should be matching the width of the river. Otherwise the flow will slow down to an unacceptable level when the river flows into sea.

2. The FPS should be maintained at a certain level. Between two adjacent frames, the color changes (if any) of each pixel should be finished in time.

3. The time gap between two adjacent musical notes should be maintained at a certain level as well. The corresponding river region needs to be chosen well to meet the time requirement.

Here's our solution:

1. Binding the time with the number of flowed over pixels. More specifically, we allocated a time value to each of the river pixels according to the current river width. The river width is measured by the size of deque (which is used for the pixel traversal).

2. Then we could set up the time gap between two adjacent frames and notes separately. As soon as enough pixels have been flowed over, the next frame or note will come up.

3. For precise time controling, we use microsecond.

The following code shows the idea:

```
#define pixToUs (min(400,(int)(2450000/(DQ.size()+2))))
#define usToFrame 50000
#define usToFirstKey 10000
#define usToKey 200000

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
    if(keyC[i][j][0]&&!keyA[keyC[i][j][0]][keyC[i][j][1]])
    {
        keyB[keyC[i][j][0]][keyC[i][j][1]]++;
        if(us>=nextKey&&keyB[keyC[i][j][0]][keyC[i][j][1]]>=pixMN
           &&keyB[keyC[i][j][0]][keyC[i][j][1]]<=pixMX)
        {
            keyA[keyC[i][j][0]][keyC[i][j][1]]=1;
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
```
