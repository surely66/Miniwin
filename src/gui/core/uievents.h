#ifndef __NGL_UIEVENTS_H__
#define __NGL_UIEVENTS_H__

namespace nglui{

typedef enum {
  EVENT_KEY=0,
  EVENT_MOUSE=1,
  EVENT_MOTION=2,
}EVENTTYPE;

class Event{
protected:
  int event_type;
  int x;
  int y;
public:   
  Event(int tp=0){
      event_type=(EVENTTYPE)tp;
  }
  int getType(){return event_type;}
  int getX(){return x;}
  int getY(){return y;}
//  static Event&make(){Event e ;return e;}
};

class KeyEvent:public Event{
private:
  int key_code;
  int key_state;
  int key_repeat;
public:
  KeyEvent(int key,int state,int repeat=0):Event(EVENT_KEY){
      key_code=key;
      key_state=state;
      key_repeat=repeat;
  }
  int getKeyCode() {return key_code;}
  void setKeyCode(int k){key_code=k;}
  int getKeyState(){return key_state;}
  int getCount(){return key_repeat;}
};
}
#endif
