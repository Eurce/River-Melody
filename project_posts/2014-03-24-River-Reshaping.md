After downloading the map image, we need to find out whether a river actually exists or not. But before conducting the search, raw map images need to be processed. For example in the image of the Pearl River, we could see bridges and small tributaries that distort the shape of river:

![Example Image](project_images/sample_1.png?raw=true "sample_1")

The OpenCV have provided several morphology process functions that could help us reshape the river. In our project, we firstly mark up all river pixels in white, and then conduct dilation and erosion several times, just as the follow code displays:

```
CvScalar cSW;
cSW.val[0]=cSW.val[1]=cSW.val[2]=255;
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
```

Afterwards, the river looks like this:

![Example Image](project_images/sample_3.png?raw=true "sample_3")
