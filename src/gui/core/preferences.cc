#include <preferences.h>
#include <iostream>
#include <strstream>
#include <fstream>
#include <ngl_types.h>
#include <ngl_log.h>

NGL_MODULE(PREFERENCE)

namespace nglui{
Preferences::Preferences(){
}

void Preferences::load(const std::string&fname){
    Json::CharReaderBuilder builder;
    Json::String errs;
    builder["collectComments"] = false;
    std::ifstream fin(fname);
    bool rc=Json::parseFromStream(builder,fin, &root, &errs);//reader.parse(data,data+size,root);
    NGLOG_VERBOSE("%s parse=%d %s",fname.c_str(),rc,errs.c_str());
}
void Preferences::save(const std::string&fname){
    Json::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::ofstream fout(fname);
    writer->write(root,&fout);
}

void Preferences::getSection(const std::string&section,Json::Value&value){
}
bool Preferences::getBool(const std::string&section,const std::string&key,bool def){
    if(!root.isMember(section))return def;
    if(!root[section].isMember(key))return def;
    if(!root[section][key].isBool())return def;
    return root[section][key].asBool();
}

int Preferences::getInt(const std::string&section,const std::string&key,int def){
    if(!root.isMember(section))return def;
    if(!root[section].isMember(key))return def;
    if(!root[section][key].isInt())return def;
    NGLOG_VERBOSE("%s.%s=%d",section.c_str(),key.c_str(),root[section][key].asInt());
    return root[section][key].asInt();
}

float Preferences::getFloat(const std::string&section,const std::string&key,float def){
    if(!root.isMember(section))return def;
    if(!root[section].isMember(key))return def;
    if(!root[section][key].isDouble())return def;
    return root[section][key].asFloat();
}

double Preferences::getDouble(const std::string&section,const std::string&key,double def){
    if(!root.isMember(section))return def;
    if(!root[section].isMember(key))return def;
    if(!root[section][key].isDouble())return def;
    return root[section][key].asDouble();
}

const std::string& Preferences::getString(const std::string&section,const std::string&key,const std::string&def){
    if(!root.isMember(section))return def;
    if(!root[section].isMember(key))return def;
    if(!root[section][key].isString())return def;
    return root[section][key].asString();
}

void Preferences::setValue(const std::string&section,const std::string&key,bool v){
    root[section][key]=v;
    NGLOG_VERBOSE("json=%s",root.toStyledString().c_str());
}
void Preferences::setValue(const std::string&section,const std::string&key,int v){
    root[section][key]=v;
    NGLOG_VERBOSE("after setint json=%s",root.toStyledString().c_str());
}
void Preferences::setValue(const std::string&section,const std::string&key,float v){
    root[section][key]=v;
    NGLOG_VERBOSE("json=%s",root.toStyledString().c_str());
}

void Preferences::setValue(const std::string&section,const std::string&key,const std::string& v){
    root[section][key]=v;
    NGLOG_VERBOSE("json=%s",root.toStyledString().c_str());
}
void Preferences::setValue(const std::string&section,const std::string&key,double v){
    root[section][key]=v;
    NGLOG_VERBOSE("json=%s",root.toStyledString().c_str());
}

}//namespace
