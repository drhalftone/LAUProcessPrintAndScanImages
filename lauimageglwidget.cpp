#include "lauimageglwidget.h"
#include <locale.h>
#include <math.h>

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImageGLWidget::LAUImageGLWidget(LAUMemoryObject obj, QWidget *parent) : QOpenGLWidget(parent), object(obj), texture(nullptr)
{
    options.setAlignment(1);
    setFocusPolicy(Qt::StrongFocus);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImageGLWidget::~LAUImageGLWidget()
{
    if (wasInitialized()) {
        makeCurrent();

        if (texture) {
            delete texture;
        }

        if (vertexArrayObject.isCreated()) {
            vertexArrayObject.release();
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUImageGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.5f, 0.0f, 0.0f, 1.0f);

    // get context opengl-version
    qDebug() << "Widget OpenGl: " << format().majorVersion() << "." << format().minorVersion();
    qDebug() << "Context valid: " << context()->isValid();
    qDebug() << "Really used OpenGl: " << context()->format().majorVersion() << "." << context()->format().minorVersion();
    qDebug() << "OpenGl information: VENDOR:       " << (const char *)glGetString(GL_VENDOR);
    qDebug() << "                    RENDERDER:    " << (const char *)glGetString(GL_RENDERER);
    qDebug() << "                    VERSION:      " << (const char *)glGetString(GL_VERSION);
    qDebug() << "                    GLSL VERSION: " << (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);

    // CREATE THE VERTEX ARRAY OBJECT FOR FEEDING VERTICES TO OUR SHADER PROGRAMS
    vertexArrayObject.create();
    vertexArrayObject.bind();

    // CREATE VERTEX BUFFER TO HOLD CORNERS OF QUADRALATERAL
    quadVertexBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    quadVertexBuffer.create();
    quadVertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (quadVertexBuffer.bind()) {
        // ALLOCATE THE VERTEX BUFFER FOR HOLDING THE FOUR CORNERS OF A RECTANGLE
        quadVertexBuffer.allocate(16 * sizeof(float));
        float *buffer = (float *)quadVertexBuffer.map(QOpenGLBuffer::WriteOnly);
        if (buffer) {
            buffer[0]  = -1.0f;
            buffer[1]  = +1.0f;
            buffer[2]  = +0.0f;
            buffer[3]  = +1.0f;

            buffer[4]  = +1.0f;
            buffer[5]  = +1.0f;
            buffer[6]  = +0.0f;
            buffer[7]  = +1.0f;

            buffer[8]  = +1.0f;
            buffer[9]  = -1.0f;
            buffer[10] = +0.0f;
            buffer[11] = +1.0f;

            buffer[12] = -1.0f;
            buffer[13] = -1.0f;
            buffer[14] = +0.0f;
            buffer[15] = +1.0f;
            quadVertexBuffer.unmap();
        } else {
            qDebug() << QString("quadVertexBuffer not allocated.") << glGetError();
        }
        quadVertexBuffer.release();
    }

    // CREATE INDEX BUFFER TO ORDERINGS OF VERTICES FORMING POLYGON
    quadIndexBuffer = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    quadIndexBuffer.create();
    quadIndexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (quadIndexBuffer.bind()) {
        quadIndexBuffer.allocate(6 * sizeof(unsigned int));
        unsigned int *indices = (unsigned int *)quadIndexBuffer.map(QOpenGLBuffer::WriteOnly);
        if (indices) {
            indices[0] = 0;
            indices[1] = 1;
            indices[2] = 2;
            indices[3] = 0;
            indices[4] = 2;
            indices[5] = 3;
            quadIndexBuffer.unmap();
        } else {
            qDebug() << QString("indiceBufferA buffer mapped from GPU.");
        }
        quadIndexBuffer.release();
    }

    // CREATE TEXTURE FOR DISPLAYING NO VIDEO SCREEN
    texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    texture->setSize((int)object.width(), (int)object.height());
    texture->setFormat(QOpenGLTexture::RGBA32F);
    texture->setWrapMode(QOpenGLTexture::ClampToBorder);
    texture->setMinificationFilter(QOpenGLTexture::Linear);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->allocateStorage();
    if (object.colors() == 1) {
        if (object.depth() == sizeof(unsigned char)) {
            texture->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, (const void *)object.constPointer(), &options);
        } else if (object.depth() == sizeof(unsigned short)) {
            texture->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt16, (const void *)object.constPointer(), &options);
        } else if (object.depth() == sizeof(float)) {
            texture->setData(QOpenGLTexture::Red, QOpenGLTexture::Float32, (const void *)object.constPointer(), &options);
        }
    } else if (object.colors() == 3) {
        if (object.depth() == sizeof(unsigned char)) {
            texture->setData(QOpenGLTexture::RGB, QOpenGLTexture::UInt8, (const void *)object.constPointer(), &options);
        } else if (object.depth() == sizeof(unsigned short)) {
            texture->setData(QOpenGLTexture::RGB, QOpenGLTexture::UInt16, (const void *)object.constPointer(), &options);
        } else if (object.depth() == sizeof(float)) {
            texture->setData(QOpenGLTexture::RGB, QOpenGLTexture::Float32, (const void *)object.constPointer(), &options);
        }
    } else if (object.colors() == 4) {
        if (object.depth() == sizeof(unsigned char)) {
            texture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, (const void *)object.constPointer(), &options);
        } else if (object.depth() == sizeof(unsigned short)) {
            texture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt16, (const void *)object.constPointer(), &options);
        } else if (object.depth() == sizeof(float)) {
            texture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, (const void *)object.constPointer(), &options);
        }
    }

    // CREATE SHADERS
    setlocale(LC_NUMERIC, "C");
    program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Shaders/displayRGBAVideo.vert");
    program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Shaders/displayRGBAVideo.frag");
    setlocale(LC_ALL, "");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUImageGLWidget::resizeGL(int w, int h)
{
    // Get the Desktop Widget so that we can get information about multiple monitors connected to the system.
    QDesktopWidget *dkWidget = QApplication::desktop();
    QList<QScreen *> screenList = QGuiApplication::screens();
    qreal devicePixelRatio = screenList[dkWidget->screenNumber(this)]->devicePixelRatio();
    localHeight = qRound(h * devicePixelRatio);
    localWidth = qRound(w * devicePixelRatio);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUImageGLWidget::paintGL()
{
    double scaleX = (double)object.width() / (double)localWidth;
    double scaleY = (double)object.height() / (double)localHeight;
    double scale = qMax(scaleX, scaleY);

    QTransform transform;
    transform.scale(0.95 * scaleX / scale, -0.95 * scaleY / scale);

    // SET THE VIEW PORT AND CLEAR THE SCREEN BUFFER
    glClearColor(0.5f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, localWidth, localHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (program.bind()) {
        if (quadVertexBuffer.bind()) {
            if (quadIndexBuffer.bind()) {
                // SET THE ACTIVE TEXTURE ON THE GPU
                glActiveTexture(GL_TEXTURE0);
                texture->bind();
                program.setUniformValue("qt_texture", 0);
                program.setUniformValue("qt_transform", QMatrix4x4(transform));

                // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                program.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                program.enableAttributeArray("qt_vertex");

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                quadIndexBuffer.release();
            }
            quadVertexBuffer.release();
        }
        program.release();
    }
}
