#ifndef MYCAMERA_H
#define MYCAMERA_H

#include <QObject>
#include <tool/CELLMath.hpp>

class MYCAMERA : public QObject
{
    Q_OBJECT
public:
    explicit MYCAMERA(QObject *parent = 0);
    enum KEYMODE
    {
        KEY_NULL,
        KEY_A,
        KEY_S,
        KEY_D,
        KEY_W,
    };


    CELL::float3    _eye;
    CELL::float3    _look;
    CELL::float3    _up;
    CELL::float3    _right;
    float           _moveSpeed;


    void initMycamera(CELL::float3 eye,CELL::float3 look,CELL::float3 up,CELL::float3 right,float moveSpeed);

    void updataMycamera(float fElapsed);

//!鼠标事件相关
    KEYMODE _type;

public slots:
    void reciveKeyEvent(KEYMODE type);
};

#endif // MYCAMERA_H
