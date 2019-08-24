#include <ngl_types.h>
#include <aui_dmx.h>
#include <aui_common.h>
#include <aui_av.h>
#include <aui_snd.h>
#include <aui_dis.h>
#include <ngl_video.h>
#include <ngl_log.h>
#include <ngl_disp.h>
#include <ngl_snd.h>

NGL_MODULE(AV)

typedef struct{
    aui_hdl hdl_av;
    aui_av_stream_info_t stream_info;
    aui_attrAV attr;
}NGLAV;

#define NB_DEMUX 4 

static NGLAV sAvPlayers[NB_DEMUX];
static aui_hdl _g_dis_handle_hd,_g_dis_handle_sd;
void init_dis_handle(void);

static int VTYPE2AUI(int nt){
  switch(nt){
  case DECV_MPEG:return AUI_DECV_FORMAT_MPEG;
  case DECV_AVC: return AUI_DECV_FORMAT_AVC;
  case DECV_AVS: return AUI_DECV_FORMAT_AVS;
  case DECV_XVID:return AUI_DECV_FORMAT_XVID;
  case DECV_FLV1:return AUI_DECV_FORMAT_FLV1;
  case DECV_VP8: return AUI_DECV_FORMAT_VP8;
  case DECV_WVC1:return AUI_DECV_FORMAT_WVC1;
  case DECV_WX3: return AUI_DECV_FORMAT_WX3;
  case DECV_RMVB:return AUI_DECV_FORMAT_RMVB;
  case DECV_MJPG:return AUI_DECV_FORMAT_MJPG;
  case DECV_HEVC:return AUI_DECV_FORMAT_HEVC;
  case DECV_INVALID:
  default:return AUI_DECV_FORMAT_INVALID;
  }
}
static int ATYPE2AUI(int nt){
  switch(nt){
  case DECA_MPEG1:return AUI_DECA_STREAM_TYPE_MPEG1;
  case DECA_MPEG2:return AUI_DECA_STREAM_TYPE_MPEG2;
  case DECA_AAC_LATM:return AUI_DECA_STREAM_TYPE_AAC_LATM;
  case DECA_AC3:return AUI_DECA_STREAM_TYPE_AC3;
  case DECA_DTS:return AUI_DECA_STREAM_TYPE_DTS;
  case DECA_PPCM:return AUI_DECA_STREAM_TYPE_PPCM;
  case DECA_LPCM_V:return AUI_DECA_STREAM_TYPE_LPCM_V;
  case DECA_LPCM_A:return AUI_DECA_STREAM_TYPE_LPCM_A;
  case DECA_PCM:return AUI_DECA_STREAM_TYPE_PCM;
  case DECA_BYE1:return AUI_DECA_STREAM_TYPE_BYE1;
  case DECA_RA8:return  AUI_DECA_STREAM_TYPE_RA8;
  case DECA_MP3:return AUI_DECA_STREAM_TYPE_MP3;
  case DECA_AAC_ADTS:return AUI_DECA_STREAM_TYPE_AAC_ADTS;
  case DECA_OGG:return AUI_DECA_STREAM_TYPE_OGG;
  case DECA_EC3:return AUI_DECA_STREAM_TYPE_EC3;
  case DECA_MP3_L3:return AUI_DECA_STREAM_TYPE_MP3_L3;
  case DECA_RAW_PCM:return AUI_DECA_STREAM_TYPE_RAW_PCM;
  case DECA_BYE1PRO:return AUI_DECA_STREAM_TYPE_BYE1PRO;
  case DECA_FLAC: return AUI_DECA_STREAM_TYPE_FLAC;
  case DECA_APE: return AUI_DECA_STREAM_TYPE_APE;
  case DECA_MP3_2:return AUI_DECA_STREAM_TYPE_MP3_2;
  case DECA_AMR: return AUI_DECA_STREAM_TYPE_AMR;
  case DECA_ADPCM:AUI_DECA_STREAM_TYPE_ADPCM;
  case DECA_INVALID:
  default:return AUI_DECA_STREAM_TYPE_INVALID;
  }
}

