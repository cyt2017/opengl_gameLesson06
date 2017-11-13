#include "qgleswidget.h"

QGLESWIDGET::QGLESWIDGET(QWidget *parent) : QWidget(parent)
{

}

QGLESWIDGET::~QGLESWIDGET()
{
    destroyOpenGLES20();
}

bool QGLESWIDGET::init_QGW(std::vector<QString> fileName)
{
    if(!initOpenGLES20())
    {
        return false;
    }
    _shader.initialize();
//    QString name = fileName[0];
    _texture = loadTextureMipmap(fileName);
//    name = fileName[1];
//    _texture1 = loadTexture(name.toLatin1().data());
    _texture1 = createTexture(_width,_height);

    //!初始化摄像机的数据
    CELL::float3 eye   =   CELL::float3(1, 1, 1);
    CELL::float3 look  =   CELL::float3(0.5f, -0.4f, -5.5f);
    CELL::float3 up    =   CELL::float3(0.0f, 1.0f, 0.0f);
    CELL::float3 right =   CELL::float3(1.0f, 0.0f, 0.0f);
    float moveSpeed    =   5.0f;

    _camera.initMycamera(eye,look,up,right,moveSpeed);
    connect(this,SIGNAL(sendKeyEvent(KEYMODE)),&_camera,SLOT(reciveKeyEvent(KEYMODE)));
}

bool QGLESWIDGET::initOpenGLES20()
{
    const EGLint attribs[] =
    {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE,24,
        EGL_NONE
    };
    EGLint 	format(0);
    EGLint	numConfigs(0);
    EGLint  major;
    EGLint  minor;

    //! 1
    _display	    =	eglGetDisplay(EGL_DEFAULT_DISPLAY);

    //! 2init
    eglInitialize(_display, &major, &minor);

    //! 3
    eglChooseConfig(_display, attribs, &_config, 1, &numConfigs);

    eglGetConfigAttrib(_display, _config, EGL_NATIVE_VISUAL_ID, &format);
    //!!! 4 使opengl与qt的窗口进行绑定<this->winId()>
    _surface	    = 	eglCreateWindowSurface(_display, _config, this->winId(), NULL);

    //! 5
    EGLint attr[]   =   { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };
    _context 	    = 	eglCreateContext(_display, _config, 0, attr);
    //! 6
    if (eglMakeCurrent(_display, _surface, _surface, _context) == EGL_FALSE)
    {
        return false;
    }

    eglQuerySurface(_display, _surface, EGL_WIDTH,  &_width);
    eglQuerySurface(_display, _surface, EGL_HEIGHT, &_height);

    return  true;
}

void QGLESWIDGET::destroyOpenGLES20()
{
    glDeleteTextures(1,&_texture);
    if (_display != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (_context != EGL_NO_CONTEXT)
        {
            eglDestroyContext(_display, _context);
        }
        if (_surface != EGL_NO_SURFACE)
        {
            eglDestroySurface(_display, _surface);
        }
        eglTerminate(_display);
    }
    _display    =   EGL_NO_DISPLAY;
    _context    =   EGL_NO_CONTEXT;
    _surface    =   EGL_NO_SURFACE;//asdsafsaf
}

void QGLESWIDGET::render()
{
    //! 清空缓冲区
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    //! 视口，在Windows窗口指定的位置和大小上绘制OpenGL内容
    glViewport(0,0,_width,_height);

    _camera.updataMycamera(0.016f);
    CELL::matrix4   matView =   CELL::lookAt(_camera._eye,_camera._look,_camera._up);


    CELL::matrix4   matWorld(1);
    CELL::matrix4   matProj =   CELL::perspective(45.0f, (GLfloat)_width / (GLfloat)_height, 0.1f, 100.0f);
    CELL::matrix4   MVP     =   matProj * matView * matWorld;

    _shader.begin();
    {
        //!绘制一个地面
        displayGround(MVP);
        //!绘制一个旋转的矩形
        displayTexMatRot(matView);
        //!在屏幕中的一个区域将拷贝到的屏幕图像显示出来
        displayTexSub2D();
    }
    _shader.end();
}

