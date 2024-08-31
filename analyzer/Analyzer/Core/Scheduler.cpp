#include "Scheduler.h"
#include "Config.h"
#include "Control.h"
#include "Worker.h"
#include "Algorithm.h"
#include "AlgorithmOnnxRuntime.h"
#include "GenerateAlarmVideo.h"
#include "Utils/Log.h"

namespace AVSAnalyzer {
    Scheduler::Scheduler(Config* config) :mConfig(config), mState(false),
        mLoopAlarmThread(nullptr)
    {
        LOGI("");

    }

    Scheduler::~Scheduler()
    {
        LOGI("");
        if (mAlgorithm) {
            delete mAlgorithm;
            mAlgorithm = nullptr;
        }
        clearAlarmQueue();
        mLoopAlarmThread->join();
        delete mLoopAlarmThread;
        mLoopAlarmThread = nullptr;
    }

    Config* Scheduler::getConfig() {
        return mConfig;
    }
    bool Scheduler::initAlgorithm() {
        LOGI("开始初始化算法模型...");

        bool ret = false;
        mAlgorithm = new AlgorithmOnnxRuntime(mConfig);
        //orthAlgorithm = new AlgorithmOnnxRuntime(mConfig);//其他

        ret = true;
        LOGI("初始化算法模型成功");
        return ret;
    }
    Algorithm* Scheduler::gainAlgorithm() {
        Algorithm* algorithm = nullptr;

        if (mAlgorithm_mtx.try_lock()) {
            if (mAlgorithm) {
                algorithm = mAlgorithm;
                mAlgorithm = nullptr;
            }
            mAlgorithm_mtx.unlock();
        }


        return algorithm;
    
    }
    void  Scheduler::giveBackAlgorithm(Algorithm* algorithm) {
        mAlgorithm_mtx.lock();
        mAlgorithm = algorithm;
        mAlgorithm_mtx.unlock();
    }
    void Scheduler::loop() {


        mLoopAlarmThread = new std::thread(Scheduler::loopAlarmThread, this);
        mLoopAlarmThread->native_handle();
        LOGI("Start Success");
        int64_t l = 0;
        while (mState)
        {
            ++l;
            handleDeleteWorker();
        
        }

    }

    int Scheduler::apiControls(std::vector<Control*>& controls) {
        int len = 0;

        mWorkerMapMtx.lock();
        for (auto f = mWorkerMap.begin(); f != mWorkerMap.end(); ++f)
        {
            ++len;
            controls.push_back(f->second->mControl);

        }
        mWorkerMapMtx.unlock();

        return len;
    }
    Control* Scheduler::apiControl(std::string& code) {
        Control* control = nullptr;
        mWorkerMapMtx.lock();
        for (auto f = mWorkerMap.begin(); f != mWorkerMap.end(); ++f)
        {
            if (f->first == code) {
                control = f->second->mControl;
            }

        }
        mWorkerMapMtx.unlock();

        return control;
    }


    void Scheduler::apiControlAdd(Control* control, int& result_code, std::string& result_msg) {

        if (isAdd(control)) {
            result_msg = "the control is running";
            result_code = 1000;
            return;
        }

        else {
            Worker* worker = new Worker(this, control);

            if (worker->start(result_msg)) {
                if (addWorker(control, worker)) {
                    result_msg = "add success";
                    result_code = 1000;
                }
                else {
                    delete worker;
                    worker = nullptr;
                    result_msg = "add error";
                    result_code = 0;
                }
            }
            else {
                delete worker;
                worker = nullptr;
                result_code = 0;
            }
        }

    }
    void Scheduler::apiControlCancel(Control* control, int& result_code, std::string& result_msg) {

        Worker* worker = getWorker(control);

        if (worker) {
            if (worker->getState()) {
                result_msg = "control is running, ";
            }
            else {
                result_msg = "control is not running, ";
            }

            removeWorker(control);

            result_msg += "cancel success";
            result_code = 1000;
            return;

        }
        else {
            result_msg = "there is no such control";
            result_code = 0;
            return;
        }

    }
    void Scheduler::setState(bool state) {
        mState = state;
    }
    bool Scheduler::getState() {
        return mState;
    }