INT nglAvInit(){
    int i,rc;
    int dmxid[]={AUI_DMX_ID_DEMUX0,AUI_DMX_ID_DEMUX1,AUI_DMX_ID_SW_DEMUX0,AUI_DMX_ID_SW_DEMUX1};
    int datatype[]={AUI_AV_DATA_TYPE_NIM_TS,AUI_AV_DATA_TYPE_RAM_TS,AUI_AV_DATA_TYPE_RAM_TS,AUI_AV_DATA_TYPE_RAM_TS};
    aui_av_init(NULL);
    aui_snd_init(NULL,NULL);
    aui_log_priority_set(AUI_MODULE_AV,AUI_LOG_PRIO_DEBUG);
    nglDispInit();
    nglSndInit();
    NGLOG_DEBUG("nglAvInit");
    for(i=0;i<NB_DEMUX;i++){
        NGLAV*av=&sAvPlayers[i];
        av->hdl_av=NULL;
        memset(&av->stream_info,0,sizeof(aui_av_stream_info_t));
        memset(&av->attr,0,sizeof(aui_attrAV));
        av->stream_info.stream_type.dmx_id=dmxid[i];
        av->stream_info.stream_type.data_type=datatype[i];
        av->stream_info.st_av_info.en_spdif_type = AUI_SND_OUT_MODE_DECODED;
        if(aui_find_dev_by_idx(AUI_MODULE_DIS,0,&av->attr.pv_hdl_dis_hd)){
             aui_attr_dis attr_dis;
             memset(&attr_dis, 0, sizeof(attr_dis));
             attr_dis.uc_dev_idx = AUI_DIS_HD;
             rc=aui_dis_open(&attr_dis, &av->attr.pv_hdl_dis_hd);
             NGLOG_DEBUG("aui_dis_open=%d",rc);
        }
    
        if(aui_find_dev_by_idx(AUI_MODULE_DIS,1,&av->attr.pv_hdl_dis_hd)){
             aui_attr_dis attr_dis;
             memset(&attr_dis, 0, sizeof(attr_dis));
             attr_dis.uc_dev_idx = AUI_DIS_SD;
             rc=aui_dis_open(&attr_dis, &av->attr.pv_hdl_dis_sd);
             NGLOG_DEBUG("aui_dis_open=%d",rc);
        }
         NGLOG_DEBUG("attr.pv_hdl_dis_hd=%p  sd=%d",av->attr.pv_hdl_dis_hd,av->attr.pv_hdl_dis_hd);
    }
    nglDispSetAspectRatio(DISP_APR_AUTO);
}

INT nglAvPlay(int dmxid,int vid,int vtype,int aid,int atype,int pcr)
{
     aui_hdl hdl_av;
     NGLAV *av=&sAvPlayers[dmxid];
     if(NULL!=av->hdl_av){
         aui_av_stop(av->hdl_av);
         aui_av_close(av->hdl_av);
         NGLOG_DEBUG("stop prev Player hdl_av=%p",av->hdl_av);
     }
     av->stream_info.st_av_info.b_audio_enable = aid!=NGL_INVALID_PID;
     av->stream_info.st_av_info.b_video_enable = vid!=NGL_INVALID_PID;
     av->stream_info.st_av_info.b_pcr_enable = pcr!=NGL_INVALID_PID;
     av->stream_info.st_av_info.b_dmx_enable = 1;
     av->stream_info.st_av_info.ui_audio_pid = aid;
     av->stream_info.st_av_info.ui_video_pid = vid;
     av->stream_info.st_av_info.ui_pcr_pid = pcr;
     av->stream_info.st_av_info.en_audio_stream_type =ATYPE2AUI(atype);// AUI_DECA_STREAM_TYPE_MPEG2;//audio_type;
     av->stream_info.st_av_info.en_video_stream_type =VTYPE2AUI(vtype);//AUI_DECV_FORMAT_MPEG;//vtype;// AUI_DECV_FORMAT_MPEG;//video_type;

     aui_av_init_attr(&av->attr, &av->stream_info);
     
     int rc=aui_av_open(&av->attr, &av->hdl_av);
     NGLOG_DEBUG_IF(rc,"aui_av_open=%d hdl_av=%p",rc,av->hdl_av);

     aui_dmx_data_path data_path_info;
     memset(&data_path_info, 0, sizeof(data_path_info));
     data_path_info.data_path_type = AUI_DMX_DATA_PATH_CLEAR_PLAY;
     rc=aui_dmx_data_path_set(av->attr.pv_hdl_dmx, &data_path_info);
     NGLOG_DEBUG("aui_dmx_data_path_set dmx=%p rc=%d",av->attr.pv_hdl_dmx,rc);

     rc=aui_av_start(av->hdl_av);
     NGLOG_DEBUG("aui_av_start=%d video=%d/%d audio=%d/%d pcr=%d",rc,vid,vtype,aid,atype,pcr);
}

