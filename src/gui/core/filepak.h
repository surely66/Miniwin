//
//			PAK File Compression
//
//		Packs any set of files (best used on resource files in a program. Images, sounds, etc.) into one .pak file.
//		Each file is encrypted with the caesar encryption, it's really simple: choose a random value between 0 and 255 and
//		randomly choose to either add or subtract then store those values in the files header. Then whenever you store a
//		file you add or subtract that value to every byte.
//
//		Reminder: char represents a byte


#ifndef FILEPAK_H
#define FILEPAH_H

#include <string.h>
#include <vector>
#include <stdio.h>
#include <string>

using namespace std;

namespace nglui{

class FilePAK
{
public:
    struct PAKFileEntry{ //Basically an entry in a table of contents for each file stored in the .pak file
       char name[64]; //name of the file, must be unique from other files
       int type;
       unsigned int size; //size of the file in bytes
       unsigned int offset; //offset of where the files in located in the .pak file in bytes
//       int changes; //corresponds with entries: -1 = deleted, 0 = normal, 1 = added
       char fullname[150]; //name of the file + the folder it is in
    };
private:
     struct PAKHeader{ //The header for the .pak file, the only thing not encyrpted
         char fileID[6]; //Identifier to make sure this is the type of .pak we want (and not some other sort of compression someone else made), I use "DBPAK"
         char version[4]; //Version of the format
         int numberFiles; //Total number of files
     };

     std::string pakname; //name of the pak file
     bool pakloaded; //set to true after readPAK() is used
     int lastEntry;
     PAKHeader header; //the header
     vector<PAKFileEntry> entries; //table of contents of all the entries
     vector<int> changes; //corresponds with entries: -1 = deleted, 0 = normal, 1 = added
	//Used to split the parameter types in createPAK() into a vector 
     vector<std::string> filetypes(const std::string& types);

     //Create PAKfileEntry
     bool createEntry(const std::string& fullname,const std::string& name);
     int readDirectory(const std::string&path,const vector<std::string>&correctTypes,const std::string&basedir);

public:
     FilePAK(void);
     ~FilePAK(void);

     //Creates new PAK file
     //name - name of PAK file to be created 
     //entryPath - path to the folder that contains all the files you want in the PAK file
     //filetypes - all filetypes to be included, seperate by |, ex: ".jpg|.png|.bmp"
     //Returns true if nothing goes wrong
     bool createPAK(const std::string& name,const std::string& entryPath,const std::string& types = "");

     //Reads a PAK file's header and entries into memory so you can manipulate it/decrypt files stored within it  
     //PAKpath - path to the PAK file to read
     //Returns true if nothing goes wrong
     bool readPAK(const std::string& PAKpath);

     //----------------------------------------------------------
     // The following functions require readPAK to be run first:
     //----------------------------------------------------------

     //Appends file to PAK
     //Run rebuildPAK() to flush changes
     //filePath - path to file to append
     //Returns true if nothing goes wrong
     //TODO: implement
     bool appendFile(const std::string& filePath);

     //Rebuilds the PAK file with buffered changes
     //Returns true if nothing goes wrong, also returns false if there are no changes to flush
     bool rebuildPAK();

	//
     bool removeFile(const std::string& name);

     //Get a file data stored in the PAK file
     //name - name of the file stored in the PAK file (don't include the folder/path)
     //Returns a pointer to the file data in memory
     void*getPAKEntryData(const std::string& name);
     static void freeEntryData(void*buffer);
     size_t getPAKEntrySize(const std::string& name);
     //Get a file stored in the PAK file
     //name - name of the file stored in the PAK file (don't include the folder/path)
     //Returns a pointer to the PAKfileEntry
     PAKFileEntry *getPAKEntry(const std::string& name);

     //Returns names of all PAK entries within the
     vector<string> getAllPAKEntries();

     //Returns the number of entries in the pak file
     int getNumPAKEntries();

     //Unpaks a PAK entry
     //name - entry to unPAK
     //path - folder to unPAK to
     //Returns true if nothing goes wrong
     bool unPAKEntry(const std::string& name, const std::string& path);

};
}//end namespace
#endif
