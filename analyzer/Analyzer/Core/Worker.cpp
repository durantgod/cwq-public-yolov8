#include "Worker.h"
#include "Utils/Log.h"
#include "Utils/Common.h"
#include "Scheduler.h"
#include "Analyzer.h"
#include "Control.h"
#include "Algorithm.h"
#include "AvPullStream.h"
#include "AvPushStream.h"
#include "GenerateAlarmVideo.h"
#include "Frame.h"

#include <fstream>
#include <iomanip>


extern "C" {
#include "libswscale/swscale.h"
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
}


namespace AVSAnalyzer {

    Worker::Worker(Scheduler* scheduler, Control* control) :
        mScheduler(scheduler),
        mControl(new Control(*control)),
        mPullStream(nullptr),
        mPushStream(nullptr),
        mAnalyzer(nullptr),
        mState(false)
    {

        mControl->startTimestamp = getCurTimestamp();

        LOGI("");
    }

    Worker::~Worker()
    {
        LOGI("");

        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        mState = false;// 将执行状态设置为false

        for (auto th : mThreads) {
            th->join();
        }

        for (auto th : mThreads) {
            delete th;
            th = nullptr;
        }
        mThreads.clear();
        clearAlarmVideoFrameQ();

        if (mPullStream) {
            delete mPullStream;
            mPullStream = nullptr;
        }
        if (mPushStream) {
            delete mPushStream;
            mPushStream = nullptr;
        }

        if (mAnalyzer) {
            delete mAnalyzer;
            mAnalyzer = nullptr;
        }

        if (mControl) {
            delete mControl;
            mControl = nullptr;
        }
        //最有一步释放mFramePool
        if (mVideoFramePool) {
            delete mVideoFramePool;
            mVideoFramePool = nullptr;
        }

    }
    bool Worker::start(std::string& msg) {

        this->mPullStream = new AvPullStream(this);
        if (this->mPullStream->connect()) {
            if (mControl->pushStream) {
                this->mPushStream = new AvPushStream(this);
                if (this->mPushStream->connect()) {
                    // success
                }
                else {
                    msg = "pull stream connect success, push stream connect error";
                    return false;
                }
            }
            else {
                // success
            }
        }
        else {
            msg = "pull stream connect error";
            return false;
        }

        int videoBgrSize = mControl->videoHeight * mControl->videoWidth * mControl->videoChannel;
        this->mVideoFramePool = new FramePool(videoBgrSize);
        this->mAnalyzer = new Analyzer(mScheduler, mControl);

        mState = true;// 将执行状态设置为true


        std::thread* th = new std::thread(AvPullStream::readThread, this->mPullStream);
        mThreads.push_back(th);

        th = new std::thread(Worker::decodeVideoThread, this);
        mThreads.push_back(th);

        th = new std::thread(Worker::generateAlarmThread, this);
        mThreads.push_back(th);


        if (mControl->pushStream) {
            if (mControl->videoIndex > -1) {
                th = new std::thread(AvPushStream::encodeVideoThread, this->mPushStream);
                mThreads.push_back(th);
            }
        }

        for (auto th : mThreads) {
            th->native_handle();
        }


        return true;
    }


