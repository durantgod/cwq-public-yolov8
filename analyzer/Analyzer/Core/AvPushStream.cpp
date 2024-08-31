#include "AvPushStream.h"
#include "Config.h"
#include "Utils/Log.h"
#include "Utils/Common.h"
#include "Control.h"
#include "Frame.h"
#include "Worker.h"
#include "Analyzer.h"
extern "C" {
#include "libswscale/swscale.h"
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
}
#pragma warning(disable: 4996)

namespace AVSAnalyzer {
    AvPushStream::AvPushStream(Worker* worker) :
        mWorker(worker)
    {
        LOGI("");
    }

    AvPushStream::~AvPushStream()
    {
        LOGI("");
        closeConnect();
        clearVideoFrameQueue();
    }


    bool AvPushStream::connect() {

        std::string pushStreamUrl = mWorker->mControl->pushStreamUrl;
        int videoWidth = mWorker->mControl->videoWidth;
        int videoHeight = mWorker->mControl->videoHeight;
        int videoFps = mWorker->mControl->videoFps;

        if (avformat_alloc_output_context2(&mFmtCtx, NULL, "rtsp", pushStreamUrl.data()) < 0) {
            LOGI("avformat_alloc_output_context2 error: pushStreamUrl=%s", pushStreamUrl.data());
            return false;
        }

        // init video start
        const AVCodec* videoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
        if (!videoCodec) {
            LOGI("avcodec_find_encoder error: pushStreamUrl=%s", pushStreamUrl.data());
            return false;
        }
        mVideoCodecCtx = avcodec_alloc_context3(videoCodec);
        if (!mVideoCodecCtx) {
            LOGI("avcodec_alloc_context3 error: pushStreamUrl=%s", pushStreamUrl.data());
            return false;
        }
        //int bit_rate = 300 * 1024 * 8;  //压缩后每秒视频的bit位大小 300kB
        int bit_rate = 4096000;
        // CBR：Constant BitRate - 固定比特率
        mVideoCodecCtx->flags |= AV_CODEC_FLAG_QSCALE;
        mVideoCodecCtx->bit_rate = bit_rate;
        mVideoCodecCtx->rc_min_rate = bit_rate;
        mVideoCodecCtx->rc_max_rate = bit_rate;
        mVideoCodecCtx->bit_rate_tolerance = bit_rate;

        //VBR
    //        mVideoCodecCtx->flags |= AV_CODEC_FLAG_QSCALE;
    //        mVideoCodecCtx->rc_min_rate = bit_rate / 2;
    //        mVideoCodecCtx->rc_max_rate = bit_rate / 2 + bit_rate;
    //        mVideoCodecCtx->bit_rate = bit_rate;

        //ABR：Average Bitrate - 平均码率
    //        mDstVimVideoCodecCtxdeoCodecCtx->bit_rate = bit_rate;

        mVideoCodecCtx->codec_id = videoCodec->id;
        mVideoCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;// 不支持AV_PIX_FMT_BGR24直接进行编码
        mVideoCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
        mVideoCodecCtx->width = videoWidth;
        mVideoCodecCtx->height = videoHeight;
        mVideoCodecCtx->time_base = { 1,videoFps };
        //        mDstVideoCodecCtx->framerate = { mDstVideoFps, 1 };
        mVideoCodecCtx->gop_size = 5;
        mVideoCodecCtx->max_b_frames = 0;
        mVideoCodecCtx->thread_count = 1;
        mVideoCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;   //添加PPS、SPS
        AVDictionary* video_codec_options = NULL;

        //H.264
        if (mVideoCodecCtx->codec_id == AV_CODEC_ID_H264) {
            //            av_dict_set(&video_codec_options, "profile", "main", 0);
            av_dict_set(&video_codec_options, "preset", "superfast", 0);
            av_dict_set(&video_codec_options, "tune", "zerolatency", 0);
        }
        //H.265
        if (mVideoCodecCtx->codec_id == AV_CODEC_ID_H265) {
            av_dict_set(&video_codec_options, "preset", "ultrafast", 0);
            av_dict_set(&video_codec_options, "tune", "zero-latency", 0);
        }
        if (avcodec_open2(mVideoCodecCtx, videoCodec, &video_codec_options) < 0) {
            LOGI("avcodec_open2 error: pushStreamUrl=%s", pushStreamUrl.data());
            return false;
        }
        mVideoStream = avformat_new_stream(mFmtCtx, videoCodec);
        if (!mVideoStream) {
            LOGI("avformat_new_stream error: pushStreamUrl=%s", pushStreamUrl.data());
            return false;
        }
        mVideoStream->id = mFmtCtx->nb_streams - 1;
        // stream的time_base参数非常重要，它表示将现实中的一秒钟分为多少个时间基, 在下面调用avformat_write_header时自动完成
        avcodec_parameters_from_context(mVideoStream->codecpar, mVideoCodecCtx);
        mVideoIndex = mVideoStream->id;
        // init video end

        av_dump_format(mFmtCtx, 0, pushStreamUrl.data(), 1);

        // open output url
        if (!(mFmtCtx->oformat->flags & AVFMT_NOFILE)) {
            if (avio_open(&mFmtCtx->pb, pushStreamUrl.data(), AVIO_FLAG_WRITE) < 0) {
                LOGI("avio_open error: pushStreamUrl=%s",pushStreamUrl.data());
                return false;
            }
        }


        AVDictionary* fmt_options = NULL;
        //av_dict_set(&fmt_options, "bufsize", "1024", 0);
        av_dict_set(&fmt_options, "rw_timeout", "30000000", 0); //设置rtmp/http-flv连接超时（单位 us）
        av_dict_set(&fmt_options, "stimeout", "30000000", 0);   //设置rtsp连接超时（单位 us）
        av_dict_set(&fmt_options, "rtsp_transport", "tcp", 0);
        //        av_dict_set(&fmt_options, "fflags", "discardcorrupt", 0);

            //av_dict_set(&fmt_options, "muxdelay", "0.1", 0);
            //av_dict_set(&fmt_options, "tune", "zerolatency", 0);

        mFmtCtx->video_codec_id = mFmtCtx->oformat->video_codec;

        if (avformat_write_header(mFmtCtx, &fmt_options) < 0) { // 调用该函数会将所有stream的time_base，自动设置一个值，通常是1/90000或1/1000，这表示一秒钟表示的时间基长度
            LOGI("avformat_write_header error: pushStreamUrl=%s", pushStreamUrl.data());
            return false;
        }

        mConnectCount++;

        return true;
    }
    bool AvPushStream::reConnect() {
        if (mConnectCount <= 100) {
            closeConnect();

            if (connect()) {
                return true;
            }
            else {
                return false;
            }

        }
        return false;

    }
    void AvPushStream::closeConnect() {
        LOGI("");

        clearVideoFrameQueue();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (mFmtCtx) {
            // 推流需要释放start
            if (mFmtCtx && !(mFmtCtx->oformat->flags & AVFMT_NOFILE)) {
                avio_close(mFmtCtx->pb);
            }
            // 推流需要释放end


            avformat_free_context(mFmtCtx);
            mFmtCtx = NULL;
        }

        if (mVideoCodecCtx) {
            if (mVideoCodecCtx->extradata) {
                av_free(mVideoCodecCtx->extradata);
                mVideoCodecCtx->extradata = NULL;
            }

            avcodec_close(mVideoCodecCtx);
            avcodec_free_context(&mVideoCodecCtx);
            mVideoCodecCtx = NULL;
            mVideoIndex = -1;
        }
    }

