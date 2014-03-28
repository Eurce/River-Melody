As a river contains lots of pixels, and the process of flowing is basically a process of sequential pixel traversal, thus it's rather important to determine the traversal order of each pixel.

In our project, the traverse order is determined by a search procedure, more specifically, a modified BFS. In order to best simulate the river flow, random traversal is applied. The traversal is divided into three parts:

1. BFS to all 4-connected adjacent pixelx.

2. BFS to 12 pixels that have a distance randomly from 1 to 50.

3. As the search procedure is implemented with deque, which allows low probalbility front push behaviour. That is to say, sometimes DFS is executed temporarily.

The above three points are implemented as follows:

```
#define randomD 50

bool chk(int x,int y)
{
    return x>-1&&x<height&&y>-1&&y<width&&b[x][y]&&!c[x][y];
}

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
```