unsigned int QGLESWIDGET::loadTexture(const char *fileName)
{
    unsigned int textureId   =   0;
    //1 获取图片格式
    FREE_IMAGE_FORMAT fifmt = FreeImage_GetFileType(fileName, 0);

    //2 加载图片
    FIBITMAP    *dib = FreeImage_Load(fifmt, fileName,0);

    //3 转化为rgb 24色
    dib     =   FreeImage_ConvertTo24Bits(dib);

    //4 获取数据指针
    BYTE    *pixels =   (BYTE*)FreeImage_GetBits(dib);

    int     width   =   FreeImage_GetWidth(dib);
    int     height  =   FreeImage_GetHeight(dib);

    for (size_t i = 0 ;i < width * height * 3 ; i+=3 )
    {
        BYTE temp       =   pixels[i];
        pixels[i]       =   pixels[i + 2];
        pixels[i + 2]   =   temp;
    }
    /**
        *   产生一个纹理Id,可以认为是纹理句柄，后面的操作将书用这个纹理id
        */
    glGenTextures( 1, &textureId );

    /**
        *   使用这个纹理id,或者叫绑定(关联)
        */
    glBindTexture( GL_TEXTURE_2D, textureId );
    /**
        *   指定纹理的放大,缩小滤波，使用线性方式，即当图片放大的时候插值方式
        */
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    /**
        *   将图片的rgb数据上传给opengl.
        */
    glTexImage2D(
                GL_TEXTURE_2D,      //! 指定是二维图片
                0,                  //! 指定为第一级别，纹理可以做mipmap,即lod,离近的就采用级别大的，远则使用较小的纹理
                GL_RGB,             //! 纹理的使用的存储格式
                width,              //! 宽度，老一点的显卡，不支持不规则的纹理，即宽度和高度不是2^n。
                height,             //! 宽度，老一点的显卡，不支持不规则的纹理，即宽度和高度不是2^n。
                0,                  //! 是否存在边
                GL_RGB,             //! 数据的格式，bmp中，windows,操作系统中存储的数据是bgr格式
                GL_UNSIGNED_BYTE,   //! 数据是8bit数据
                pixels
                );
    /**
        *   释放内存
        */
    FreeImage_Unload(dib);

    return  textureId;
}

unsigned int QGLESWIDGET::loadTextureMipmap(std::vector<QString> fName)
{
    //!函数实现功能：把多个文理尺寸放在同一个文理中，
    //!实现各种显示出来的效果（有文理分明的过度和渐变过度，主要是调整<glTexParameteri>函数中的最后一个参数来实现）????????????
    unsigned int textureId = 0;
    glGenTextures(1,&textureId);
    glBindTexture( GL_TEXTURE_2D, textureId );
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    int fileCount = fName.size();
    printf("file number:%d\n",fileCount);
    fflush(NULL);
    for(int j=0;j<fileCount;++j)
    {
        QString fileName = fName[j];
        FREE_IMAGE_FORMAT fifmt = FreeImage_GetFileType(fileName.toLatin1().data(), 0);

        //2 加载图片
        FIBITMAP    *dib = FreeImage_Load(fifmt,fileName.toLatin1().data(),0);

        //3 转化为rgb 24色
        dib     =   FreeImage_ConvertTo24Bits(dib);

        //4 获取数据指针
        BYTE    *pixels =   (BYTE*)FreeImage_GetBits(dib);

        int     width   =   FreeImage_GetWidth(dib);
        int     height  =   FreeImage_GetHeight(dib);

        for (size_t i = 0 ;i < width * height * 3 ; i+=3 )
        {
            BYTE temp       =   pixels[i];
            pixels[i]       =   pixels[i + 2];
            pixels[i + 2]   =   temp;
        }
        glTexImage2D(
                    GL_TEXTURE_2D,      //! 指定是二维图片
                    j,                  //! 指定为第一级别，纹理可以做mipmap,即lod,离近的就采用级别大的，远则使用较小的纹理
                    GL_RGB,             //! 纹理的使用的存储格式
                    width,              //! 宽度，老一点的显卡，不支持不规则的纹理，即宽度和高度不是2^n。
                    height,             //! 宽度，老一点的显卡，不支持不规则的纹理，即宽度和高度不是2^n。
                    0,                  //! 是否存在边
                    GL_RGB,             //! 数据的格式，bmp中，windows,操作系统中存储的数据是bgr格式
                    GL_UNSIGNED_BYTE,   //! 数据是8bit数据
                    pixels
                    );
        /**
            *   释放内存
            */
        FreeImage_Unload(dib);
    }

    return textureId;
}