    void AvPushStream::addVideoFrame(Frame* frame) {
        mVideoFrameQ_mtx.lock();
        mVideoFrameQ.push(frame);
        mVideoFrameQ_mtx.unlock();
    }
    int AvPushStream::getVideoFrameQSize() {
        int size = 0;
        mVideoFrameQ_mtx.lock();
        size = mVideoFrameQ.size();
        mVideoFrameQ_mtx.unlock();

        return size;
    }

    bool AvPushStream::getVideoFrame(Frame*& frame) {

        mVideoFrameQ_mtx.lock();

        if (!mVideoFrameQ.empty()) {
            frame = mVideoFrameQ.front();
            mVideoFrameQ.pop();
            mVideoFrameQ_mtx.unlock();
            return true;
        }
        else {
            mVideoFrameQ_mtx.unlock();
            return false;
        }

    }
    void AvPushStream::clearVideoFrameQueue() {

        mVideoFrameQ_mtx.lock();
        while (!mVideoFrameQ.empty())
        {
            Frame* frame = mVideoFrameQ.front();
            mVideoFrameQ.pop();
            mWorker->mVideoFramePool->giveBack(frame);
        }
        mVideoFrameQ_mtx.unlock();

    }
    void AvPushStream::handleEncodeVideo() {
        int width = mWorker->mControl->videoWidth;
        int height = mWorker->mControl->videoHeight;

        Frame* videoFrame = NULL; // 未编码的视频帧（bgr格式）

        AVFrame* frame_yuv420p = av_frame_alloc();
        frame_yuv420p->format = mVideoCodecCtx->pix_fmt;
        frame_yuv420p->width = width;
        frame_yuv420p->height = height;

        int frame_yuv420p_buff_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width, height, 1);
        uint8_t* frame_yuv420p_buff = (uint8_t*)av_malloc(frame_yuv420p_buff_size);
        av_image_fill_arrays(frame_yuv420p->data, frame_yuv420p->linesize,
            frame_yuv420p_buff,
            AV_PIX_FMT_YUV420P,
            width, height, 1);



