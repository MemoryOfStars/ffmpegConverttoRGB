#include <stdio.h>

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include <libavutil/imgutils.h>
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#ifdef __cplusplus
};
#endif
#endif


int main(int argc, char* argv[])
{
    AVFormatContext *pFormatCtx;                     //视频文件的信息
    int             i, videoindex;                
    AVCodecContext  *pCodecCtx;
    AVCodec         *pCodec;
    AVFrame *pFrame,*pFrameYUV;
    uint8_t *out_buffer;
    AVPacket *packet;
    int y_size;
    int ret, got_picture;
    struct SwsContext *img_convert_ctx;

    char filepath[]="ws2.mp4";
    
    //FILE *fp_yuv=fopen("output.rgb","wb+");          //yuv文件

    FILE *fp_RGB=fopen("Output.rgb","wb+");        //h264文件
    /*if(fp_RGB == NULL)
    {
        printf("FILE open error");
        return -1;

    }*/
    av_register_all();//注册所有组件
    avformat_network_init();//初始化网络
    pFormatCtx = avformat_alloc_context();//初始化一个AVFormatContext

    if(avformat_open_input(&pFormatCtx,filepath,NULL,NULL)!=0){//打开输入的视频文件
        printf("Couldn't open input stream.\n");
        return -1;
    }
    if(avformat_find_stream_info(pFormatCtx,NULL)<0){//获取视频文件信息
        printf("Couldn't find stream information.\n");
        return -1;
    }
    videoindex=-1;
    for(i=0; i<pFormatCtx->nb_streams; i++) 
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
            videoindex=i;                        //遍历视频文件中所有的流，找到视频流的index
            break;
        }

    if(videoindex==-1){
        printf("Didn't find a video stream.\n");
        return -1;
    }

    pCodecCtx=pFormatCtx->streams[videoindex]->codec;//取得视频流的解码器
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);//查找解码器

    
    if(pCodec==NULL){
        printf("Codec not found.\n");
        return -1;
    }

    printf("解码器名字：%s",pCodec->long_name);

    if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){//打开解码器--------用pCodec去初始化pCodecCtx
        printf("Could not open codec.\n");
        return -1;
    }

    pFrame=av_frame_alloc();                    //为解码帧分配内存
    pFrameYUV=av_frame_alloc();                 //分配内存

    /*if(pFrame == NULL || pFrameYUV == NULL)
    {
        printf("memory allocation error");
        return -1;        
    }*/

    out_buffer=(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height,1));
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,  
            AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);
    packet=(AVPacket *)av_malloc(sizeof(AVPacket));
    //Output Info-----------------------------
    printf("--------------- File Information ----------------\n");
    av_dump_format(pFormatCtx,0,filepath,0);
    printf("-------------------------------------------------\n");
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 
        pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL); 
    int count = 0;
    char image_name[30];
    while(av_read_frame(pFormatCtx, packet)>=0){//读取一帧压缩数据
        
        if(packet->stream_index==videoindex){

            //fwrite(packet->data,1,packet->size,fp_h264); //把H264数据写入fp_h264文件

            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);//解码一帧压缩数据
            if(ret < 0){
                printf("Decode Error.\n");
                return -1;
            }
            if(got_picture){
                sprintf(image_name, "%s%d%s", "MVimagesNotHeight/image", ++count, ".rgb");//保存的图片名
                FILE *fp_yuv=fopen(image_name,"wb+");          //yuv文件
                sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, 
                    pFrameYUV->data, pFrameYUV->linesize);

                y_size=480*272*3; //这样是正确的
                printf("Width:%d,Height:%d\n",pCodecCtx->width,pCodecCtx->height);
                //y_size=(pCodecCtx->width)*(pCodecCtx->height)*3;
                fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);
                fwrite(pFrameYUV->data[0],1,y_size,fp_RGB);
                fclose(fp_yuv);
                printf("Succeed to decode %d frame!\n",count);

            }
        }
        av_free_packet(packet);


        
    }










    
    
    fclose(fp_RGB);
    sws_freeContext(img_convert_ctx);

    
    av_free(out_buffer);
    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    return 0;
}
