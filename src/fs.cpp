#include "fs.h"
#include "debug.h"
#include <list>
#include <string>
#include <cstdio>
#include <zzip/zzip.h>
#include "SDL_rwops_zzip.h"

using std::list;
using std::string;

static bool fs_Active = false;
static list<string> dirList;
static list<ZZIP_DIR*> zipList;
#ifdef linux
static char dirSepator='/';
#endif

#ifdef _WIN32
static char dirSepator='\\';
#endif

void fs_startUp() {
    if (fs_Active)
        return;
    fs_cleanEnvironment();
    fs_Active = true;
    DTRACETIME("clean");
}

void fs_cleanUp() {
    if (!fs_Active)
        return;
    fs_Active = false;
    DTRACETIME("clean");
}

void fs_cleanEnvironment(){
    dirList.clear();
    list<ZZIP_DIR*>::iterator zipIter;
    for(zipIter=zipList.begin(); zipIter!=zipList.end(); zipIter++){
        zzip_dir_close(*zipIter);
    }
    zipList.clear();
    fs_registerZip("data.zip");
}

void fs_registerDir(const char* dir){
    dirList.push_back(string(dir));
}

bool fs_registerZip(const char* file){
    ZZIP_DIR* dir = zzip_dir_open(file,0);
    if (dir) {
        zipList.push_back(dir);
    }
    else{
        ETRACE("Failed to register file %s", file);
        return true;
    }
    return false;
}

char *fs_loadAsString(const char* fileName){
    list<string>::iterator dirIter;
    for(dirIter=dirList.begin(); dirIter!=dirList.end(); dirIter++){
        string path=*dirIter+dirSepator+string(fileName);
        FILE *file = fopen(path.c_str(), "rb");
        if(file==NULL) continue;
        fseek(file, 0, SEEK_END);
        int size=ftell(file);
        fseek(file, 0, SEEK_SET);
        char *data=new char[size+1];
        int read=fread(data, 1, size, file);
        if(read!=size) return NULL;
        fclose(file);
        data[size]=0;
        return data;
    }
    list<ZZIP_DIR*>::iterator zipIter;
    for(zipIter=zipList.begin(); zipIter!=zipList.end(); zipIter++){
        ZZIP_FILE* fp = zzip_file_open(*zipIter,fileName,0);
        if(fp==NULL) continue;
        zzip_seek(fp, 0, SEEK_END);
        int size=zzip_tell(fp);
        zzip_seek(fp, 0, SEEK_SET);
        char *data=new char[size+1];
        int read=zzip_file_read(fp, data, size);
        if(read!=size) return NULL;
        zzip_file_close(fp);
        data[size]=0;
        return data;
    }
    return NULL;
}

SDL_RWops *fs_loadAsSDL_RWops(const char* fileName){
    list<string>::iterator dirIter;
    for(dirIter=dirList.begin(); dirIter!=dirList.end(); dirIter++){
        string path=*dirIter+dirSepator+string(fileName);
        FILE *file = fopen(path.c_str(), "rb");
        if(file==NULL) continue;
        fclose(file);
        /*
        can not use SDL_RWFromFP
        cite http://sdl.beuc.net/sdl.wiki/SDL_RWFromFP:
        Note: This is not available under Win32, since files opened in an application on that platform cannot be used by a dynamically linked library.
        */
        return SDL_RWFromFile(path.c_str(), "rb");
    }
    list<ZZIP_DIR*>::iterator zipIter;
    for(zipIter=zipList.begin(); zipIter!=zipList.end(); zipIter++){
        ZZIP_FILE* fp = zzip_file_open(*zipIter,fileName,0);
        if(fp==NULL) continue;
        return SDL_RWFromZZIP(fp);
    }
    return NULL;
}