    bool Worker::getState() {
        return mState;
    }
    void Worker::remove() {
        mState = false;
        mScheduler->removeWorker(mControl);
    }
    void Worker::generateAlarmThread(void* arg) {
        Worker* worker = (Worker*)arg;
        worker->handleGenerateAlarm();
    }
    void Worker::handleGenerateAlarm() {

        int width = mControl->videoWidth;
        int height = mControl->videoHeight;
        int channels = mControl->videoChannel;

        Frame* videoFrame = nullptr; // 未编码的视频帧（bgr格式）

        std::queue<Frame* > cacheV;
        int prefix_size = 60; //75 = 25 * 3，事件发生前缓存3秒的数据，1张压缩图片100kb

        bool happening = false;// 当前是否正在发生报警行为
        std::queue<Frame* > happenV;
        int  total_size = 120;//报警总帧数最大长度

        int64_t last_alarm_timestamp = 0;// 上一次报警的时间戳

        int64_t t1, t2 = 0;

        while (getState())
        {
            if (getAlarmVideoFrame(videoFrame)) {

                if (happening && cacheV.empty()) {// 报警事件已经发生，正在进行中
                    
                    happenV.push(videoFrame);

                    if (happenV.size() >= total_size) {
                        last_alarm_timestamp = getCurTimestamp();

                        //产生报警视频
                        Alarm* alarm = new Alarm(
                            height,
                            width,
                            mControl->videoFps,
                            last_alarm_timestamp,prefix_size,
                            mControl->code.data()
                        );
     
                        while (!happenV.empty())
                        {
                            Frame * p = happenV.front();
                            happenV.pop();
                            alarm->frames.push_back(p);
                        }
                    
                        mScheduler->addAlarm(alarm);
                        happening = false;
                    }

                }
                else {// 暂未发生报警事件
          
                 
                    if (!cacheV.empty() && cacheV.size() >= prefix_size) {
                        //缓存帧超过容量上限 prefix_size
                        Frame* head = cacheV.front();
                        cacheV.pop();
                        mVideoFramePool->giveBack(head);
                    }
                    cacheV.push(videoFrame);


                    if (videoFrame->happen &&
                        cacheV.size() >= prefix_size &&
                        (getCurTimestamp() - last_alarm_timestamp) > mControl->minInterval) {
                        //满足报警开始条件
                        happening = true;
                        
                        while (!cacheV.empty())
                        {
                            Frame * p = cacheV.front();
                            cacheV.pop();
                            happenV.push(p);
                        }        
                    }
                }

            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }

        }

        while (!happenV.empty())
        {
            Frame * p = happenV.front();
            happenV.pop();
            delete p;
            p = nullptr;
            
        }
        while (!cacheV.empty())
        {
            Frame* p = cacheV.front();
            cacheV.pop();
            delete p;
            p = nullptr;

        }


    }
    void Worker::decodeVideoThread(void* arg) {
        Worker* worker = (Worker*)arg;
        worker->handleDecodeVideo();
    }

    void updateSharedCount(int count) {
        std::ofstream file("E:/new/BXC_VideoAnalyzer_v3.41/Analyzer/count_data.txt");
        if (file.is_open()) {
            file << std::setprecision(4) << count;
            file.close();
        }
    }





