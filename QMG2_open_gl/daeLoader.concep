#include <stdio.h>
uint8_t* read_UTF8_file_to_UTF32(const char* filename){
    FILE* fp;
    fp=fopen(filename,"rb"));
    if(!fp){
        printf("Error: Failed to open file %s!\n", filename);
        return NULL;
    }
    //Calculate the Maximum possible File size
    fseek(fp, 0, SEEK_END);
    long filesize=ftell(fp);
    if(filesize)
    filesize*=4; //The max length per byte of file is 4 byte long unicode sequence
    fseek(fp, 0, SEEK_SET);
    uint8_t* readBuffer=(uint8_t*)malloc(filesize);
    printf("Info: Read %d and expected %d.\n",fread(readBuffer,1,filesize,fp),filesize);
    for(long currentBufferIndex=0;currentBufferIndex<filesize;currentBufferIndex++){
        if(readBuffer[currentBufferIndex]>0x7F){
            switch(readBuffer[currentBufferIndex]){
                case 0x80:
                break;
            }
        }
    }

    fclose(fp);
    return readBuffer;
}