INT nglAvSetVideoWindow(int dmxid,const NGLRect*inRect,const NGLRect*outRect){
     NGLAV*av=&sAvPlayers[dmxid];
     NGLRect src={0,0,1280,720};
     NGLRect dst={0,0,1280,720};    
     if(inRect)src=*inRect;
     if(outRect)dst=*outRect; 
     src.x=src.x*720/1280;
     src.y=src.y*2880/720;
     src.w=src.w*720/1280;
     src.h=src.h*2880/720;

     dst.x=dst.x*720/1280;
     dst.y=dst.y*2880/720;
     dst.w=dst.w*720/1280;
     dst.h=dst.h*2880/720;
     int rc;
     int mode=(dst.w>=src.w&&dst.h>=src.h)?AUI_VIEW_MODE_FULL:AUI_VIEW_MODE_PREVIEW;
     aui_hdl dis_hdl,dis_sd;
     aui_find_dev_by_idx( AUI_MODULE_DIS, AUI_DIS_HD, &dis_hdl ); 
     aui_find_dev_by_idx( AUI_MODULE_DIS, AUI_DIS_SD, &dis_sd ); 

     if(mode==AUI_VIEW_MODE_PREVIEW){
          rc=aui_dis_mode_set(dis_hdl,mode,&src,&dst);
          rc=aui_dis_mode_set(dis_sd,mode,&src,&dst);
     }else{
          rc=aui_dis_mode_set(dis_hdl,mode,NULL,NULL);
          rc=aui_dis_mode_set(dis_sd,mode,NULL,NULL);
     }
     NGLOG_DEBUG("aui_dis_mode_set(hd=%p/%p mode=%d %dx%d)=%d ",av->attr.pv_hdl_dis_hd,dis_hdl,mode,dst.w,dst.h,rc);
     return rc;
}

static void first_i_frame_deocded(void * p_user_hld, unsigned int parm1, unsigned parm2){
	(void)p_user_hld;
	(void)parm1;
	(void)parm2;
	//if(GL_PLAY_Video_Callback)
        //   (*GL_PLAY_Video_Callback)(2,0);
        NGLOG_DEBUG("");
}
static void frame_rate_info_change(void * p_user_hld, unsigned int parm1, unsigned parm2){
	(void)p_user_hld;
	(void)parm2;
	struct aui_decv_info_cb *new_info = (struct aui_decv_info_cb *)parm1;
	//if(GL_PLAY_Video_Callback)
        //(*GL_PLAY_Video_Callback)(1,(int)new_info->frame_rate);
	NGLOG_DEBUG("video_info_change_cb,width =%d,height =%d ,frame_rate =%d \n",new_info->pic_width,new_info->pic_height,new_info->frame_rate);
}

static int PlayStatus = 0;
static void play_status_info_change(void * p_user_hld, unsigned int parm1, unsigned parm2){
	(void)p_user_hld;
	(void)parm2;
	(void)parm1;
	if(PlayStatus != 1)
	   PlayStatus = 1;
        NGLOG_DEBUG("");	
}

static void cb_first_frame(void *p1, void *p2, void *p_usr_data){
    (void)p1;
    (void)p2;
    (void)p_usr_data;	
    if(PlayStatus != 1)
	PlayStatus = 1;	
    NGLOG_DEBUG("");
}


