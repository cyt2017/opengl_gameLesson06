#ifndef PROGRAMID_H
#define PROGRAMID_H

#include <assert.h>
#include <GLES2/gl2.h>
class    ShaderId
{
public:
    ShaderId()
    {
        _shaderId   =   -1;
    }
    int _shaderId;
};

class ProgramId
{
public:
    int         _programId;
    ShaderId    _vertex;
    ShaderId    _fragment;
public:
    ProgramId();
    bool createProgram( const char* vertex,const char* fragment );
    /**
    *   使用程序
    */
    virtual void    begin();
    /**
    *   使用完成
    */
    virtual void    end();
};

#endif // PROGRAMID_H