    void Worker::handleDecodeVideo() {
   
        int width = mPullStream->mVideoCodecCtx->width;
        int height = mPullStream->mVideoCodecCtx->height;

        AVPacket pkt; // 未解码的视频帧
        int      pktQSize = 0; // 未解码视频帧队列当前长度

        AVFrame* frame_yuv420p = av_frame_alloc();// pkt->解码->frame
        AVFrame* frame_bgr = av_frame_alloc();

        int frame_bgr_buff_size = av_image_get_buffer_size(AV_PIX_FMT_BGR24, width, height, 1);
        uint8_t* frame_bgr_buff = (uint8_t*)av_malloc(frame_bgr_buff_size);
        av_image_fill_arrays(frame_bgr->data, frame_bgr->linesize, frame_bgr_buff, AV_PIX_FMT_BGR24, width, height, 1);

        SwsContext* sws_ctx_yuv420p2bgr = sws_getContext(width, height,
            mPullStream->mVideoCodecCtx->pix_fmt,
            mPullStream->mVideoCodecCtx->width,
            mPullStream->mVideoCodecCtx->height,
            AV_PIX_FMT_BGR24,
            SWS_BICUBIC, nullptr, nullptr, nullptr);

        int fps = mControl->videoFps;

        //算法检测参数start
        bool cur_is_check = false;// 当前帧是否进行算法检测
        int  continuity_check_count = 0;// 当前连续进行算法检测的帧数
        int  continuity_check_max_time = 6000;//连续进行算法检测，允许最长的时间。单位毫秒
        int64_t continuity_check_start = getCurTime();//单位毫秒
        int64_t continuity_check_end = 0;
        //算法检测参数end

        int ret = -1;
        int64_t frameCount = 0;
        bool happen = false;
        float happenScore = 0.0;
        std::vector<DetectObject> happenDetects;



        while (getState())
        {
            if (mPullStream->getVideoPkt(pkt, pktQSize)) {

                if (mControl->videoIndex > -1) {

                    ret = avcodec_send_packet(mPullStream->mVideoCodecCtx, &pkt);
                    if (ret == 0) {
                        ret = avcodec_receive_frame(mPullStream->mVideoCodecCtx, frame_yuv420p);

                        if (ret == 0) {
                            frameCount++;

                            // frame（yuv420p） 转 frame_bgr
                            sws_scale(sws_ctx_yuv420p2bgr,
                                frame_yuv420p->data, frame_yuv420p->linesize, 0, height,
                                frame_bgr->data, frame_bgr->linesize);

                            cv::Mat image(mControl->videoHeight, mControl->videoWidth, CV_8UC3, frame_bgr->data[0]);


                            if (pktQSize == 0) {
                                cur_is_check = mAnalyzer->handleVideoFrame(frameCount, image, happenDetects, happen, happenScore);
                                if (cur_is_check) {
                                    continuity_check_count += 1;
                                }
                            }
                            else {
                                cur_is_check = false;
                            }

                            continuity_check_end = getCurTime();
                            if (continuity_check_end - continuity_check_start > continuity_check_max_time) {
                                mControl->checkFps = float(continuity_check_count) / (float(continuity_check_end - continuity_check_start) / 1000);
                                continuity_check_count = 0;
                                continuity_check_start = getCurTime();
                            }
                            //设置动态字体
                            float baseFontSize = 2.0;//字号大小
                            float baseFontSize_count = 2.0;//左上角标签字号大小
                            float baseFat = 2.0;//字体粗细
                            float fontSize = baseFontSize * (image.cols / 1920.0);
                            if (fontSize < 0.5) {  //320*180
                                fontSize = 0.2;
                                baseFontSize_count = 0.4;
                                baseFat = 1;
                            }
                            else if (0.5 < fontSize < 1) {  //640*320
                                fontSize = 0.5;
                                baseFontSize_count = 0.3;
                                baseFat = 1;
                            }
                            //动态字体位置
                            int textOffsetY = 30; // 基准偏移量  
                            int adjustedOffsetY = textOffsetY * (image.rows / 1080.0); // 根据高度比例调整偏移量  

                            //绘制start
                            cv::polylines(image, mControl->recognitionRegion_points, mControl->recognitionRegion_points.size(), cv::Scalar(0, 0, 255), baseFat, 8);//绘制多边形
                            if (happenDetects.size() > 0) {
                                int x1, y1, x2, y2;
                                for (size_t i = 0; i < happenDetects.size(); i++)
                                {
                                    x1 = happenDetects[i].x1;
                                    y1 = happenDetects[i].y1;
                                    x2 = happenDetects[i].x2;
                                    y2 = happenDetects[i].y2;

                                    std::string class_name = happenDetects[i].class_name;
                                    float       class_score = happenDetects[i].score;

                                    std::stringstream class_score_ss;
                                    class_score_ss << std::setprecision(1) << class_score;
                                    std::string title = class_name + ":" + class_score_ss.str();

                                    cv::rectangle(image, cv::Rect(x1, y1, (x2 - x1), (y2 - y1)), cv::Scalar(0, 0, 255), baseFat, cv::LINE_8, 0);
                                    
                                    cv::rectangle(image, cv::Point(x1, y1 - adjustedOffsetY), cv::Point(x2, y1), cv::Scalar(0, 255, 255), -1);
                                    cv::putText(image, title, cv::Point(x1, y1), cv::FONT_HERSHEY_SIMPLEX, fontSize, cv::Scalar(255, 0, 0), baseFat, 8);
                                    //FONT_HERSHEY_COMPLEX
                                }
                            }
                            //cv::putText(image, mControl->algorithmCode, cv::Point(20 * (image.cols / 1920.0), 80 * (image.rows / 1080.0)), cv::FONT_HERSHEY_COMPLEX, fontSize, cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
                            //std::stringstream fps_stream;
                            //fps_stream << std::setprecision(4) << mControl->checkFps;
                            //std::string fps_title = "checkfps:" + fps_stream.str();
                            //cv::putText(image, fps_title, cv::Point(20 * (image.cols / 1920.0), 140 * (image.rows / 1080.0)), cv::FONT_HERSHEY_COMPLEX, fontSize, cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
                            std::stringstream count_stream;
                            count_stream << std::setprecision(4) << mControl->count;
                            std::string count_title = "Count:" + count_stream.str();
                            cv::putText(image, count_title, cv::Point(20 * (image.cols / 1920.0), 150 * (image.rows / 1280.0)), cv::FONT_HERSHEY_SIMPLEX, baseFontSize_count, cv::Scalar(0, 0, 255), baseFat, cv::LINE_AA);
                            //绘制end


                            // 更新共享数据文件
                            updateSharedCount(mControl->count);
                            mControl->count = 0;
     

                            if (mControl->pushStream) {//需要推算法实时流
                                int size = mPushStream->getVideoFrameQSize();
                                if (size < 3) {
                                    Frame* frame = mVideoFramePool->gain();
                                    frame->setBuf(frame_bgr->data[0], frame_bgr_buff_size);
                                    frame->happen = happen;
                                    frame->happenScore = happenScore;
                                    mPushStream->addVideoFrame(frame);
                                }
    
                            }
                            
                            //添加合成报警视频帧start
                            int size = getAlarmVideoFrameQSize();
                            if (size < 3) {
                                Frame* frame = mVideoFramePool->gain();
                                frame->setBuf(frame_bgr->data[0], frame_bgr_buff_size);
                                frame->happen = happen;
                                frame->happenScore = happenScore;
                                this->addAlarmVideoFrameQ(frame);
                            }

                            //添加合成报警视频帧end
                            
                        
                        }
                        else {
                            LOGE("avcodec_receive_frame error : ret=%d", ret);
                        }
                    }
                    else {
                        LOGE("avcodec_send_packet error : ret=%d", ret);
                    }
                }

                // 队列获取的pkt，必须释放!!!
                //av_free_packet(&pkt);//过时
                av_packet_unref(&pkt);
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }


        av_frame_free(&frame_yuv420p);
        //av_frame_unref(frame_yuv420p);
        frame_yuv420p = NULL;

        av_frame_free(&frame_bgr);
        //av_frame_unref(frame_bgr);
        frame_bgr = NULL;


        av_free(frame_bgr_buff);
        frame_bgr_buff = NULL;


        sws_freeContext(sws_ctx_yuv420p2bgr);
        sws_ctx_yuv420p2bgr = NULL;

    }


    void Worker::addAlarmVideoFrameQ(Frame* frame) {

        mAlarmVideoFrameQ_mtx.lock();
        mAlarmVideoFrameQ.push(frame);
        mAlarmVideoFrameQ_mtx.unlock();

    }
    int Worker::getAlarmVideoFrameQSize() {
        int size = 0;
        mAlarmVideoFrameQ_mtx.lock();
        size = mAlarmVideoFrameQ.size();
        mAlarmVideoFrameQ_mtx.unlock();
        return size;
    }
    bool Worker::getAlarmVideoFrame(Frame*& frame) {
        bool res = false;
        if (mAlarmVideoFrameQ_mtx.try_lock()) {
            if (!mAlarmVideoFrameQ.empty()) {
                frame = mAlarmVideoFrameQ.front();
                mAlarmVideoFrameQ.pop();
                res = true;
            }
            mAlarmVideoFrameQ_mtx.unlock();
        }
        return res;
    }
    void Worker::clearAlarmVideoFrameQ() {

        mAlarmVideoFrameQ_mtx.lock();
        while (!mAlarmVideoFrameQ.empty())
        {
            Frame* frame = mAlarmVideoFrameQ.front();
            mAlarmVideoFrameQ.pop();
            mVideoFramePool->giveBack(frame);
        }
        mAlarmVideoFrameQ_mtx.unlock();

    }

}