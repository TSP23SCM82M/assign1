#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<string.h>

#include "storage_mgr.h"

FILE *pageFile;

extern void initStorageManager (void) {
}

extern RC createPageFile (char *fileName) {
}

extern RC openPageFile (char *fileName, SM_FileHandle *fHandle) {
}

extern RC closePageFile (SM_FileHandle *fHandle) {
}


extern RC destroyPageFile (char *fileName) {
}

extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
}

extern int getBlockPos (SM_FileHandle *fHandle) {
}

extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
}

extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
}

extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {

}

extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
}

extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
}

extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
}

extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
}

extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle) {
}
