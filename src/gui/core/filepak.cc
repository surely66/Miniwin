#include "filepak.h"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <random>
#include <time.h>
#include <dirent.h>
#include <gzstream.h>
#include <memory>

namespace nglui{

FilePAK::FilePAK(void){
    pakloaded = false;
}


FilePAK::~FilePAK(void){
    entries.clear();
}

bool FilePAK::createPAK(const std::string& name, const std::string& entryPath, const std::string& types)
{
    pakloaded = false;
    pakname = name;
    srand((unsigned) time(NULL)); //seedin'
    ogzstream PAKout;
    ifstream fileIn;
    int numberFiles = 0;

    memcpy(header.fileID, "DBPAK\0", 6); //Using memcpy because lol char array
    memcpy(header.version, "1.0\0", 4);

    vector<std::string> correctTypes = filetypes(types);
    DIR *dir; //dirent.h stuff to accumulate all files within a folder
    dirent *entry;
    if(dir = opendir(entryPath.c_str())){
        while(entry = readdir(dir)){
            if(entry->d_type != DT_DIR && entry->d_type == DT_REG){ //if it's not a folder and a regular file
	 	bool correctType = false;
		if(types.empty()) correctType = true;
	        else{
		   for(unsigned int i = 0; i < correctTypes.size(); i++){
	 	        std::string comparestr = entry->d_name;
		        int found = comparestr.find_last_of('.');
		        comparestr = comparestr.substr(found);
                        if(!comparestr.compare(correctTypes[i]))
                            correctType = true;
                    }
                }
	        if(correctType){
	            numberFiles++;
	            if(!createEntry(entryPath, entry->d_name)) return false;
	        }
	    }
        }
    }

    delete dir, entry;

    header.numberFiles = numberFiles;

    int offset = sizeof(PAKheader) + (numberFiles * sizeof(PAKfileEntry)); //there's always 1 header, and there's a PAKfileEntry for every file so find the sum of
    for(unsigned int i = 0; i < entries.size(); i++){						//their sizes to find the offset for the first file
        entries[i].offset = offset; //calculate all the offsets for each file
        offset += entries[i].size;
    }

    if(numberFiles){ //if any files were found at all
         PAKout.open(name.c_str());//ofstream::binary | ofstream::trunc
         PAKout.write((char *) &header, sizeof(header)); //write the header
 	 char *buffer;
 	 for(unsigned int i = 0; i < entries.size(); i++){
	     buffer = new char[sizeof(PAKfileEntry)]; //char array to hold each table of contents entry
	     memcpy(buffer, &(entries[i]), sizeof(PAKfileEntry)); //copy over the current entry in the for loop
             PAKout.write(buffer, sizeof(PAKfileEntry)); //finally write the entry
	     delete [] buffer; //no memory leaks in this code, no sir
	 }
	 for(unsigned int i = 0; i < entries.size(); i++){
	     int size = entries[i].size;
	     buffer = new char[size];

	     fileIn.open(entries[i].fullname, ifstream::binary);
	     if(!fileIn.is_open()) return false;
	     fileIn.read(buffer, size); //read in the file so it can be encrypted then written into the PAK file
	     PAKout.write(buffer, size); //write it out
             delete [] buffer;
             fileIn.close();
         }
         PAKout.close();
	 //else return false; //PAKout not open
    }
    if(numberFiles < 1) return false; //no files found :(

    return true;
}

bool FilePAK::createEntry(const std::string& path, const std::string& name)
{
    ifstream fileIn;
    PAKfileEntry fentry; //creates a new table of contents entry

    string entryName; //Sets up the path/name strings
    entryName += path;
    entryName += name;
    memcpy(fentry.name, name.c_str(), 50); //only the file name
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
    PAKread.read((char *) &header, sizeof(PAKheader)); //read in the header information so you can decrypt
    if(strcmp(header.fileID, "DBPAK") != 0 || !(header.numberFiles > 0) || strcmp(header.version, "1.0") != 0) {
        //if the fileIDs or versions don't match or if there's 0 or less files
        PAKread.close(); 
        return false;
    }
    entries.clear(); //entries could be full from createPAK()
    char *buffer;

    for(int i = 0; i < header.numberFiles; i++){
        buffer = new char[sizeof(PAKfileEntry)];
        PAKfileEntry entry;
        PAKread.read(buffer, sizeof(PAKfileEntry));
        memcpy(&entry, buffer, sizeof(PAKfileEntry)); //store the decrypted stuff into the entry
        entries.push_back(entry); //append to the vector
        delete [] buffer;
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

    vector<PAKfileEntry> original(entries);

    for(unsigned int i = 0; i < changes.size(); i++)
       if(changes[i] >= 0) //count all changes that aren't deletions
	    numberFiles++;

       header.numberFiles = numberFiles;

       int offset = sizeof(PAKheader) + (numberFiles * sizeof(PAKfileEntry));
       for(unsigned int i = 0; i < entries.size(); i++){ //find out new offsets
   	   if(changes[i] == -1) continue; //don't factor in deletions
	   entries[i].offset = offset;
	   offset += entries[i].size;
       }

       if(PAKout.is_open()){
  	   PAKout.write((char *) &header, sizeof(header)); //write out header		
	   char *buffer;
  	   for(unsigned int i = 0; i < entries.size(); i++){
	       if(changes[i] == -1){ //if this change is a deletion
	   	   continue;
	       }
	       buffer = new char[sizeof(PAKfileEntry)]; //char array to hold each table of contents entry
	       memcpy(buffer, &(entries[i]), sizeof(PAKfileEntry)); //copy over the current entry in the for loop

	       PAKout.write(buffer, sizeof(PAKfileEntry)); //finally write the entry
	       delete [] buffer; //no memory leaks in this code, no sir
	   }

	   for(unsigned int i = 0; i < entries.size(); i++){
	       if(changes[i] == -1) continue; //again, don't factor in deletions
	       if(changes[i] == 1){
	    	   PAKin.open(entries[i].fullname, ifstream::binary); //if it's an addition, load the file
	       }else{ //if it's already in the PAK file, load it from there
	           PAKin.open(pakname, ifstream::binary);
		   PAKin.seekg(original[i].offset);
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

char* FilePAK::getPAKEntryData(const std::string& name)
{
    if( PAKfileEntry *entry = getPAKEntry( name ) )	{
        char *buffer = NULL;

        igzstream PAKread;
        PAKread.open(pakname.c_str());//, ios::binary);
	//if(PAKread.is_open())
	{
            buffer = new char[entry->size];
            PAKread.seekg(entry->offset);//, ifstream::beg); //seek to the offset of the file in the .pak file
            PAKread.read(buffer, entry->size); //read everything into the buffer
            return buffer; //return it all
	}
	/*else
	{
	    cout << "Critical error: getPAKEntryData() could not open stream\n";
	    return buffer; //NULL
	}*/
    }
    return NULL; //PAK file isn't loaded, or entry isn't found
}

FilePAK::PAKfileEntry *FilePAK::getPAKEntry(const std::string& name){
    if(pakloaded){
        for(int i = 0; i < header.numberFiles; i++){
            if(strcmp(entries[i].name, name.c_str()) == 0){
                return &entries[i];
	    }
	}
    }
    return NULL; //PAK file isn't loaded, or entry isn't found
}

int FilePAK::getPAKEntrySize(const std::string& name){
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
        char *buffer = getPAKEntryData(name);
        int size = getPAKEntrySize(name);
        if(buffer == NULL || size <= 0) return false;
        output.write(buffer, size);
        delete [] buffer;
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
           FilePAK::PAKfileEntry*r=pak.getPAKEntry(nm);
           cout<<"  size:"<<r->size<<"  offset:"<<r->offset<<" name:"<<r->name<<" fullname:"<<r->fullname<<std::endl;
           pak.getPAKEntryData(nm);
       }
   }
   return 0;
}
#endif
