#include "filepak.h"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <random>
#include <time.h>
#include <dirent.h>
#include <gzstream.h>
#include <memory>

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
   }else if(argc==1){
       cout<<"1: package a dir to resource pak "<<endl;
       cout<<"    "<<argv[0]<<" pakfile resource_dir"<<endl;
       cout<<"2: list resources of resourcepak"<<endl;
       cout<<"    "<<argv[0]<<" pak_file"<<endl;
   }
   return 0;
}
