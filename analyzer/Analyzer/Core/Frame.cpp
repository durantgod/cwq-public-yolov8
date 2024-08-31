#include "Frame.h"
#include "Utils/Log.h"
#include "Utils/Common.h"
#include <memory.h>

namespace AVSAnalyzer {
	Frame::Frame(int bufInitSize) {
        //LOGI("");
		this->mBufInitSize = bufInitSize;
		this->mBuf = new uint8_t[this->mBufInitSize];
	}
	Frame::~Frame() {
        //LOGI("");
		delete[]this->mBuf;
		this->mBuf = nullptr;
	}
    void Frame::setBuf(unsigned char* buf, int size) {

        if (this->mBufInitSize == size) {
            this->mBufSize = size;
            memcpy(this->mBuf, buf, size);
        }
        else {
            LOGE("Frame::setBuf size=%d over max", size);
            this->mBufSize = -1;
        }

    }
    unsigned char* Frame::getBuf() {
        return this->mBuf;
    }
    int Frame::getSize() {
        return this->mBufSize;
    }
    FramePool::FramePool(int size) :mSize(size)
    {
        LOGI("");

    }
    FramePool::~FramePool()
    {
        LOGI("");
        clearFrameQ();
    }

    void FramePool::clearFrameQ() {
        mFrameQ_mtx.lock();
        while (!mFrameQ.empty())
        {
            Frame* frame = mFrameQ.front();
            mFrameQ.pop();

            delete frame;
            frame = nullptr;
        }
        mFrameQ_mtx.unlock();
    }
    Frame* FramePool::gain() {
        Frame* frame = nullptr;

        if (mFrameQ_mtx.try_lock()) {
            //LOGI("FramePool::gain() mFrameQ.size()=%lld", mFrameQ.size());

            if (!mFrameQ.empty()) {

                while (mFrameQ.size() > 5)
                {
                    frame = mFrameQ.front();
                    mFrameQ.pop();
                    delete frame;
                    frame = nullptr;
                }

                frame = mFrameQ.front();
                mFrameQ.pop();
            }

            mFrameQ_mtx.unlock();
        }

        if(!frame){
            frame = new Frame(mSize);
        }

        return frame;
    }
    void FramePool::giveBack(Frame* frame) {
        mFrameQ_mtx.lock();
        mFrameQ.push(frame);
        mFrameQ_mtx.unlock();

    }
}
