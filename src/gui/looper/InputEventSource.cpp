#include <looper/FileSource.h>
#include <stdlib.h>  
#include <unistd.h>  
#include <string.h>  
#include <sys/types.h>  
#include <fcntl.h>  
#include <errno.h>  
#include <time.h> 

namespace looper{

int InputEventSource::getType(std::string&type){
#define test_bit(bit) (mask[(bit)/8] & (1 << ((bit)%8)))  
    char          name[64];           /* RATS: Use ok, but could be better */  
    char          buf[256] = { 0, };  /* RATS: Use ok */  
    unsigned char mask[EV_MAX/8 + 1];
    int           version;
    ioctl(fd, EVIOCGVERSION, &version);  
    ioctl(fd, EVIOCGNAME(sizeof(buf)), buf);  
    ioctl(fd, EVIOCGBIT(0, sizeof(mask)), mask);
    printf("%s evdev version:%d.%d.%d", name,version >> 16, (version >> 8) & 0xff, version & 0xff);  
    printf("    name: %s\n", buf);  
    printf("    features:");  
    for (int j = 0; j < EV_MAX; j++) {  
       if (test_bit(j)) {  
            type = "unknown";  
            switch(j) {  
            case EV_KEY: type = "keys/buttons"; break;  
            case EV_REL: type = "relative";     break;  
            case EV_ABS: type = "absolute";     break;  
            case EV_MSC: type = "reserved";     break;  
            case EV_LED: type = "leds";         break;  
            case EV_SND: type = "sound";        break;  
            case EV_REP: type = "repeat";       break;  
            case EV_FF:  type = "feedback";     break;  
            }  
       }  
   }
   return 0;  
}

int InputEventSource::getEvent(struct input_event&event){
    return read(fd,&event,sizeof(struct input_event));
}

void InputEventSource::dumpEvent(struct input_event&event){
    const char          *tmp;
    printf("%-24.24s.%06lu type 0x%04x; code 0x%04x;"  
                       " value 0x%08x; \r\n",  
                       ctime(&event.time.tv_sec),  
                       event.time.tv_usec,  
                       event.type, event.code, event.value);
    switch (event.type) {  
    case EV_KEY:  
          if (event.code > BTN_MISC) {  
               printf("Button %d %s",event.code & 0xff, event.value ? "press" : "release");  
          } else {  
               printf("Key %d (0x%x) %s", event.code & 0xff,event.code & 0xff,event.value ? "press" : "release");  
          }  
          break;  
    case EV_REL:  
          switch (event.code) {  
          case REL_X:      tmp = "X";       break;  
          case REL_Y:      tmp = "Y";       break;  
          case REL_HWHEEL: tmp = "HWHEEL";  break;  
          case REL_DIAL:   tmp = "DIAL";    break;  
          case REL_WHEEL:  tmp = "WHEEL";   break;  
          case REL_MISC:   tmp = "MISC";    break;  
          default:         tmp = "UNKNOWN"; break;  
          }  
          printf("Relative %s %d\r\n", tmp, event.value);  
          break;  
    case EV_ABS:  
         switch (event.code) {  
         case ABS_X:        tmp = "X";        break;  
         case ABS_Y:        tmp = "Y";        break;  
         case ABS_Z:        tmp = "Z";        break;  
         case ABS_RX:       tmp = "RX";       break;  
         case ABS_RY:       tmp = "RY";       break;  
         case ABS_RZ:       tmp = "RZ";       break;  
         case ABS_THROTTLE: tmp = "THROTTLE"; break;  
         case ABS_RUDDER:   tmp = "RUDDER";   break;  
         case ABS_WHEEL:    tmp = "WHEEL";    break;  
         case ABS_GAS:      tmp = "GAS";      break;  
         case ABS_BRAKE:    tmp = "BRAKE";    break;  
         case ABS_HAT0X:    tmp = "HAT0X";    break;  
         case ABS_HAT0Y:    tmp = "HAT0Y";    break;  
         case ABS_HAT1X:    tmp = "HAT1X";    break;  
         case ABS_HAT1Y:    tmp = "HAT1Y";    break;  
         case ABS_HAT2X:    tmp = "HAT2X";    break;  
         case ABS_HAT2Y:    tmp = "HAT2Y";    break;  
         case ABS_HAT3X:    tmp = "HAT3X";    break;  
         case ABS_HAT3Y:    tmp = "HAT3Y";    break;  
         case ABS_PRESSURE: tmp = "PRESSURE"; break;  
         case ABS_DISTANCE: tmp = "DISTANCE"; break;  
         case ABS_TILT_X:   tmp = "TILT_X";   break;  
         case ABS_TILT_Y:   tmp = "TILT_Y";   break;  
         case ABS_MISC:     tmp = "MISC";     break;  
         default:           tmp = "UNKNOWN";  break;  
         }  
         printf("Absolute %s %d\r\n", tmp, event.value);  
         break;  
   case EV_MSC: printf("Misc\r\n"); break;  
   case EV_LED: printf("Led\r\n");  break;  
   case EV_SND: printf("Snd\r\n");  break;  
   case EV_REP: printf("Rep\r\n");  break;  
   case EV_FF:  printf("FF\r\n");   break;  
   break;  
   }  
}

}//namespace looper
