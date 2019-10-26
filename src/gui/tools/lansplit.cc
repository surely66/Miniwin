#include <json/json.h>
#include <iostream>
#include <fstream>
#include <algorithm>

typedef std::function<void(void)>VOIDFUN;

int main(int argc,char*argv[]){
    Json::Value root;
    Json::String errs;
    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    std::ifstream sin(argv[1]);
    bool rc=Json::parseFromStream(builder,sin, &root, &errs);//reader.parse(data,data+size,root);
    if(!root.isArray()){
        printf("invalid language file\r\n");
        return -1;
    }
    Json::Value jlans;
    for(int i=0;i<root.size();i++){
         Json::Value v=root[i];
         Json::Value::Members mems=v.getMemberNames();
         if(!v.isMember("valuename")||mems.size()<2){
             printf("invalid language format\r\n");
             continue;
         }
         std::string strname=v["valuename"].asString();
         std::transform(strname.begin(), strname.end(), strname.begin(), ::tolower);
         if(strname.empty())continue;
         for(int j=0;j<mems.size();j++){
              if(mems[j]==strname)continue;
              jlans[mems[j]][strname]=v[mems[j]];
         }
    }
    Json::Value::Members lannames=jlans.getMemberNames();
    for(int i=0;i<lannames.size();i++){
        if(lannames[i]=="valuename")continue;
        std::string ofname="strings-"+lannames[i]+".json";
        Json::StreamWriterBuilder builder;
        builder["commentStyle"] = "None";
        std::transform(ofname.begin(), ofname.end(), ofname.begin(), ::tolower);
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        std::ofstream fout(ofname);
        writer->write(jlans[lannames[i]],&fout);
    }
    return 0;
}
