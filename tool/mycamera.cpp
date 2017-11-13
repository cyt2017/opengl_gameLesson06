#include "mycamera.h"

MYCAMERA::MYCAMERA(QObject *parent) : QObject(parent)
{

}

void MYCAMERA::initMycamera(CELL::float3 eye, CELL::float3 look, CELL::float3 up, CELL::float3 right, float moveSpeed)
{
    _eye        = eye;
    _look       = look;
    _up         = up;
    _right      = right;
    _moveSpeed  = moveSpeed;
}

void MYCAMERA::updataMycamera(float fElapsed)
{
    CELL::float3    dir     =   _look - _eye;
    dir =   CELL::normalize(dir);
    switch (_type) {
    case KEY_A:
       {
            _eye    -=  (_right*_moveSpeed) * fElapsed;
            _look   -=  (_right*_moveSpeed) * fElapsed;
       }
        break;
    case KEY_S:
       {
            _eye    +=  (dir*-_moveSpeed) * fElapsed;
            _look   +=  (dir*-_moveSpeed) * fElapsed;
       }
        break;
    case KEY_D:
       {
            _eye    +=  (_right*_moveSpeed) * fElapsed;
            _look   +=  (_right*_moveSpeed) * fElapsed;
       }
        break;
    case KEY_W:
       {
            _eye    -=  dir*-_moveSpeed * fElapsed;
            _look   -=  dir*-_moveSpeed * fElapsed;
       }
        break;
    default:
        break;
    }
//    if(_type!=KEY_NULL)
//    {
//        printf("eye:%.3f,%.3f,%.3f\nlook:%.3f,%.3f,%.3f\n",_eye.x,_eye.y,_eye.z,_look.x,_look.y,_look.z);
//        fflush(NULL);
//    }
    _type=KEY_NULL;

}

void MYCAMERA::reciveKeyEvent(KEYMODE type)
{
    _type = type;
}

