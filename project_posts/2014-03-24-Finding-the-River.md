Before searching for the river, let's make some hypotheses:

1. There is one single downstream side and one single upstream side.

2. The downstream side has a large chunk of river pixels.

3. The upstream side locates at the diagonal position of the downstream side.

4. The pixels belonge to the same river are all 4-connected.

Based on the above hypotheses, we could build up the process for river searching.

```
#define river_size 40

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

int dia_rx=height-river_x-river_size;
int dia_ry=width-river_y-river_size;

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
            if(abs(x-dia_rx)+abs(y-dia_ry)<k)
            {
                k=abs(x-dia_rx)+abs(y-dia_ry);
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
```
