#ifndef __NGLUI_EDITBOX_H__
#define __NGLUI_EDITBOX_H__
#include <widget.h>

namespace nglui{

class EditBox:public Widget{
public:
  DECLARE_UIEVENT(void,AfterTextChanged,EditBox&);
protected:
  int edit_mode_;//0-->insert mode 1-->replace_mode
  int caretPos;
  int label_width_;
  int label_alignment_;
  std::string label_;
  std::string inputPattern;
  int labelBkColor;
  AfterTextChanged afterChanged;
  bool match();
public:
  EditBox(int w,int h);
  EditBox(const std::string&txt,int w,int h);
  static int Unicode2UTF8(UINT unicode,std::string&utf8);
  void setLabelColor(int color);
  const std::string&replace(size_t start,size_t len,const std::string&txt);
  const std::string&replace(size_t start,size_t len,const char*txt,size_t size);
  void setPattern(const std::string&pattern);
  virtual void setTextWatcher(AfterTextChanged ls);
  virtual void setLabelWidth(int w);
  virtual void setLabel(const std::string&txt);
  virtual void setLabelAlignment(int align);
  virtual const std::string&getLabel();
  virtual void setEditMode(int mode);
  virtual void onDraw(GraphContext&ctx)override;
  virtual void setCaretPos(int idx);
  virtual int getCaretPos();
  virtual bool onKeyRelease(KeyEvent&evt)override; 
  virtual bool onMessage(DWORD msg,DWORD wp,ULONG lp);
};
}//endof nglui

#endif

