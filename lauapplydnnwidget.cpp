#include "lauapplydnnwidget.h"

#include <QScreen>
#include <QFileInfo>
#include <QSettings>
#include <QApplication>
#include <QDesktopWidget>
#include <QStandardPaths>
#include <QGuiApplication>

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUApplyDNNGLWidget::LAUApplyDNNGLWidget(LAUMemoryObject obj, QWidget *parent) : QOpenGLWidget(parent), object(obj), objectTexture(nullptr), frameBufferObjectA(nullptr), frameBufferObjectB(nullptr), frameBufferObjectC(nullptr), frameBufferObjectD(nullptr)
{
    options.setAlignment(1);
    setFocusPolicy(Qt::StrongFocus);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUApplyDNNGLWidget::~LAUApplyDNNGLWidget()
{
    if (wasInitialized()) {
        makeCurrent();

        while (filterTextures.count() > 0) {
            delete filterTextures.takeFirst();
        }
        if (objectTexture) {
            delete objectTexture;
        }
        if (frameBufferObjectA) {
            delete frameBufferObjectA;
        }
        if (frameBufferObjectB) {
            delete frameBufferObjectB;
        }
        if (frameBufferObjectC) {
            delete frameBufferObjectC;
        }
        if (frameBufferObjectD) {
            delete frameBufferObjectD;
        }

        if (vertexArrayObject.isCreated()) {
            vertexArrayObject.release();
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUApplyDNNGLWidget::updateBuffer(LAUMemoryObject obj)
{
    if (obj.isValid()) {
        // KEEP A LOCAL COPY OF THE INCOMING OBJECT
        object = obj;

        // MAKE THIS THE CURRENT OPENGL CONTEXT
        makeCurrent();

        // UPLOAD THE INCOMING OBJECT TO THE GPU
        if (object.colors() == 1) {
            if (object.depth() == sizeof(unsigned char)) {
                objectTexture->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, (const void *)object.constPointer(), &options);
            } else if (object.depth() == sizeof(unsigned short)) {
                objectTexture->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt16, (const void *)object.constPointer(), &options);
            } else if (object.depth() == sizeof(float)) {
                objectTexture->setData(QOpenGLTexture::Red, QOpenGLTexture::Float32, (const void *)object.constPointer(), &options);
            }
        } else if (object.colors() == 3) {
            if (object.depth() == sizeof(unsigned char)) {
                objectTexture->setData(QOpenGLTexture::RGB, QOpenGLTexture::UInt8, (const void *)object.constPointer(), &options);
            } else if (object.depth() == sizeof(unsigned short)) {
                objectTexture->setData(QOpenGLTexture::RGB, QOpenGLTexture::UInt16, (const void *)object.constPointer(), &options);
            } else if (object.depth() == sizeof(float)) {
                objectTexture->setData(QOpenGLTexture::RGB, QOpenGLTexture::Float32, (const void *)object.constPointer(), &options);
            }
        } else if (object.colors() == 4) {
            if (object.depth() == sizeof(unsigned char)) {
                objectTexture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, (const void *)object.constPointer(), &options);
            } else if (object.depth() == sizeof(unsigned short)) {
                objectTexture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt16, (const void *)object.constPointer(), &options);
            } else if (object.depth() == sizeof(float)) {
                objectTexture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, (const void *)object.constPointer(), &options);
            }
        }

        // MAKE SURE WE HAVE OUR FRAME BUFFER OBJECTS ALLOCATED ON THE GPU
        if (frameBufferObjectA && frameBufferObjectB && frameBufferObjectC) {
            // BIND FRAMEBUFFER A TO HOLD THE FIRST LEVEL WAVELET DECOMPOSITION
            if (frameBufferObjectA->bind()) {
                // CLEAR THE FRAME BUFFER OBJECT
                glViewport(0, 0, frameBufferObjectA->width() / 2, frameBufferObjectA->height());
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                if (programB.bind()) {
                    // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
                    if (quadVertexBuffer.bind()) {
                        if (quadIndexBuffer.bind()) {
                            // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                            glActiveTexture(GL_TEXTURE0);
                            objectTexture->bind();
                            programB.setUniformValue("qt_texture", 0);
                            programB.setUniformValue("qt_rowStep", 1);
                            programB.setUniformValue("qt_width", objectTexture->width());

                            // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                            programB.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                            programB.enableAttributeArray("qt_vertex");

                            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                            // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                            quadIndexBuffer.release();
                        }
                        quadVertexBuffer.release();
                    }
                    programB.release();
                }
                frameBufferObjectA->release();
            }

            // BIND FRAMEBUFFER B TO HOLD THE SECOND LEVEL WAVELET DECOMPOSITION
            if (frameBufferObjectB->bind()) {
                // CLEAR THE FRAME BUFFER OBJECT
                glViewport(0, 0, frameBufferObjectB->width() / 4, frameBufferObjectB->height());
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                if (programB.bind()) {
                    // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
                    if (quadVertexBuffer.bind()) {
                        if (quadIndexBuffer.bind()) {
                            // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                            glActiveTexture(GL_TEXTURE0);
                            glBindTexture(GL_TEXTURE_2D, frameBufferObjectA->texture());
                            programB.setUniformValue("qt_texture", 0);
                            programB.setUniformValue("qt_rowStep", 2);
                            programB.setUniformValue("qt_width", frameBufferObjectA->width() / 2);

                            // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                            programB.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                            programB.enableAttributeArray("qt_vertex");

                            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                            // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                            quadIndexBuffer.release();
                        }
                        quadVertexBuffer.release();
                    }
                    programB.release();
                }
                frameBufferObjectB->release();
            }

            // BIND FRAMEBUFFER C TO HOLD THE THIRD LEVEL WAVELET DECOMPOSITION
            if (frameBufferObjectC->bind()) {
                // CLEAR THE FRAME BUFFER OBJECT
                glViewport(0, 0, frameBufferObjectC->width() / 8, frameBufferObjectC->height());
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                if (programB.bind()) {
                    // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
                    if (quadVertexBuffer.bind()) {
                        if (quadIndexBuffer.bind()) {
                            // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                            glActiveTexture(GL_TEXTURE0);
                            glBindTexture(GL_TEXTURE_2D, frameBufferObjectB->texture());
                            programB.setUniformValue("qt_texture", 0);
                            programB.setUniformValue("qt_rowStep", 2);
                            programB.setUniformValue("qt_width", frameBufferObjectB->width() / 4);

                            // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                            programB.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                            programB.enableAttributeArray("qt_vertex");

                            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                            // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                            quadIndexBuffer.release();
                        }
                        quadVertexBuffer.release();
                    }
                    programB.release();
                }
                frameBufferObjectC->release();
            }

            // BIND FRAMEBUFFER D TO HOLD THE RECONSTITUTED FIRST LEVEL WAVELET DECOMPOSITION
            if (frameBufferObjectD->bind()) {
                // CLEAR THE FRAME BUFFER OBJECT
                glViewport(0, 0, frameBufferObjectD->width() / 4, frameBufferObjectD->height() / 2);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                if (programC.bind()) {
                    // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
                    if (quadVertexBuffer.bind()) {
                        if (quadIndexBuffer.bind()) {
                            // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                            glActiveTexture(GL_TEXTURE0);
                            glBindTexture(GL_TEXTURE_2D, frameBufferObjectC->texture());
                            programC.setUniformValue("qt_texture", 0);
                            programC.setUniformValue("qt_width", frameBufferObjectD->width() / 4);

                            // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                            programC.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                            programC.enableAttributeArray("qt_vertex");

                            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                            // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                            quadIndexBuffer.release();
                        }
                        quadVertexBuffer.release();
                    }
                    programC.release();
                }
                frameBufferObjectD->release();
            }

            LAUMemoryObject obj(frameBufferObjectB->width(), frameBufferObjectB->height(), 4, sizeof(float));
            glBindTexture(GL_TEXTURE_2D, frameBufferObjectA->texture());
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, obj.constPointer());
            obj.save(QString("/tmp/objectA.tif"));

            glBindTexture(GL_TEXTURE_2D, frameBufferObjectB->texture());
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, obj.constPointer());
            obj.save(QString("/tmp/objectB.tif"));

            glBindTexture(GL_TEXTURE_2D, frameBufferObjectC->texture());
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, obj.constPointer());
            obj.save(QString("/tmp/objectC.tif"));

            glBindTexture(GL_TEXTURE_2D, frameBufferObjectD->texture());
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, obj.constPointer());
            obj.save(QString("/tmp/objectD.tif"));
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUApplyDNNGLWidget::initializeGL()
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
    objectTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    objectTexture->setSize((int)object.width(), (int)object.height());
    objectTexture->setFormat(QOpenGLTexture::RGBA32F);
    objectTexture->setWrapMode(QOpenGLTexture::ClampToBorder);
    objectTexture->setMinificationFilter(QOpenGLTexture::Linear);
    objectTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    objectTexture->allocateStorage();

    // CREATE A FORMAT OBJECT FOR CREATING THE FRAME BUFFER
    QOpenGLFramebufferObjectFormat frameBufferObjectFormat;
    frameBufferObjectFormat.setTextureTarget(GL_TEXTURE_2D);
    frameBufferObjectFormat.setInternalTextureFormat(GL_RGBA32F);

    // CREATE NEW FRAME BUFFER OBJECTS
    frameBufferObjectA = new QOpenGLFramebufferObject((int)object.width(), 2 * (int)object.height(), frameBufferObjectFormat);
    frameBufferObjectA->release();
    frameBufferObjectB = new QOpenGLFramebufferObject((int)object.width(), 2 * (int)object.height(), frameBufferObjectFormat);
    frameBufferObjectB->release();
    frameBufferObjectC = new QOpenGLFramebufferObject((int)object.width(), 2 * (int)object.height(), frameBufferObjectFormat);
    frameBufferObjectC->release();
    frameBufferObjectD = new QOpenGLFramebufferObject((int)object.width(), 2 * (int)object.height(), frameBufferObjectFormat);
    frameBufferObjectD->release();

    // CREATE SHADERS
    setlocale(LC_NUMERIC, "C");
    if (object.colors() == 1) {
        programA.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Shaders/displayGRAYVideo.vert");
        programA.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Shaders/displayGRAYVideo.frag");
        programA.link();
    } else {
        programA.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Shaders/displayRGBAVideo.vert");
        programA.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Shaders/displayRGBAVideo.frag");
        programA.link();
    }

    programB.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Shaders/DNN/waveletDecompFilter.vert");
    programB.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Shaders/DNN/waveletDecompFilter.frag");
    programB.link();

    programC.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Shaders/DNN/waveletReconstFilter.vert");
    programC.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Shaders/DNN/waveletReconstFilter.frag");
    programC.link();

    setlocale(LC_ALL, "");

    // RUN THE FILTER ON THE THE PRIVATE MEMORY OBJECT
    updateBuffer(object);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUApplyDNNGLWidget::resizeGL(int w, int h)
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
void LAUApplyDNNGLWidget::paintGL()
{
    // SET THE VIEW PORT AND CLEAR THE SCREEN BUFFER
    glClearColor(0.5f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, localWidth, localHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (frameBufferObjectD) {
        if (programA.bind()) {
            if (quadVertexBuffer.bind()) {
                if (quadIndexBuffer.bind()) {
                    // SET THE ACTIVE TEXTURE ON THE GPU
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, frameBufferObjectD->texture());
                    programA.setUniformValue("qt_texture", 0);

                    // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                    programA.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                    programA.enableAttributeArray("qt_vertex");

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                    quadIndexBuffer.release();
                }
                quadVertexBuffer.release();
            }
            programA.release();
        }
    } else if (objectTexture) {
        if (programA.bind()) {
            if (quadVertexBuffer.bind()) {
                if (quadIndexBuffer.bind()) {
                    // SET THE ACTIVE TEXTURE ON THE GPU
                    glActiveTexture(GL_TEXTURE0);
                    objectTexture->bind();
                    programA.setUniformValue("qt_texture", 0);

                    // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                    programA.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                    programA.enableAttributeArray("qt_vertex");

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                    quadIndexBuffer.release();
                }
                quadVertexBuffer.release();
            }
            programA.release();
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObject LAUApplyDNNGLWidget::result()
{
    return (object);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUApplyDNNWidget::LAUApplyDNNWidget(LAUMemoryObject obj, QWidget *parent) : QWidget(parent), object(obj), widget(nullptr)
{
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(0, 0, 0, 0);

    if (object.isNull()) {
        QSettings settings;
        QString directory = settings.value("LAUApplyDNNWidget::directory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
        if (QDir().exists(directory) == false) {
            directory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        }
        QString fileString = QFileDialog::getOpenFileName(this, QString("Select scan from disk..."), directory, QString("*.tif;*.tiff"));
        if (fileString.isEmpty() == false) {
            settings.setValue("LAUApplyDNNWidget::directory", QFileInfo(fileString).absolutePath());
        } else {
            return;
        }
        object = LAUMemoryObject(fileString);
    }

    if (object.isValid()) {
        widget = new LAUApplyDNNGLWidget(object);
        this->layout()->addWidget(widget);
        this->setMinimumSize(480, 640);
    }
}
