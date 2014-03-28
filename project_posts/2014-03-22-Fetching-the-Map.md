First of all, we need the river images from google map. According to https://developers.google.com/maps/documentation/staticmaps/, the simplest way to download a specific map image is by using http request. 

For example, if we need a map image that have a center at (22.6983427,113.7134584) with zoom level 11, we could simply request for "http://maps.googleapis.com/maps/api/staticmap?center=22.6983427,113.7134584&zoom=11&size=640x640&maptype=roadmap&sensor=false"(which is the Pearl River). Thus we could use curl to download the map image.

## Example Code
```
string lo,la,z,sc;
if(argc==4)
{
    lo=argv[1];
    la=argv[2];
    z=argv[3];
}
sc="curl -o catch.png \"http://maps.googleapis.com/maps/api/staticmap?center="+lo+","+la+"&zoom="+z+"&size=640x640&maptype=roadmap&sensor=false\"";
system(sc.c_str());
while(!img)
    img=cvLoadImage( "catch.png" );
```
