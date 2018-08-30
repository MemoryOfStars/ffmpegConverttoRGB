   /** 
    *  使用FFmpeg解析出RGB数据 
    */  
      
    #include <stdio.h>  
    #include <iostream>  
    extern "C"  
    {  
    #include "libavcodec/avcodec.h"  
    #include "libavformat/avformat.h"  
    #include "libswscale/swscale.h"  
    #include "libavutil/imgutils.h"  
    };  
      
    #pragma comment(lib, "avcodec.lib")  
    #pragma comment(lib, "avformat.lib")  
    #pragma comment(lib, "swscale.lib")  
    #pragma comment(lib, "avutil.lib")  
    using namespace std;

    int main(int argc, char* argv[])  
    {  
        AVFormatContext     *pFormatCtx = NULL;  
        AVCodecContext      *pCodecCtx = NULL;  
        AVCodec             *pCodec = NULL;  
        AVFrame             *pFrame = NULL, *pFrameRGB = NULL;  
        unsigned char       *out_buffer = NULL;  
        AVPacket            packet;  
        struct SwsContext   *img_convert_ctx = NULL;  
        int                 got_picture;  
        int                 videoIndex;  
        int                 frame_cnt = 1;  
        char filepath[] = "ws.mp4";  
        FILE *fp_rgb = fopen("Output.rgb", "wb+");
        FILE *fp_sample;
        char filename[30];



        if (fp_rgb == NULL)  
        {  
            printf("FILE open error");  
            return -1;  
        }  
        av_register_all();  
        if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0){  
            printf("Couldn't open an input stream.\n");  
            return -1;  
        }  
        if (avformat_find_stream_info(pFormatCtx, NULL) < 0){  
            printf("Couldn't find stream information.\n");  
            return -1;  
        }  
        videoIndex = -1;  
        for (int i = 0; i < pFormatCtx->nb_streams; i++)  
            if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){  
                videoIndex = i;  
                break;  
            }  
        if (videoIndex == -1){  
            printf("Couldn't find a video stream.\n");  
            return -1;  
        }  
        pCodecCtx = pFormatCtx->streams[videoIndex]->codec;  
        pCodec = avcodec_find_decoder(pCodecCtx->codec_id);  
        if (pCodec == NULL){  
            printf("Codec not found.\n");  
            return -1;  
        }  
        if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0){  
            printf("Could not open codec.\n");  
            return -1;  
        }  
        pFrame = av_frame_alloc();  
        pFrameRGB = av_frame_alloc();  
        if (pFrame == NULL || pFrameRGB == NULL)  
        {  
            printf("memory allocation error\n");  
            return -1;  
        }  
        /** 
        *  RGB--------->AV_PIX_FMT_RGB24 
        *  YUV420P----->AV_PIX_FMT_YUV420P 
        *  UYVY422----->AV_PIX_FMT_UYVY422 
        *  YUV422P----->AV_PIX_FMT_YUV422P 
        */  
        out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24, 480, 272, 1));  
        av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_buffer,  
            AV_PIX_FMT_RGB24, 480, 272, 1);
        img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,  
            480, 272, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
        while (av_read_frame(pFormatCtx, &packet) >= 0)  
        {  
            if (packet.stream_index == videoIndex)  
            {  
                if (avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet) < 0)  
                {  
                    printf("Decode Error.\n");  
                    return -1;  
                }  
                if (got_picture)  
                {  
                    sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,  
                        pFrameRGB->data, pFrameRGB->linesize);  
                    fwrite(pFrameRGB->data[0], 1,(480) * (272) * 3,  fp_rgb);

                    
                    sprintf(filename,"%s%d%s","images/",frame_cnt,".rgb");
                    cout<<filename<<endl;
                    fp_sample=fopen(filename, "wb+");
                    fwrite(pFrameRGB->data[0], 1,(480) * (272) * 3,  fp_sample);
                    fclose(fp_sample);  
                    printf("Succeed to decode %d frame!\n", frame_cnt);  
                    frame_cnt++;  
                }  
            }  
            av_free_packet(&packet);  
        }  


        fclose(fp_rgb);   
        sws_freeContext(img_convert_ctx);  
        av_free(out_buffer);  
        av_frame_free(&pFrameRGB);  
        av_frame_free(&pFrame);  
        avcodec_close(pCodecCtx);  
        avformat_close_input(&pFormatCtx);  
        return 0;  
    }  
