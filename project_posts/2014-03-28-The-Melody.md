Eventually, we are determining the melody. 

In this project, we simply use a hash function that takes the river coordinate as input, and outputs the random seed. Then each of the musical notes are defined by the rand() function as a piano key ranging from C1 to C6.

```
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
    else //the default coordinate belongs to the pearl river
    {
        lo="22.6983427";
        la="113.7134584";
        z="11";
    }
    srand(hs(lo.c_str())*hs(la.c_str())*hs(z.c_str()));
    
    ...
```
