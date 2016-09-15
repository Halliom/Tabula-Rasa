#pragma once

/*
 * Interface for the job class. Subclasses to this are
 * automatically generated for every job and all they do
 * is implement the Execute function where you can put
 * your own code
 */
class IJob
{
public:
    
    IJob() :
    m_bFinished(false)
    {
    }
    
    virtual void Execute() = 0;
    
    bool Finished() const
    {
        return m_bFinished;
    }
    
protected:
    volatile bool	m_bFinished;
};

#define DECLARE_JOB_NOPARAM(JobName, ExecutableCode) \
class JobNoParam##JobName : public IJob \
{ \
public:\
JobNoParam##JobName() { } \
\
virtual void Execute() override \
{ \
ExecutableCode; \
m_bFinished = true; \
} \
};

#define DECLARE_JOB_ONEPARAM(JobName, Param1Type, Param1Name, ExecutableCode) \
class JobOneParam##JobName : public IJob \
{ \
public:\
    JobOneParam##JobName(Param1Type Param1) : Param1Name(Param1) { } \
    \
    virtual void Execute() override \
    { \
        ExecutableCode; \
        m_bFinished = true; \
    } \
private: \
    Param1Type Param1Name; \
};

#define DECLARE_JOB_TWOPARAM(JobName, Param1Type, Param1Name, Param2Type, Param2Name, ExecutableCode) \
class JobTwoParam##JobName : public IJob \
{ \
public:\
JobTwoParam##JobName(Param1Type Param1, Param2Type Param2) : Param1Name(Param1), Param2Name(Param2) { } \
\
virtual void Execute() override \
{ \
ExecutableCode; \
m_bFinished = true; \
} \
private: \
Param1Type Param1Name; \
Param2Type Param2Name; \
};

#define DECLARE_JOB_THREEPARAM(JobName, Param1Type, Param1Name, Param2Type, Param2Name, Param3Type, Param3Name, ExecutableCode) \
class JobThreeParam##JobName : public IJob \
{ \
public:\
JobThreeParam##JobName(Param1Type Param1, Param2Type Param2, Param3Type Param3) : Param1Name(Param1), Param2Name(Param2), Param3Name(Param3) { } \
\
virtual void Execute() override \
{ \
ExecutableCode; \
m_bFinished = true; \
} \
private: \
Param1Type Param1Name; \
Param2Type Param2Name; \
Param3Type Param3Name; \
};

#define DECLARE_JOB_FOURPARAM(JobName, Param1Type, Param1Name, Param2Type, Param2Name, Param3Type, Param3Name, Param4Type, Param4Name, ExecutableCode) \
class JobFourParam##JobName : public IJob \
{ \
public:\
JobFourParam##JobName(Param1Type Param1, Param2Type Param2, Param3Type Param3, Param4Type Param4) : Param1Name(Param1), Param2Name(Param2), Param3Name(Param3), Param4Name(Param4) { } \
\
virtual void Execute() override \
{ \
ExecutableCode; \
m_bFinished = true; \
} \
private: \
Param1Type Param1Name; \
Param2Type Param2Name; \
Param3Type Param3Name; \
Param4Type Param4Name \
};

#define CREATE_JOB_NOPARAM(JobName, ExecutableCode) \
DECLARE_JOB_NOPARAM(JobName, ExecutableCode) \
JobNoParam##JobName* JobName = new JobNoParam##JobName();

#define CREATE_JOB_ONEPARAM(JobName, Param1Type, Param1Name, ExecutableCode) \
DECLARE_JOB_ONEPARAM(JobName, Param1Type, Param1Name, ExecutableCode) \
JobOneParam##JobName* JobName = new JobOneParam##JobName(Param1Name)

#define CREATE_JOB_TWOPARAM(JobName, Param1Type, Param1Name, Param2Type, Param2Name, ExecutableCode) \
DECLARE_JOB_TWOPARAM(JobName, Param1Type, Param1Name, Param2Type, Param2Name, ExecutableCode) \
JobTwoParam##JobName* JobName = new JobTwoParam##JobName(Param1Name, Param2Name)

#define CREATE_JOB_THREEPARAM(JobName, Param1Type, Param1Name, Param2Type, Param2Name, Param3Type, Param3Name, ExecutableCode) \
DECLARE_JOB_THREEPARAM(JobName, Param1Type, Param1Name, Param2Type, Param2Name, Param3Type, Param3Name, ExecutableCode) \
JobThreeParam##JobName* JobName = new JobThreeParam##JobName(Param1Name, Param2Name, Param3Name)

#define CREATE_JOB_FOURPARAM(JobName, Param1Type, Param1Name, Param2Type, Param2Name, Param3Type, Param3Name, Param4Type, Param4Name, ExecutableCode) \
DECLARE_JOB_FOURPARAM(JobName, Param1Type, Param1Name, Param2Type, Param2Name, Param3Type, Param3Name, Param4Type, Param4Name, ExecutableCode) \
JobFourParam##JobName* JobName = new JobFourParam##JobName(Param1Name, Param2Name, Param3Name, Param4Name)