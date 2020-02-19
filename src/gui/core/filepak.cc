#include "filepak.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <random>
#include <time.h>
#include <dirent.h>
#include <gzstream.h>
#include <memory>
#include <stddef.h>

namespace nglui{
#define ENTRY_SIZE offsetof(PAKFileEntry,fullname)
FilePAK::FilePAK(void){
    pakloaded = false;
}


FilePAK::~FilePAK(void){
    entries.clear();
}

int FilePAK::readDirectory(const std::string&entryPath,const vector<std::string>&correctTypes,const std::string&basedir){
    DIR*dir;
    dirent*entry;
    int rc=0;
    if((dir=opendir(entryPath.c_str()))==nullptr)
        return 0;
    while(entry=readdir(dir)){
        if(entry->d_type==DT_DIR && entry->d_name[0]!='.'){
            std::string path=entryPath+"/";
            path+=entry->d_name;
            rc+=readDirectory(path,correctTypes,basedir);
        }else if(entry->d_type==DT_REG){
            bool correctType = correctTypes.size()==0;
            for(unsigned int i = 0; i < correctTypes.size(); i++){
                std::string comparestr = entry->d_name;
                int found = comparestr.find_last_of('.');
                comparestr = comparestr.substr(found);
                if(!comparestr.compare(correctTypes[i]))
                     correctType = true;
            }
            if(correctType){
                if(!createEntry(entryPath, entry->d_name)){
                    printf("%s/%s package error\r\n",entryPath.c_str(),entry->d_name);
                    continue;
                }rc++;
            }
        }
    }
    closedir(dir);
    return rc;
}

bool FilePAK::createPAK(const std::string& name, const std::string& entryPath, const std::string& types)
{
    pakloaded = false;
    pakname = name;
    srand((unsigned) time(NULL)); //seedin'
    ogzstream outpak;
    ifstream fileIn;
    std::string path=entryPath;
    if(path[path.length()-1]!='/')
       path.append("/");
    memcpy(header.fileID, "DBPAK\0", 6); //Using memcpy because lol char array
    memcpy(header.version, "1.0\0", 4);

    vector<std::string> correctTypes = filetypes(types);
    header.numberFiles=readDirectory(path,correctTypes,path);

    int offset = sizeof(PAKHeader) + (header.numberFiles * ENTRY_SIZE);
    //there's always 1 header, and there's a PAKfileEntry for every file so find the sum of
    for(unsigned int i = 0; i < entries.size(); i++){//their sizes to find the offset for the first file
        entries[i].offset = offset; //calculate all the offsets for each file
        offset += entries[i].size;
        printf("%d:%d %s\r\n",entries[i].size,entries[i].offset,entries[i].name); 
    }
    printf("package dir %s (%d resource) to %s\r\n",path.c_str(),header.numberFiles,name.c_str());
    if(header.numberFiles){ //if any files were found at all
         outpak.open(name.c_str());//ofstream::binary | ofstream::trunc
         outpak.write((char *) &header, sizeof(header)); //write the header
 	 char *buffer=new char[512];
 	 for(unsigned int i = 0; i < entries.size(); i++){
	     memcpy(buffer, &(entries[i]), ENTRY_SIZE); //copy over the current entry in the for loop
             outpak.write(buffer, ENTRY_SIZE); //finally write the entry
	 }
	 for(unsigned int i = 0; i < entries.size(); i++){
	     int size = entries[i].size;

	     fileIn.open(entries[i].fullname, ifstream::binary);
	     if(!fileIn.is_open()) return false;
             while(size>0){
	         int sz=fileIn.readsome(buffer,512); //read in the file so it can be encrypted then written into the PAK file
	         outpak.write(buffer,sz); //write it out
                 size-=sz;
             }
             fileIn.close();
         }
         delete []buffer;
         outpak.close();
    }
    if(header.numberFiles < 1) return false; //no files found :(

    return true;
}

bool FilePAK::createEntry(const std::string& path, const std::string& name)
{
    ifstream fileIn;
    PAKFileEntry fentry; //creates a new table of contents entry

    string entryName=path; //Sets up the path/name strings
    if(path[path.length()-1]!='/')
       entryName+="/";
    entryName += name;
    size_t pos=entryName.rfind("/");
    pos=entryName.rfind("/",pos-1);
    strncpy(fentry.name, entryName.c_str()+pos+1, 50); //only the file name
    memcpy(fentry.fullname, entryName.c_str(), 150); //file name + folders

    fileIn.open(entryName, ifstream::binary | ifstream::ate);

    if(fileIn.is_open()){
	fentry.size = (unsigned int) fileIn.tellg(); //to calculate the file's size
    }else{
	return false;
    }

    fileIn.close();
    fentry.offset = 0; //unknown right now
    entries.push_back(fentry); //append to the vector
    return true;
}

bool FilePAK::readPAK(const std::string& PAKpath)
{
    igzstream PAKread;
    pakname=PAKpath;
    PAKread.open(PAKpath.c_str());//,ios::binary);
    PAKread.read((char *) &header, sizeof(PAKHeader)); //read in the header information so you can decrypt
    if(strcmp(header.fileID, "DBPAK") != 0 || !(header.numberFiles > 0) || strcmp(header.version, "1.0") != 0) {
        //if the fileIDs or versions don't match or if there's 0 or less files
        PAKread.close(); 
        return false;
    }
    entries.clear(); //entries could be full from createPAK()
    char buffer[ENTRY_SIZE];

    for(int i = 0; i < header.numberFiles; i++){
        PAKFileEntry entry;
        memset(&entry,0,sizeof(entry));
        PAKread.read(buffer, ENTRY_SIZE);
        memcpy(&entry,buffer,ENTRY_SIZE); //store the decrypted stuff into the entry
        entries.push_back(entry); //append to the vector
    }
    PAKread.close();
    pakloaded = true;
    return true;
}

bool FilePAK::rebuildPAK()
{
    if(changes.empty()||(pakloaded==false)) return false; //if no changes are buffered
    ofstream PAKout;
    ifstream PAKin;

    PAKout.open(pakname+".new", ofstream::binary); //temporary new file to write to

    int numberFiles = 0;

    vector<PAKFileEntry> original(entries);

    for(unsigned int i = 0; i < changes.size(); i++)
       if(changes[i] >= 0) //count all changes that aren't deletions
	    numberFiles++;

       header.numberFiles = numberFiles;

       int offset = sizeof(PAKHeader) + (numberFiles * ENTRY_SIZE);
       for(unsigned int i = 0; i < entries.size(); i++){ //find out new offsets
   	   if(changes[i] == -1) continue; //don't factor in deletions
	   entries[i].offset = offset;
	   offset += entries[i].size;
       }

       if(PAKout.is_open()){
  	   PAKout.write((char *) &header, sizeof(header)); //write out header		
  	   for(unsigned int i = 0; i < entries.size(); i++){
	       if(changes[i] == -1){ //if this change is a deletion
	   	   continue;
	       }
	       PAKout.write((char*)&entries[i], ENTRY_SIZE); //finally write the entry
	   }
	   for(unsigned int i = 0; i < entries.size(); i++){
	       char *buffer;
	       if(changes[i] == -1) continue; //again, don't factor in deletions
	       if(changes[i] == 1){
	    	   PAKin.open(entries[i].fullname, ifstream::binary); //if it's an addition, load the file
	       }else{ //if it's already in the PAK file, load it from there
	           PAKin.open(pakname, ifstream::binary);
		   PAKin.seekg(original[i].offset);//get filecurrent postion as resource offset
	       }

	       if(PAKin.is_open()){
	           buffer = new char[entries[i].size];
	  	   PAKin.read(buffer, entries[i].size);
		   if(changes[i] == 1){
		   }
		   PAKout.write(buffer, entries[i].size);
	       }else{
	 	   original.clear();
		   return false;
	       }
	       PAKin.close();
	       delete [] buffer;
	   }

	   original.clear();
     }else{
  	  original.clear();
	  return false;
     }
     PAKout.close();
     remove(pakname.c_str()); //deleting old PAK

     char* filename = new char[150];
     strcpy(filename, pakname.c_str());
     strcat(filename, ".new");

     rename(filename, pakname.c_str());
     delete filename;

     for(unsigned int i = 0; i < entries.size(); i++){ //erase all deletions
         if(changes[i] == -1)
             entries.erase(entries.begin()+i, entries.begin()+i+1);
     }
     changes.clear();

     return true;
}

bool FilePAK::appendFile(const std::string& name)
{
    int found = name.find_last_of("/\\"); //seperating path from filename
    string path = name.substr(0, found+1);
    string file = name.substr(found+1);

    for(unsigned int i = 0; i < entries.size(); i++){ //if file name already exists
	if(!file.compare(entries[i].name))
	    return false;
    }
    if(!createEntry(path, file)) return false;

    if(changes.empty()) changes.assign(entries.size()-1, 0);
	changes.push_back(1);

    return true;
}

void* FilePAK::getPAKEntryData(const std::string& name){

    if( PAKFileEntry *entry = getPAKEntry( name ) ){
        igzstream PAKread;
        PAKread.open(pakname.c_str());//, ios::binary);
	if(PAKread.good()){//PAKread.is_open())
            void*buffer=malloc(entry->size);
            PAKread.seekg(entry->offset);//, ifstream::beg); //seek to the offset of the file in the .pak file
            PAKread.read((char*)buffer, entry->size); //read everything into the buffer
            return buffer;
	}else{
	    cout << "Critical error: getPAKEntryData() could not open stream\n";
	    return nullptr;
	}
    }
    return nullptr; //PAK file isn't loaded, or entry isn't found
}

void FilePAK::freeEntryData(void*buffer){
    free(buffer);
}
size_t FilePAK::getPAKEntrySize(const std::string& name){
    if(pakloaded){
        for(int i = 0; i < header.numberFiles; i++){
            if(strcmp(entries[i].name, name.c_str()) == 0){
                return entries[i].size;
            }
        }
       return -1; // This shouldn't happen. Treat as a critical error.
    }
    return -2; //PAK file isn't loaded
}

FilePAK::PAKFileEntry *FilePAK::getPAKEntry(const std::string& name){
    if(pakloaded){
        for(int i = 0; i < header.numberFiles; i++){
            if(name.compare(entries[i].name) == 0){
                return &entries[i];
	    }
	}
    }
    return NULL; //PAK file isn't loaded, or entry isn't found
}

vector<string> FilePAK::getAllPAKEntries(){
    vector<string> allentries;
    if(pakloaded){
        for(unsigned int i = 0; i < entries.size(); i++){
           allentries.push_back(entries[i].name);
	}
    }
    return allentries; //NULL if pakloaded == false
}

bool FilePAK::unPAKEntry(const std::string& name, const std::string& path){
    ofstream output;
    output.open(path, ofstream::binary | ofstream::trunc);
    if(output.is_open()){
        void*buffer=getPAKEntryData(name);
        int size =getPAKEntrySize(name);
        if(buffer == NULL || size <= 0) return false;
        output.write((const char*)buffer, size);
        freeEntryData(buffer);
    }else{
        return false;
    }
    output.close();
    return true;
}

vector<string> FilePAK::filetypes(const std::string& types){
    vector<std::string> splittypes;
    if(types.empty()) return splittypes;
    int numtypes = 0;
    size_t pos = -1;

    do{
       pos = types.find('|', pos+1);
       if(pos == string::npos) { numtypes++; break; }
       numtypes++;
    } while(true);

    string splittype;
    pos = -1;

    for(int i = 0; i < numtypes; i++){
        splittype = types.substr(pos+1, types.find('|', pos+1));
        pos = types.find('|', pos+1);
        splittypes.push_back(splittype);
    }
    return splittypes;
}

//Returns the number of entries in the pak file
int FilePAK::getNumPAKEntries()
{
	return header.numberFiles;
}

bool FilePAK::removeFile(const std::string& name){
    for(unsigned int i = 0; i < entries.size(); i++){
        if(name.compare(entries[i].name) == 0)	{
            if(changes.empty()) changes.assign(entries.size()-1, 0);
                changes[i] = -1;
		return true;
            }
    }
    return false;
}

}//namespace
#ifdef PACK_TOOL

using namespace nglui;
int main(int argc,const char*argv[]){
   FilePAK pak;
   if(argc>=3){
       cout<<argv[0]<<" "<<argv[1] << " "<<argv[2]<<endl;
       pak.createPAK(argv[1],argv[2]);
   }
   if(argc==2){
       pak.readPAK(argv[1]);
       std::vector<std::string>names=pak.getAllPAKEntries();
       for(auto nm:names){
           FilePAK::PAKFileEntry*r=pak.getPAKEntry(nm);
           cout<<"  size:"<<r->size<<"  offset:"<<r->offset<<" name:"<<r->name<<" fullname:"<<r->fullname<<std::endl;
           pak.getPAKEntryData(nm);
       }
   }
   return 0;
}
#endif