unsigned int QGLESWIDGET::createTexture(int width, int height)
{
    //!函数功能：产生一个新的空白文理，这个文理产生时是黑色的
    unsigned int textureId;
    glGenTextures(1,&textureId);//!生成文理
    glBindTexture(GL_TEXTURE_2D,textureId);//绑定文理
    /**
     *以下函数（）说明：这是一个设置文理参数的函数，同时也是起到对文理的包装
     * 对文理的放大、缩小滤波起到多层文理的过度作用
     * 主要说明最后一个参数：
     * 当第二个参数是<GL_TEXTURE_MAG_FILTER>时，
     * 参数3：GL_LINEAR
     * 参数3：GL_NEAREST
     * 当第二个参数是<GL_TEXTURE_MIN_FILTER>时，
     * 参数3：GL_LINEAR
     * 参数3：GL_NEAREST
     * 参数3：GL_LINEAR_MIPMAP_LINEAR 针对多层文理可用
     * 参数3：GL_NEAREST_MIPMAP_LINEAR 针对多层文理可用
     * 参数3：GL_LINEAR_MIPMAP_NEAREST 针对多层文理可用
     * 参数3：GL_NEAREST_MIPMAP_NEAREST 针对多层文理可用
     */
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);//设置文理的相关参数1、文理的放大滤波
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);//设置文理的相关参数2、文理的缩小滤波
    /**
     *针对文理的包装:
     * 参数2是<GL_TEXTURE_WRAP_S>时，表示水平文理，
     *       <GL_TEXTURE_WRAP_T>时，表示垂直文理
     * 参数3：GL_REPEAT        表示文理根据所给文理重复
     * 参数3：GL_CLAMP_TO_EDGE 表示文理根据所给文理的最后一个像素重复
     * 参数3：GL_REPEAT        表示文理根据所给文理镜像重复
     */
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);//设置文理的相关参数3、水平文理的重复格式
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);//设置文理的相关参数4、垂直文理的重复格式
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,0);
    return textureId;
}


void QGLESWIDGET::drawImage()
{
    render();
    eglSwapBuffers(_display,_surface);
}

void QGLESWIDGET::displayGround(CELL::matrix4 matMVP)
{
    CELL::matrix4 MVP = matMVP;
    float   gSize   =   100;
    float   gPos    =   -5;
    float   rept    =   100;

    Vertex grounds[] =
    {
         -gSize, gPos,-gSize,  0.0f, 0.0f,     1.0f, 1.0f, 1.0f,1.0f ,
          gSize, gPos,-gSize,  rept, 0.0f,     1.0f, 1.0f, 1.0f,1.0f ,
          gSize, gPos, gSize,  rept, rept,     1.0f, 1.0f, 1.0f,1.0f ,

         -gSize, gPos,-gSize,  0.0f, 0.0f,     1.0f, 1.0f, 1.0f,1.0f ,
          gSize, gPos, gSize,  rept, rept,     1.0f, 1.0f, 1.0f,1.0f ,
         -gSize, gPos, gSize,  0.0f, rept,     1.0f, 1.0f, 1.0f,1.0f ,
    };
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,_texture);

    glUniform1i(_shader._texture,0);

    glUniformMatrix4fv(_shader._MVP, 1, false, MVP.data());
    glVertexAttribPointer(_shader._position,3,GL_FLOAT,false,sizeof(Vertex),&(grounds[0].x));
    glVertexAttribPointer(_shader._uv,2,GL_FLOAT,false,sizeof(Vertex),&(grounds[0].u));
    /**
     * 当数据类型改变时，一定要注意
     * 当颜色数据是BYTE时，需要归一化，opengl接受的是浮点型数据，
     * 但是当颜色数据是float时，就不用归一化，统一规定到（0~1）之间
     */
    glVertexAttribPointer(_shader._color,4,GL_FLOAT,false,sizeof(Vertex),&(grounds[0].r));
    glDrawArrays(GL_TRIANGLES,0,6);
}