        AVPacket* pkt = av_packet_alloc();// 编码后的视频帧
        int64_t  encodeSuccessCount = 0;
        int64_t  frameCount = 0;

        int64_t t1 = 0;
        int64_t t2 = 0;
        int ret = -1;
        while (mWorker->getState())
        {
            if (getVideoFrame(videoFrame)) {

                // frame_bgr 转  frame_yuv420p
                bgr24ToYuv420p(videoFrame->getBuf(), width, height, frame_yuv420p_buff);
                mWorker->mVideoFramePool->giveBack(videoFrame);


                frame_yuv420p->pts = frame_yuv420p->pkt_dts = av_rescale_q_rnd(frameCount,
                    mVideoCodecCtx->time_base,
                    mVideoStream->time_base,
                    (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

                frame_yuv420p->pkt_duration = av_rescale_q_rnd(1,
                    mVideoCodecCtx->time_base,
                    mVideoStream->time_base,
                    (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

                frame_yuv420p->pkt_pos = -1;

                t1 = getCurTime();
                ret = avcodec_send_frame(mVideoCodecCtx, frame_yuv420p);
                if (ret >= 0) {
                    ret = avcodec_receive_packet(mVideoCodecCtx, pkt);
                    if (ret >= 0) {
                        t2 = getCurTime();
                        encodeSuccessCount++;

                        //LOGI("encode 1 frame spend：%lld(ms),frameCount=%lld, encodeSuccessCount = %lld, frameQSize=%d,ret=%d", 
                        //    (t2 - t1), frameCount, encodeSuccessCount, frameQSize, ret);
                        pkt->stream_index = mVideoIndex;

                        pkt->pos = -1;
                        pkt->duration = frame_yuv420p->pkt_duration;

                        ret = av_interleaved_write_frame(mFmtCtx, pkt);
                        if (ret < 0) {
                            LOGE("av_interleaved_write_frame error : ret=%d", ret);
                        }

                    }
                    else {
                        LOGE("avcodec_receive_packet error : ret=%d", ret);
                    }

                }
                else {
                    LOGE("avcodec_send_frame error : ret=%d", ret);
                }

                frameCount++;
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        //av_write_trailer(mFmtCtx);//写文件尾

        av_packet_unref(pkt);
        pkt = NULL;


        av_free(frame_yuv420p_buff);
        frame_yuv420p_buff = NULL;

        av_frame_free(&frame_yuv420p);
        //av_frame_unref(frame_yuv420p);
        frame_yuv420p = NULL;
    }
    void AvPushStream::encodeVideoThread(void* arg) {
        AvPushStream* pushStream = (AvPushStream*)arg;
        pushStream->handleEncodeVideo();
    }


    unsigned char AvPushStream::clipValue(unsigned char x, unsigned char min_val, unsigned char  max_val) {

        if (x > max_val) {
            return max_val;
        }
        else if (x < min_val) {
            return min_val;
        }
        else {
            return x;
        }
    }

    bool AvPushStream::bgr24ToYuv420p(unsigned char* bgrBuf, int w, int h, unsigned char* yuvBuf) {

        unsigned char* ptrY, * ptrU, * ptrV, * ptrRGB;
        memset(yuvBuf, 0, w * h * 3 / 2);
        ptrY = yuvBuf;
        ptrU = yuvBuf + w * h;
        ptrV = ptrU + (w * h * 1 / 4);
        unsigned char y, u, v, r, g, b;

        for (int j = 0; j < h; ++j) {

            ptrRGB = bgrBuf + w * j * 3;
            for (int i = 0; i < w; i++) {

                b = *(ptrRGB++);
                g = *(ptrRGB++);
                r = *(ptrRGB++);


                y = (unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
                u = (unsigned char)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
                v = (unsigned char)((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
                *(ptrY++) = clipValue(y, 0, 255);
                if (j % 2 == 0 && i % 2 == 0) {
                    *(ptrU++) = clipValue(u, 0, 255);
                }
                else {
                    if (i % 2 == 0) {
                        *(ptrV++) = clipValue(v, 0, 255);
                    }
                }
            }
        }
        return true;

    }
}


