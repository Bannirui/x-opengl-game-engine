//
// Created by dingrui on 3/1/26.
//

#pragma once

#include "pch.h"

#include <thread>

struct ProfileResult
{
    std::string name;
    long long   start, end;
    uint32_t    threadID;
};

struct InstrumentationSession
{
    std::string name;
};

class Instrumentor
{
private:
    InstrumentationSession *m_currentSession;
    std::ofstream           m_outputStream;
    int                     m_profileCount;

public:
    Instrumentor() : m_currentSession(nullptr), m_profileCount(0) {}

    void BeginSession(const std::string &name, const std::string &filepath = "asset/out/result.json")
    {
        m_outputStream.open(filepath);
        writeHeader();
        m_currentSession = new InstrumentationSession{name};
    }

    void EndSession()
    {
        writeFooter();
        m_outputStream.close();
        delete m_currentSession;
        m_currentSession = nullptr;
        m_profileCount   = 0;
    }

    void WriteProfile(const ProfileResult &result)
    {
        if (m_profileCount++ > 0)
        {
            m_outputStream << ",";
        }
        std::string name = result.name;
        std::replace(name.begin(), name.end(), '"', '\'');

        m_outputStream << "{";
        m_outputStream << "\"cat\":\"function\",";
        m_outputStream << "\"dur\":" << (result.end - result.start) << ',';
        m_outputStream << "\"name\":\"" << name << "\",";
        m_outputStream << "\"ph\":\"X\",";
        m_outputStream << "\"pid\":0,";
        m_outputStream << "\"tid\":" << result.threadID << ",";
        m_outputStream << "\"ts\":" << result.start;
        m_outputStream << "}";
        m_outputStream.flush();
    }

    static Instrumentor &Get()
    {
        static Instrumentor instance;
        return instance;
    }

private:
    void writeHeader()
    {
        m_outputStream << "{\"otherData\": {}, \"traceEvents\":[";
        m_outputStream.flush();
    }
    void writeFooter()
    {
        m_outputStream << "]}";
        m_outputStream.flush();
    }
};

class InstrumentationTimer
{
public:
    InstrumentationTimer(const char *name) : m_name(name)
    {
        m_startTimePoint = std::chrono::high_resolution_clock::now();
    }
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
        auto      endTimepoint = std::chrono::high_resolution_clock::now();
        long long start =
            std::chrono::time_point_cast<std::chrono::microseconds>(m_startTimePoint).time_since_epoch().count();
        long long end =
            std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();
        uint32_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
        Instrumentor::Get().WriteProfile({m_name, start, end, threadID});
        m_stopped = true;
    }

private:
    const char                                                 *m_name;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTimePoint;
    bool                                                        m_stopped;
};

#define X_PROFILE 1

// cross platform, windows or linux
#if defined(_MSC_VER)
#define X_FUNC_SIG __FUNCSIG__
#else
#define X_FUNC_SIG __PRETTY_FUNCTION__
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