void QGLESWIDGET::displayTexMatRot(CELL::matrix4 matView)
{
    Vertex vertexs[] =
    {
       { -1.0f,-1.0f, 1.0f,0.0f, 0.0f,1.0f, 1.0f, 1.0f,1.0f },
       {  1.0f,-1.0f, 1.0f,1.0f, 0.0f,1.0f, 1.0f, 1.0f,1.0f },
       {  1.0f, 1.0f, 1.0f,1.0f, 1.0f,1.0f, 1.0f, 1.0f,1.0f },

       { -1.0f,-1.0f, 1.0f,0.0f, 0.0f,1.0f, 1.0f, 1.0f,1.0f },
       {  1.0f, 1.0f, 1.0f,1.0f, 1.0f,1.0f, 1.0f, 1.0f,1.0f },
       { -1.0f, 1.0f, 1.0f,0.0f, 1.0f,1.0f, 1.0f, 1.0f,1.0f },
    };
    CELL::matrix4   matProj =   CELL::perspective(45.0f, (GLfloat)_width / (GLfloat)_height, 0.1f, 100.0f);
    //在原有屏幕中添加一个旋转的矩形，只需要改变矩阵，与顶点相关参数
    CELL::matrix4 matRot;
    static float angle = 0;
    matRot.rotateZ(angle);
    angle += 1;
    CELL::matrix4   MVP1     =   matProj * matView * matRot;

    glUniformMatrix4fv(_shader._MVP, 1, false, MVP1.data());
    glVertexAttribPointer(_shader._position,3,GL_FLOAT,false,sizeof(Vertex),&(vertexs[0].x));
    glVertexAttribPointer(_shader._uv,2,GL_FLOAT,false,sizeof(Vertex),&(vertexs[0].u));
    glVertexAttribPointer(_shader._color,4,GL_FLOAT,false,sizeof(Vertex),&(vertexs[0].r));
    glDrawArrays(GL_TRIANGLE_STRIP,0,6);
}

void QGLESWIDGET::displayTexSub2D()
{
    float   x   =   0;
    float   y   =   0;
    float   w   =   200;
    float   h   =   200;
    Vertex  vertex[]   =
    {
        {x,      y,         0,  0,1,    1.0f, 1.0f, 1.0f,1.0f},
        {x + w,  y,         0,  1,1,    1.0f, 1.0f, 1.0f,1.0f},
        {x,      y + h,     0,  0,0,    1.0f, 1.0f, 1.0f,1.0f},
        {x + w,  y + h,     0,  1,0,    1.0f, 1.0f, 1.0f,1.0f},
    };
    //在屏幕上选择一个区域显示二维图像，这个二维图像是从屏幕中截取出来的
    CELL::matrix4   screenProj  =   CELL::ortho<float>(0,float(_width),float(_height),0,-100.0f,100);

    //!以下函数中的文理是直接创造的空白文理，
    //! 如果不添加<glCopyTexSubImage2D>拷贝函数，那么呈现出来的效果图就是全黑画面
    glBindTexture(GL_TEXTURE_2D,_texture1);


    /**
     * 以下函数(glCopyTexSubImage2D)是opengl中的拷贝函数，拷贝屏幕中的某些部分图像
     * 参数1、使用(GL_TEXTURE_2D)即可
     * 参数2、拷贝到哪一级文理中（一般显示在屏幕中由多级文理图像组成）
     * 参数3,4、屏幕的相对位置（距离<0,0>点的偏移量）
     * 参数5~8、文理的大小（坐标）
     */
    glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,_width,_height);

    glUniformMatrix4fv(_shader._MVP, 1, false, screenProj.data());
    glVertexAttribPointer(_shader._position,3,GL_FLOAT,false,sizeof(Vertex),&(vertex[0].x));
    glVertexAttribPointer(_shader._uv,2,GL_FLOAT,false,sizeof(Vertex),&(vertex[0].u));
    glVertexAttribPointer(_shader._color,4,GL_FLOAT,false,sizeof(Vertex),&(vertex[0].r));
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}

bool QGLESWIDGET::eventFilter(QObject *target, QEvent *event)
{
    if( target == parent )
     {
         if( event->type() == QEvent::KeyPress )
         {
             QKeyEvent *ke = (QKeyEvent *) event;
             keyPressEvent(ke);
          }
     }
    return true;
}

void QGLESWIDGET::keyPressEvent(QKeyEvent *e)
{
//    printf("keyPressEvent\n");
//    fflush(NULL);
    KEYMODE type = KEY_NULL;
    if(e->key()==Qt::Key_A)
    {

        type = KEY_A;
    }
    else if(e->key()==Qt::Key_S)
    {
        type = KEY_S;
    }
    else if(e->key()==Qt::Key_D)
    {
        type = KEY_D;
    }
    else if(e->key()==Qt::Key_W)
    {
        type = KEY_W;
    }
    emit sendKeyEvent(type);
}


