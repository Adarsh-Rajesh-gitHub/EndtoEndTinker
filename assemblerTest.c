// test_firstParse.c
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "tinker_h"

//helper for label flush like main before processing a code/data line
static void flush_labels(List *curLabels,hashMap *hM,List *labels_seen,uint64_t pcval){
    for(int i=0;i<curLabels->numElements;i++){
        assert(label_add(curLabels->entries[i],hM,labels_seen,pcval)==1);
    }
    clearList(curLabels);
}

//tested just helper methods and made invalid and valid test files for input and ran in terminal

int main(void){
    uint64_t v=0;

    // strictParse
    assert(parse_u64_strict("0",&v) && v==0);
    assert(parse_u64_strict("18446744073709551615",&v) && v==UINT64_MAX);
    assert(!parse_u64_strict("-1",&v));
    assert(!parse_u64_strict("12x",&v));


    // commaSpace
    assert(commaSpace("add r1, r2, r3"));
    assert(!commaSpace("add r1,r2, r3"));
    assert(!commaSpace("add r1,  r2, r3"));
    assert(!commaSpace("add r1 , r2, r3"));
    assert(!commaSpace("add r1,\tr2, r3"));

    // validLabel
    assert(validLabel("L1"));
    assert(validLabel("_x9"));
    assert(!validLabel("1bad"));
    assert(!validLabel("a-b"));

    // op_is
    const char *after=0;
    assert(op_is("add r1, r2, r3","add",&after) && strcmp(after,"r1, r2, r3")==0);
    assert(op_is("add, r1, r2, r3","add",&after) && strcmp(after,"r1, r2, r3")==0);
    assert(!op_is("addiX r1, 5","addi",&after));


    hashMap *hM=createHashMap();
    List *labels_seen=createList();
    List *curLabels=createList();

    uint64_t pc_test=0x2000;
    uint64_t pcData_test=0x10000;

     //flush a label in code mode at current pc
    add(curLabels,strdup(":L0"));
    flush_labels(curLabels,hM,labels_seen,pc_test);
    assert((uint64_t)find(hM,"L0")==0x2000);
    pc_test+=4;
    //simld macro size rthen  next label should land after it
    add(curLabels,strdup(":L1"));
    flush_labels(curLabels,hM,labels_seen,pc_test);
    assert((uint64_t)find(hM,"L1")==0x2004);
    pc_test+=48;

    add(curLabels,strdup(":L2"));
    flush_labels(curLabels,hM,labels_seen,pc_test);
    assert((uint64_t)find(hM,"L2")==0x2034);
    pc_test+=8;

    add(curLabels,strdup(":D0"));
    flush_labels(curLabels,hM,labels_seen,pcData_test);
    assert((uint64_t)find(hM,"D0")==0x10000);
    pcData_test+=8;

    add(curLabels,strdup(":D1"));
    flush_labels(curLabels,hM,labels_seen,pcData_test);
    assert((uint64_t)find(hM,"D1")==0x10008);
    pcData_test+=8;

    //label at end of data should point to curr pcData
    add(curLabels,strdup(":DEND"));
    flush_labels(curLabels,hM,labels_seen,pcData_test);
    assert((uint64_t)find(hM,"DEND")==0x10010);

    //dupe label
    assert(label_add(":L0",hM,labels_seen,0x9999)==0);

    puts("passed");
    return 0;
}