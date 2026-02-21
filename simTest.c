#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TESTING
#include "Converter.c"   // change to your simulator filename

static void makeBin(const char *path,const void *bytes,size_t n){
    FILE *fp=fopen(path,"wb");
    assert(fp);
    fwrite(bytes,1,n,fp);
    fclose(fp);
}

int main(void){
    //badMem
    assert(badMem(0x0FFF,4)==true);
    assert(badMem(0x1000,4)==false);
    assert(badMem(0x1001,4)==true);
    assert(badMem(0x1004,4)==false);
    assert(badMem(512*1024-4,4)==false);
    assert(badMem(512*1024-3,4)==true);

    //readStrict: feed stdin
    {
        FILE *f=fopen("tmp_stdin.txt","wb");
        assert(f);
        fputs("0\n18446744073709551615\n00012\n",f);
        fclose(f);

        assert(freopen("tmp_stdin.txt","rb",stdin));
        uint64_t x=999;

        assert(readStrict(&x)==1&&x==0ULL);
        assert(readStrict(&x)==1&&x==18446744073709551615ULL);
        assert(readStrict(&x)==1&&x==12ULL);
    }
    {
        FILE *f=fopen("tmp_stdin.txt","wb");
        assert(f);
        fputs("-1\n+1\n1.0\n123abc\n18446744073709551616\n\n",f);
        fclose(f);

        assert(freopen("tmp_stdin.txt","rb",stdin));
        uint64_t x=0;

        assert(readStrict(&x)==0);
        assert(readStrict(&x)==0);
        assert(readStrict(&x)==0);
        assert(readStrict(&x)==0);
        assert(readStrict(&x)==0);

        while(readStrict(&x)==1){}
        assert(readStrict(&x)==0);
    }

    //execute: sanity check a couple ops (same vibe as your old tests)
    {
        uint64_t pc=0x1000;
        memset(registers,0,sizeof(registers));
        registers[1]=6;
        registers[2]=3;
        //and op=0 rd=3 rs=1 rt=2
        assert(execute((0u<<27)|(3u<<22)|(1u<<17)|(2u<<12),&pc)==0);
        assert(registers[3]==2);
    }
    {
        uint64_t pc=0x1000;
        memset(registers,0,sizeof(registers));
        registers[1]=2;
        registers[2]=5;
        //add op=24 rd=3 rs=1 rt=2
        assert(execute((24u<<27)|(3u<<22)|(1u<<17)|(2u<<12),&pc)==0);
        assert(registers[3]==7);
    }

    //header validation integration: make an obviously-invalid bin (bad codeStart)
    //Your main should FAIL (nonzero).
    {
        uint64_t hdr[5];
        hdr[0]=0;        //fileType
        hdr[1]=0x1004;   //codeStart (not 0x1000-aligned per your badMem)
        hdr[2]=4;        //codeSize
        hdr[3]=0x10000;  //dataStart
        hdr[4]=0;        //dataSize
        makeBin("bad.bin",hdr,sizeof(hdr));

        int rc=system("./hw5-sim bad.bin > sim_out.txt 2> sim_err.txt");
        assert(rc!=0);
    }

    remove("tmp_stdin.txt");
    remove("bad.bin");
    remove("sim_out.txt");
    remove("sim_err.txt");

    puts("Passed sim tests");
    return 0;
}