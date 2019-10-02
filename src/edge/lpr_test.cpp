#include <iostream>
#include <fstream>
#include <string>
#include <time.h>

#include <libyuv.h>
#include "jpg_codec.h"

using namespace std;

int main(int argc, char ** argv)
{
    if(argc < 2)
    {
        cout << argv[0] << " filename" << endl;
        return 0;
    }
    
        cout<<"LPR_ALG init OK"<<endl;
        std::ifstream file(argv[1], std::ios::in | std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            fprintf(stderr, "open file fail: %s\n", argv[1]);
            return -1;
        }
        
        std::streampos size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::string buffer(size, ' ');
        file.read(&buffer[0], size);
        file.close();
    
    char * argb_data=(char *)malloc(4096*2160*4);
    EyeJpegCodec ejc;
    int c=4;
    int w=0, h=0;
    bool ret = ejc.JpegUnCompress((const char *)buffer.c_str(), size, (char *)argb_data,
                                  4096*2160*3, w, h, c);
    if(ret)
    {
        cout << "w:\t" << w << "\th:\t" << h << endl;
    }
    else
        cout << "jpeg decode error!" << endl;
}

/*
int main(int argc, char ** argv)
{
    if(argc < 2)
    {
        cout << argv[0] << " filename" << endl;
        return 0;
    }
    if(vlpr_init())
    {
        cout<<"LPR_ALG init OK"<<endl;
        std::ifstream file(argv[1], std::ios::in | std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            fprintf(stderr, "open file fail: %s\n", argv[1]);
            return -1;
        }
        
        std::streampos size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::string buffer(size, ' ');
        file.read(&buffer[0], size);
        file.close();
        VPR vpr;
        for(int i=0;i<100;i++)
        {
            clock_t t=clock();
            if(vlpr_analyze((const unsigned char *)buffer.c_str(), (int)size, &vpr))
            {
                cout << "Time\t" << (clock()-t)/1000 << " ms" << endl;
                cout << "OK\t" << vpr.license << "\t" << vpr.color << "\t" << vpr.nColor << "\t" << vpr.nConfidence << endl;
                cout << "Coor\t"  << vpr.left << "\t" << vpr.top << "\t" << vpr.right << "\t" << vpr.bottom << endl;
            }else{
                cout << "Time\t" << (clock()-t)/1000 << " ms" << endl;
                cout << "Fail" << endl;
            }
        }
        
    }else{
        cout<<"LPR_ALG init Fail, Exit"<<endl;
    }
    
    return 0;
}
*/
