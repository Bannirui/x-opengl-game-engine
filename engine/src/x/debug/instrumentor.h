//
// Created by dingrui on 3/1/26.
//

#pragma once

#include "pch.h"

using FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;

struct ProfileResult
{
    std::string               name;
    FloatingPointMicroseconds start;
    std::chrono::microseconds elapsedTime;
    std::thread::id           threadID;
};

struct InstrumentationSession
{
    std::string name;
};

class Instrumentor
{
public:
    Instrumentor(const Instrumentor&) = delete;
    Instrumentor(Instrumentor&&)      = delete;

    void BeginSession(const std::string& name, const std::string& filepath = "asset/out/result.json")
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_currentSession)
        {
            if (XLog::get_coreLogger())
            {  // Edge case: BeginSession() might be before Log::Init()
                X_CORE_ERROR("Instrumentor::BeginSession('{0}') when session '{1}' already open.", name,
                             m_currentSession->name);
            }
            internalEndSession();
        }

        m_outputStream.open(filepath);
        if (m_outputStream.is_open())
        {
            m_currentSession = new InstrumentationSession({name});
            writeHeader();
        }
        else
        {
            if (XLog::get_coreLogger())
            {
                // Edge case: BeginSession() might be before Log::Init()
                X_CORE_ERROR("Instrumentor could not open results file '{0}'.", filepath);
            }
        }
    }

    void EndSession()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        internalEndSession();
    }

    void WriteProfile(const ProfileResult& result)
    {
        std::stringstream json;
        json << std::setprecision(3) << std::fixed;
        json << ",{";
        json << "\"cat\":\"function\",";
        json << "\"dur\":" << (result.elapsedTime.count()) << ',';
        json << "\"name\":\"" << result.name << "\",";
        json << "\"ph\":\"X\",";
        json << "\"pid\":0,";
        json << "\"tid\":" << result.threadID << ",";
        json << "\"ts\":" << result.start.count();
        json << "}";
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_currentSession)
        {
            m_outputStream << json.str();
            m_outputStream.flush();
        }
    }

    static Instrumentor& Get()
    {
        static Instrumentor instance;
        return instance;
    }

private:
    Instrumentor() : m_currentSession(nullptr) {}
    ~Instrumentor() { EndSession(); }

    void writeHeader()
    {
        m_outputStream << "{\"otherData\": {}, \"traceEvents\":[{}";
        m_outputStream.flush();
    }
    void writeFooter()
    {
        m_outputStream << "]}";
        m_outputStream.flush();
    }
    void internalEndSession()
    {
        if (m_currentSession)
        {
            writeFooter();
            m_outputStream.close();
            delete m_currentSession;
            m_currentSession = nullptr;
        }
    }

private:
    std::mutex              m_mutex;
    InstrumentationSession* m_currentSession;
    std::ofstream           m_outputStream;
};

class InstrumentationTimer
{
public:
    InstrumentationTimer(const char* name) : m_name(name) { m_startTimePoint = std::chrono::steady_clock::now(); }
    ~InstrumentationTimer()
    {
        if (!m_stopped)
        {
            stop();
        }
    }

private:
    void stop()
    {
        auto endTimePoint = std::chrono::steady_clock::now();
        auto highResStart = FloatingPointMicroseconds{m_startTimePoint.time_since_epoch()};
        auto elapsedTime  = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch() -
                           std::chrono::time_point_cast<std::chrono::microseconds>(m_startTimePoint).time_since_epoch();
        Instrumentor::Get().WriteProfile({m_name, highResStart, elapsedTime, std::this_thread::get_id()});
        m_stopped = true;
    }

private:
    const char*                                        m_name;
    std::chrono::time_point<std::chrono::steady_clock> m_startTimePoint;
    bool                                               m_stopped{false};
};

#define X_PROFILE 1

// 跨平台兼容
#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || \
    defined(__ghs__)
#define X_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
#define X_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__) || (_MSC_VERION)
#define X_FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define X_FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#define X_FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define X_FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
#define X_FUNC_SIG __func__
#else
#define X_FUNC_SIG "X_FUNC_SIG unknown!"
#endif

#if X_PROFILE
#define X_PROFILE_BEGIN_SESSION(name, filepath) ::Instrumentor::Get().BeginSession(name, filepath)
#define X_PROFILE_END_SESSION() ::Instrumentor::Get().EndSession()
#define X_PROFILE_SCOPE(name) ::InstrumentationTimer timer##__LINE__(name);
#define X_PROFILE_FUNCTION() X_PROFILE_SCOPE(X_FUNC_SIG)
#else
#define X_PROFILE_BEGIN_SESSION(name, filepath)
#define X_PROFILE_END_SESSION()
#define X_PROFILE_SCOPE(name)
#define X_PROFILE_FUNCTION()
#endif
