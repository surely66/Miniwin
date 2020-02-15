#include <preferences.h>
#include <iostream>
#include <strstream>
#include <fstream>
#include <ngl_types.h>
#include <ngl_log.h>

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/memorystream.h>

NGL_MODULE(PREFERENCE)

namespace nglui{
Preferences::Preferences(){
   doc=new rapidjson::Document();
   ((rapidjson::Document*)doc)->SetObject();
}

void Preferences::load(const std::string&fname){

    rapidjson::Document&d=*(rapidjson::Document*)doc;
    std::ifstream fin(fname);
    rapidjson::IStreamWrapper isw(fin);
    if(fin.good())
        d.ParseStream(isw);

    NGLOG_VERBOSE("parse=%s",fname.c_str());
}
void Preferences::save(const std::string&fname){
    rapidjson::Document&d=*(rapidjson::Document*)doc;
    rapidjson::Document::AllocatorType& allocator = d.GetAllocator();
    std::ofstream fout(fname);
    rapidjson::OStreamWrapper out(fout);
    rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(out);
    d.Accept(writer);
}

//void Preferences::getSection(const std::string&section,Json::Value&value){
//}

bool Preferences::getBool(const std::string&section,const std::string&key,bool def){
    rapidjson::Document&d=*(rapidjson::Document*)doc;
    if(!d.HasMember(section))return def;
    if(!d[section].HasMember(key))return def;
    if(!d[section][key].IsBool())return def;
    return d[section][key].GetBool();
}

int Preferences::getInt(const std::string&section,const std::string&key,int def){
    rapidjson::Document&d=*(rapidjson::Document*)doc;
    if(!d.HasMember(section))return def;
    if(!d[section].HasMember(key))return def;
    if(!d[section][key].IsInt())return def;
    NGLOG_VERBOSE("%s.%s=%d",section.c_str(),key.c_str(),d[section][key].GetInt());
    return d[section][key].GetInt();
}

float Preferences::getFloat(const std::string&section,const std::string&key,float def){
    rapidjson::Document&d=*(rapidjson::Document*)doc;
    if(!d.HasMember(section))return def;
    if(!d[section].HasMember(key))return def;
    if(!d[section][key].IsDouble())return def;
    return d[section][key].GetFloat();
}

double Preferences::getDouble(const std::string&section,const std::string&key,double def){
    rapidjson::Document&d=*(rapidjson::Document*)doc;
    if(!d.HasMember(section))return def;
    if(!d[section].HasMember(key))return def;
    if(!d[section][key].IsDouble())return def;
    return d[section][key].GetDouble();
}

const std::string& Preferences::getString(const std::string&section,const std::string&key,const std::string&def){
    rapidjson::Document&d=*(rapidjson::Document*)doc;
    if(!d.HasMember(section))return def;
    if(!d[section].HasMember(key))return def;
    if(!d[section][key].IsString())return def;
    return d[section][key].GetString();
}

void Preferences::setValue(const std::string&section,const std::string&key,bool v){
    rapidjson::Document&d=*(rapidjson::Document*)doc;
    d[section][key]=v;
    //NGLOG_VERBOSE("json=%s",root.toStyledString().c_str());
}
void Preferences::setValue(const std::string&section,const std::string&key,int v){
    rapidjson::Document&d=*(rapidjson::Document*)doc;
    d[section][key]=v;
    //NGLOG_VERBOSE("after setint json=%s",root.toStyledString().c_str());
}
void Preferences::setValue(const std::string&section,const std::string&key,float v){
    rapidjson::Document&d=*(rapidjson::Document*)doc;
    d[section][key]=v;
    //NGLOG_VERBOSE("json=%s",root.toStyledString().c_str());
}

void Preferences::setValue(const std::string&section,const std::string&key,const std::string& v){
    rapidjson::Document&d=*(rapidjson::Document*)doc;
    d[section][key].SetString(v.c_str(),d.GetAllocator());
    //NGLOG_VERBOSE("json=%s",root.toStyledString().c_str());
}
void Preferences::setValue(const std::string&section,const std::string&key,double v){
    rapidjson::Document&d=*(rapidjson::Document*)doc;
    d[section][key]=v;
    //NGLOG_VERBOSE("json=%s",root.toStyledString().c_str());
}

}//namespace