    int Scheduler::getWorkerSize() {
        mWorkerMapMtx.lock();
        int size = mWorkerMap.size();
        mWorkerMapMtx.unlock();

        return size;
    }
    bool Scheduler::isAdd(Control* control) {

        mWorkerMapMtx.lock();
        bool isAdd = mWorkerMap.end() != mWorkerMap.find(control->code);
        mWorkerMapMtx.unlock();

        return isAdd;
    }
    bool Scheduler::addWorker(Control* control, Worker* worker) {
        bool add = false;

        mWorkerMapMtx.lock();
        bool isAdd = mWorkerMap.end() != mWorkerMap.find(control->code);
        if (!isAdd) {
            mWorkerMap.insert(std::pair<std::string, Worker* >(control->code, worker));
            add = true;
        }
        mWorkerMapMtx.unlock();
        return add;
    }
    bool Scheduler::removeWorker(Control* control) {
        bool result = false;

        mWorkerMapMtx.lock();
        auto f = mWorkerMap.find(control->code);
        if (mWorkerMap.end() != f) {
            Worker* worker = f->second;

            // 添加到待删除队列start
            std::unique_lock <std::mutex> lck(mTobeDeletedWorkerQ_mtx);
            mTobeDeletedWorkerQ.push(worker);
            //mTobeDeletedWorkerQ_cv.notify_all();
            mTobeDeletedWorkerQ_cv.notify_one();
            // 添加到待删除队列end

            result = mWorkerMap.erase(control->code) != 0;
        }
        mWorkerMapMtx.unlock();
        return result;
    }
    Worker* Scheduler::getWorker(Control* control) {
        Worker* worker = nullptr;

        mWorkerMapMtx.lock();
        auto f = mWorkerMap.find(control->code);
        if (mWorkerMap.end() != f) {
            worker = f->second;
        }
        mWorkerMapMtx.unlock();
        return worker;
    }

    void Scheduler::handleDeleteWorker() {

        std::unique_lock <std::mutex> lck(mTobeDeletedWorkerQ_mtx);
        mTobeDeletedWorkerQ_cv.wait(lck);

        while (!mTobeDeletedWorkerQ.empty()) {
            Worker* worker = mTobeDeletedWorkerQ.front();
            mTobeDeletedWorkerQ.pop();

            LOGI("code=%s,streamUrl=%s", worker->mControl->code.data(), worker->mControl->streamUrl.data());


            delete worker;
            worker = nullptr;
        }

    }
    void Scheduler::handleLoopAlarm() {

        int alarmQSize;

        while (true) {

            Alarm* alarm = nullptr;
            if (getAlarm(alarm, alarmQSize)) {

                GenerateAlarmVideo gen(mConfig,alarm);
                gen.genAlarmVideo();

                delete alarm;
                alarm = nullptr;
            }

        }


    
    }
    void Scheduler::loopAlarmThread(void* arg) {
        Scheduler* scheduler = (Scheduler*)arg;
        scheduler->handleLoopAlarm();
    }
    void Scheduler::addAlarm(Alarm* alarm) {
        mAlarmQ_mtx.lock();
        if (mAlarmQ.size() > 0) {
            //扔掉
            delete alarm;
            alarm = nullptr;
        }
        else {
            mAlarmQ.push(alarm);
        }
   
        mAlarmQ_mtx.unlock();
    }

    bool Scheduler::getAlarm(Alarm*& alarm, int& alarmQSize) {
        mAlarmQ_mtx.lock();

        if (!mAlarmQ.empty()) {
            alarm = mAlarmQ.front();
            mAlarmQ.pop();
            alarmQSize = mAlarmQ.size();
            mAlarmQ_mtx.unlock();
            return true;

        }
        else {
            alarmQSize = 0;
            mAlarmQ_mtx.unlock();
            return false;
        }
    }
    void Scheduler::clearAlarmQueue() {}

}
