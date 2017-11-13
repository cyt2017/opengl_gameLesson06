#include "program_p2_c4.h"

PROGRAM_P2_C4::PROGRAM_P2_C4()
{
    _position   =   -1;
    _color      =   -1;
    _uv         =   -1;
    _MVP        =   -1;
    _texture    =   -1;
}

bool PROGRAM_P2_C4::initialize()
{
    //!使用（varying），是表示需要在（vs）和（ps）之间进行传递的变量，把需要传递的变量在（vs）中赋值，并在（ps）中重复使用（varying）语句即可
    const char* vs  =
    {
//        "precision lowp         float; "
        "uniform   mat4         _MVP;"
        "attribute vec3         _position;"
        "attribute vec2         _uv;"
        "attribute vec4         _color;"
        "varying   vec4         _outColor;"
        "varying   vec2         _outUV;"

        "void main()"
        "{"
        "   vec4    pos =   vec4(_position.x,_position.y,_position.z,1);"
        "   _outColor   =   _color;"
        "   _outUV      =   _uv;"
        "   gl_Position =   _MVP * pos;"
        "}"
    };
    const char* ps  =
    {
        "precision  lowp float; "
        "uniform    sampler2D   _texture;"
        "varying    vec4        _outColor;"
        "varying    vec2        _outUV;"
        "void main()"
        "{"
        "   vec4 tcolor0    =   texture2D(_texture,_outUV);"
        "   gl_FragColor    =   tcolor0 * _outColor;"
        "}"
    };
    //!在上面的（"gl_FragColor=tcolor0 + tcolor1;"）中，用（+）图像变亮；用（*）图像变暗。
    bool    res =   createProgram(vs,ps);
    if(res)
    {
        //!~需要注意变量的类型
        _position   =   glGetAttribLocation(_programId,"_position");
        _color      =   glGetAttribLocation(_programId,"_color");
        _uv         =   glGetAttribLocation(_programId,"_uv");
        _MVP        =   glGetUniformLocation(_programId,"_MVP");
        _texture    =   glGetUniformLocation(_programId,"_texture");
    }
    return  res;
}

void PROGRAM_P2_C4::begin()
{
    glUseProgram(_programId);
    //!在显卡里面使用的局部变量，在使用时是需要进行使能和关闭的
    glEnableVertexAttribArray(_position);       
    glEnableVertexAttribArray(_color);
    glEnableVertexAttribArray(_uv);
}

void PROGRAM_P2_C4::end()
{
    glDisableVertexAttribArray(_position);
    glDisableVertexAttribArray(_color);
    glDisableVertexAttribArray(_uv);
    glUseProgram(0);
}
