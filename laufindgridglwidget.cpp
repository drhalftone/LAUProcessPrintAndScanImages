#include "laufindgridglwidget.h"
#include <QInputDialog>
#include <locale.h>
#include <math.h>

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
double logFunction(double lambda)
{
    return ((lambda > 0.0) ? lambda * log(1.0 / lambda) : 0.0);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUFindGridGLWidget::LAUFindGridGLWidget(LAUMemoryObject obj, QWidget *parent) : QOpenGLWidget(parent), object(obj), frameBufferObjectC(nullptr), bestAngle(0.0)
{
    options.setAlignment(1);
    setFocusPolicy(Qt::StrongFocus);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUFindGridGLWidget::~LAUFindGridGLWidget()
{
    if (wasInitialized()) {
        makeCurrent();

        if (frameBufferObjectC) {
            delete frameBufferObjectC;
        }

        if (vertexArrayObject.isCreated()) {
            vertexArrayObject.release();
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFindGridGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.5f, 0.0f, 0.0f, 1.0f);

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

    // CREATE SHADERS
    setlocale(LC_NUMERIC, "C");
    programA.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/Shaders/Shaders/filterFitGridX.vert");
    programA.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Shaders/filterFitGridX.frag");

    programB.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/Shaders/Shaders/filterFitGridY.vert");
    programB.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Shaders/filterFitGridY.frag");

    programC.addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/Shaders/Shaders/filterRotateImage.vert");
    programC.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Shaders/filterRotateImage.frag");

    programD.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Shaders/displayRGBAVideo.vert");
    programD.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Shaders/displayRGBAVideo.frag");
    setlocale(LC_ALL, "");

    // PROCESS THE CURRENT OBJECT
    process(object);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFindGridGLWidget::process(LAUMemoryObject obj)
{
    if (obj.isValid()) {
        // SAVE A LOCAL COPY OF THE INCOMING OBJECT
        object = obj;

        // SEE IF WE NEED TO CROP THE INCOMING MEMORY OBJECT
        int targetWidth = qFloor(printedWidth * printedResolution);
        int targetHeight = qFloor(printedHeight * printedResolution);
        if (object.width() != targetWidth || object.height() != targetHeight) {
            object = object.crop(targetWidth, targetHeight);
        }

        // MAKE THIS THE CURRENT OPENGL CONTEXT
        makeCurrent();

        // CREATE A BUFFER TO HOLD THE ROW AND COLUMN COORDINATES OF IMAGE PIXELS FOR THE TEXEL FETCHES
        QOpenGLBuffer pixlVertexBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        pixlVertexBuffer.create();
        pixlVertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
        if (pixlVertexBuffer.bind()) {
            pixlVertexBuffer.allocate((int)object.width() * (int)object.height() * 2 * sizeof(float));
            float *vertices = (float *)pixlVertexBuffer.map(QOpenGLBuffer::WriteOnly);
            if (vertices) {
                int index = 0;
                for (unsigned int row = 0; row < object.height(); row++) {
                    for (unsigned int col = 0; col < object.width(); col++) {
                        vertices[index++] = (float)col;
                        vertices[index++] = (float)row;
                    }
                }
                pixlVertexBuffer.unmap();
            } else {
                qDebug() << QString("Unable to map vertexBuffer from GPU.");
            }
        }

        // CREATE AN INDEX BUFFER FOR THE RESULTING POINT CLOUD DRAWN AS TRIANGLES
        QOpenGLBuffer pixlIndexBuffer = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
        pixlIndexBuffer.create();
        pixlIndexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
        if (pixlIndexBuffer.bind()) {
            pixlIndexBuffer.allocate((int)object.width() * (int)object.height() * sizeof(unsigned int));
            unsigned int *indices = (unsigned int *)pixlIndexBuffer.map(QOpenGLBuffer::WriteOnly);
            if (indices) {
                for (unsigned int ind = 0; ind < (object.width() * object.height()); ind++) {
                    indices[ind] = ind;
                }
                pixlIndexBuffer.unmap();
            } else {
                qDebug() << QString("Unable to map indiceBuffer from GPU.");
            }
        }

        // CREATE TEXTURE FOR DISPLAYING NO VIDEO SCREEN
        QOpenGLTexture texture(QOpenGLTexture::Target2D);
        texture.setSize((int)object.width(), (int)object.height());
        texture.setFormat(QOpenGLTexture::RGBA32F);
        texture.setWrapMode(QOpenGLTexture::ClampToBorder);
        texture.setMinificationFilter(QOpenGLTexture::Linear);
        texture.setMagnificationFilter(QOpenGLTexture::Linear);
        texture.allocateStorage();

        // UPLOAD THE NEW OBJECT TO THE GPU TEXTURE
        if (object.colors() == 1) {
            if (object.depth() == sizeof(unsigned char)) {
                texture.setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, (const void *)object.constPointer(), &options);
            } else if (object.depth() == sizeof(unsigned short)) {
                texture.setData(QOpenGLTexture::Red, QOpenGLTexture::UInt16, (const void *)object.constPointer(), &options);
            } else if (object.depth() == sizeof(float)) {
                texture.setData(QOpenGLTexture::Red, QOpenGLTexture::Float32, (const void *)object.constPointer(), &options);
            }
        } else if (object.colors() == 3) {
            if (object.depth() == sizeof(unsigned char)) {
                texture.setData(QOpenGLTexture::RGB, QOpenGLTexture::UInt8, (const void *)object.constPointer(), &options);
            } else if (object.depth() == sizeof(unsigned short)) {
                texture.setData(QOpenGLTexture::RGB, QOpenGLTexture::UInt16, (const void *)object.constPointer(), &options);
            } else if (object.depth() == sizeof(float)) {
                texture.setData(QOpenGLTexture::RGB, QOpenGLTexture::Float32, (const void *)object.constPointer(), &options);
            }
        } else if (object.colors() == 4) {
            if (object.depth() == sizeof(unsigned char)) {
                texture.setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, (const void *)object.constPointer(), &options);
            } else if (object.depth() == sizeof(unsigned short)) {
                texture.setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt16, (const void *)object.constPointer(), &options);
            } else if (object.depth() == sizeof(float)) {
                texture.setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, (const void *)object.constPointer(), &options);
            }
        }

        // CREATE A FORMAT OBJECT FOR CREATING THE FRAME BUFFER
        QOpenGLFramebufferObjectFormat frameBufferObjectFormat;
        frameBufferObjectFormat.setTextureTarget(GL_TEXTURE_2D);
        frameBufferObjectFormat.setInternalTextureFormat(GL_RGBA32F);

        // CREATE NEW FRAME BUFFER OBJECTS
        QOpenGLFramebufferObject frameBufferObjectA((int)object.width(), 1, frameBufferObjectFormat);
        frameBufferObjectA.release();

        QOpenGLFramebufferObject frameBufferObjectB(1, (int)object.height(), frameBufferObjectFormat);
        frameBufferObjectB.release();

        if (frameBufferObjectC) {
            delete frameBufferObjectC;
        }
        frameBufferObjectC = new QOpenGLFramebufferObject((int)object.width(), (int)object.height(), frameBufferObjectFormat);
        frameBufferObjectC->release();

        // TURN ON ALPHA BLENDING TO ADD POINTS TOGETHER IN FINAL FBO
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glPointSize(1.0f);

        // CREATE A LAUMEMORYOBJECT AND DOWNLOAD FBO TO IT AND SAVE TO DISK
        LAUMemoryObject obj(frameBufferObjectA.width(), frameBufferObjectA.height(), 4, sizeof(float));
        LAUMemoryObject bestObjX(frameBufferObjectA.width(), frameBufferObjectA.height(), 4, sizeof(float));

        // KEEP A LIST OF VARIABLES WITH THE BEST ANGLE
        double bestEntrp = 1e6;
        bestAngle = 0.0;

        // INITIALIZE THE ANGLE STEP SIZE
        double delta = 1.0;

        // ITERATE THROUGH FIVE STEP SIZES
        for (int dlt = 0; dlt < 4; dlt++) {
            // ITERATE THROUGH SEARCH SPACE
            for (int row = 0; row < 20; row++) {
                // DERIVE ANGLE FOR THE CURRENT ITERATION
                double angle = (double)(row - 10) * delta + bestAngle;

                // BIND THE FRAME BUFFER OBJECT FOR PROCESSING THE PHASE DFT COEFFICIENTS
                // ALONG WITH THE GLSL PROGRAMS THAT WILL DO THE PROCESSING
                if (frameBufferObjectA.bind()) {
                    if (programA.bind()) {
                        // CLEAR THE FRAME BUFFER OBJECT
                        glViewport(0, 0, frameBufferObjectA.width(), frameBufferObjectA.height());
                        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                        // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
                        if (pixlVertexBuffer.bind()) {
                            if (pixlIndexBuffer.bind()) {
                                // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                                glActiveTexture(GL_TEXTURE0);
                                texture.bind();
                                programA.setUniformValue("qt_texture", 0);

                                // SET THE MAXIMUM INTENSITY VALUE FOR THE INCOMING VIDEO
                                programA.setUniformValue("qt_angle", (float)angle * PI / 180.0f);
                                programA.setUniformValue("qt_offset", QPointF(texture.width() - 1, texture.height() - 1) / 2.0);

                                // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                                glVertexAttribPointer(programA.attributeLocation("qt_vertex"), 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
                                programA.enableAttributeArray("qt_vertex");
                                glDrawElements(GL_POINTS, (int)texture.width() * (int)texture.height(), GL_UNSIGNED_INT, nullptr);

                                // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                                pixlIndexBuffer.release();
                            }
                            pixlVertexBuffer.release();
                        }
                        programA.release();
                    }
                    frameBufferObjectA.release();
                }
                glBindTexture(GL_TEXTURE_2D, frameBufferObjectA.texture());
                glPixelStorei(GL_PACK_ALIGNMENT, 1);
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, obj.constScanLine(0));

                double entrpy = entropy(obj);
                if (entrpy < bestEntrp) {
                    bestEntrp = entrpy;
                    bestAngle = angle;
                    memcpy(bestObjX.constPointer(), obj.constPointer(), bestObjX.length());

                    qDebug() << bestAngle << bestEntrp;
                }
            }
            delta /= 4.0;
        }
        qDebug() << bestAngle << bestEntrp;

        // CREATE A LAUMEMORYOBJECT AND DOWNLOAD FBO TO IT AND SAVE TO DISK
        LAUMemoryObject bestObjY(frameBufferObjectB.width(), frameBufferObjectB.height(), 4, sizeof(float));

        // BIND THE FRAME BUFFER OBJECT FOR PROCESSING THE PHASE DFT COEFFICIENTS
        // ALONG WITH THE GLSL PROGRAMS THAT WILL DO THE PROCESSING
        if (frameBufferObjectB.bind()) {
            if (programB.bind()) {
                // CLEAR THE FRAME BUFFER OBJECT
                glViewport(0, 0, frameBufferObjectB.width(), frameBufferObjectB.height());
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
                if (pixlVertexBuffer.bind()) {
                    if (pixlIndexBuffer.bind()) {
                        // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                        glActiveTexture(GL_TEXTURE0);
                        texture.bind();
                        programB.setUniformValue("qt_texture", 0);

                        // SET THE MAXIMUM INTENSITY VALUE FOR THE INCOMING VIDEO
                        programB.setUniformValue("qt_angle", (float)bestAngle * PI / 180.0f);
                        programB.setUniformValue("qt_offset", QPointF(texture.width() - 1, texture.height() - 1) / 2.0);

                        // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                        glVertexAttribPointer(programB.attributeLocation("qt_vertex"), 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
                        programB.enableAttributeArray("qt_vertex");
                        glDrawElements(GL_POINTS, (int)texture.width() * (int)texture.height(), GL_UNSIGNED_INT, nullptr);

                        // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                        pixlIndexBuffer.release();
                    }
                    pixlVertexBuffer.release();
                }
                programB.release();
            }
            frameBufferObjectB.release();
        }

        glBindTexture(GL_TEXTURE_2D, frameBufferObjectB.texture());
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, bestObjY.constScanLine(0));
        entropy(bestObjY);

        // UPDATE THE 3X3 TRANSFORM MATRIX
        solution = findGaps(bestObjX, bestObjY);

        double xScale = (double)object.width() / (double)qMax(object.width(), object.height());
        double yScale = (double)object.height() / (double)qMax(object.width(), object.height());

        QTransform transformA, transformB, transformC;
        transformA.scale(xScale, yScale);
        transformB.rotate(bestAngle);
        transformC.scale(1.0 / xScale, 1.0 / yScale);

        QTransform transform = transformA * transformB * transformC;

        // BIND THE FRAME BUFFER OBJECT FOR PROCESSING THE PHASE DFT COEFFICIENTS
        // ALONG WITH THE GLSL PROGRAMS THAT WILL DO THE PROCESSING
        if (frameBufferObjectC && frameBufferObjectC->bind()) {
            // TURN OFF BLENDING
            glDisable(GL_BLEND);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glViewport(0, 0, frameBufferObjectC->width(), frameBufferObjectC->height());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (programC.bind()) {
                // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
                if (quadVertexBuffer.bind()) {
                    if (quadIndexBuffer.bind()) {
                        // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                        glActiveTexture(GL_TEXTURE1);
                        texture.bind();
                        programC.setUniformValue("qt_texture", 1);
                        programC.setUniformValue("qt_transform", QMatrix4x4(transform));
                        programC.setUniformValue("qt_grid", solution);

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
            frameBufferObjectC->release();

            // DOWNLOAD THE NEW OBJECT FROM THE GPU TEXTURE
            glBindTexture(GL_TEXTURE_2D, frameBufferObjectC->texture());
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            if (object.colors() == 1) {
                if (object.depth() == sizeof(unsigned char)) {
                    glGetTexImage(GL_TEXTURE_2D, 0, GL_R, GL_UNSIGNED_BYTE, object.constPointer());
                } else if (object.depth() == sizeof(unsigned short)) {
                    glGetTexImage(GL_TEXTURE_2D, 0, GL_R, GL_UNSIGNED_SHORT, object.constPointer());
                } else if (object.depth() == sizeof(float)) {
                    glGetTexImage(GL_TEXTURE_2D, 0, GL_R, GL_FLOAT, object.constPointer());
                }
            } else if (object.colors() == 3) {
                if (object.depth() == sizeof(unsigned char)) {
                    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, object.constPointer());
                } else if (object.depth() == sizeof(unsigned short)) {
                    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_SHORT, object.constPointer());
                } else if (object.depth() == sizeof(float)) {
                    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, object.constPointer());
                }
            } else if (object.colors() == 4) {
                if (object.depth() == sizeof(unsigned char)) {
                    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, object.constPointer());
                } else if (object.depth() == sizeof(unsigned short)) {
                    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_SHORT, object.constPointer());
                } else if (object.depth() == sizeof(float)) {
                    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, object.constPointer());
                }
            }

            // REDRAW THE IMAGE ON THE DISPLAY
            update();
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFindGridGLWidget::resizeGL(int w, int h)
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
void LAUFindGridGLWidget::paintGL()
{
    // SET THE VIEW PORT AND CLEAR THE SCREEN BUFFER
    glClearColor(0.5f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, localWidth, localHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (frameBufferObjectC) {
        double scaleX = (double)object.width() / (double)localWidth;
        double scaleY = (double)object.height() / (double)localHeight;
        double scale = qMax(scaleX, scaleY);

        QTransform transform;
        transform.scale(0.95 * scaleX / scale, -0.95 * scaleY / scale);

        if (programD.bind()) {
            if (quadVertexBuffer.bind()) {
                if (quadIndexBuffer.bind()) {
                    // SET THE ACTIVE TEXTURE ON THE GPU
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, frameBufferObjectC->texture());
                    programD.setUniformValue("qt_texture", 0);
                    programD.setUniformValue("qt_transform", QMatrix4x4(transform));

                    // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                    programD.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                    programD.enableAttributeArray("qt_vertex");

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                    quadIndexBuffer.release();
                }
                quadVertexBuffer.release();
            }
            programD.release();
        }
    }
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
double LAUFindGridGLWidget::entropy(LAUMemoryObject object)
{
    double cumSum[4] = { 0.0, 0.0, 0.0, 0.0 };

    for (unsigned int row = 0; row < object.height(); row++) {
        float *buffer = (float *)object.constScanLine(row);
        for (unsigned int col = 0; col < object.width(); col++) {
            if (buffer[4 * col + 3] > 0.0f) {
                buffer[4 * col + 0] /= buffer[4 * col + 3];
                buffer[4 * col + 1] /= buffer[4 * col + 3];
                buffer[4 * col + 2] /= buffer[4 * col + 3];
                buffer[4 * col + 3] /= buffer[4 * col + 3];
            } else {
                buffer[4 * col + 0] = 0.0f;
                buffer[4 * col + 1] = 0.0f;
                buffer[4 * col + 2] = 0.0f;
                buffer[4 * col + 3] = 0.0f;
            }
            cumSum[0] += (double)buffer[4 * col + 0];
            cumSum[1] += (double)buffer[4 * col + 1];
            cumSum[2] += (double)buffer[4 * col + 2];
            cumSum[3] += (double)buffer[4 * col + 3];
        }
    }

    double entrpy[4] = { 0.0, 0.0, 0.0, 0.0 };
    for (unsigned int row = 0; row < object.height(); row++) {
        float *buffer = (float *)object.constScanLine(row);
        for (unsigned int col = 0; col < object.width(); col++) {
            entrpy[0] += logFunction((double)buffer[4 * col + 0] / cumSum[0]);
            entrpy[1] += logFunction((double)buffer[4 * col + 1] / cumSum[1]);
            entrpy[2] += logFunction((double)buffer[4 * col + 2] / cumSum[2]);
            entrpy[3] += logFunction((double)buffer[4 * col + 3] / cumSum[3]);
        }
    }

    return (entrpy[0] + entrpy[1] + entrpy[2]);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObject LAUFindGridGLWidget::result(int col, int row)
{
    if (col < 0 && row < 0) {
        return (object);
    } else if (row < 0) {
        row = (col / numCols);
        col = (col % numCols);
    }
    return (object.crop(QRect(col * solution.x() + solution.y(), row * solution.z() + solution.w(), solution.x(), solution.z())));
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QVector4D LAUFindGridGLWidget::findGaps(LAUMemoryObject objX, LAUMemoryObject objY)
{
    objX.save(QString("/tmp/objX.tif"));
    objY.save(QString("/tmp/objY.tif"));

    QVector4D solution;

    float cumSumOptX = 0.0f;
    float *bufferX = reinterpret_cast<float *>(objX.constPointer());
    for (int a = (int)objX.width() / (2 * numCols); a < (int)objX.width(); a++) {
        for (int b = 0; b < (int)objX.width() / numCols; b++) {
            float cumSum = 0.0f;
            for (int n = 0; n < numCols; n++) {
                int pos = n * a + b;
                if (pos < (int)objX.width()) {
                    cumSum += bufferX[4 * pos + 0];
                    cumSum += bufferX[4 * pos + 1];
                    cumSum += bufferX[4 * pos + 2];
                } else {
                    cumSum = 0.0f;
                    break;
                }
            }

            if (cumSum > cumSumOptX) {
                cumSumOptX = cumSum;
                solution.setX((float)a);
                solution.setY((float)b);
            }
        }
    }

    float cumSumOptY = 0.0f;
    float *bufferY = reinterpret_cast<float *>(objY.constPointer());
    for (int a = (int)objY.height() / (2 * numRows); a < (int)objY.height(); a++) {
        for (int b = 0; b < (int)objY.height() / numRows; b++) {
            float cumSum = 0.0f;
            for (int n = 0; n < numRows; n++) {
                int pos = n * a + b;
                if (pos < (int)objY.height()) {
                    cumSum += bufferY[4 * pos + 0];
                    cumSum += bufferY[4 * pos + 1];
                    cumSum += bufferY[4 * pos + 2];
                } else {
                    cumSum = 0.0f;
                    break;
                }
            }

            if (cumSum > cumSumOptY) {
                cumSumOptY = cumSum;
                solution.setZ((float)a);
                solution.setW((float)b);
            }
        }
    }
    return (solution);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFindGridDialog::accept()
{
    QSettings settings;
    QString directory = settings.value("LAUMemoryObject::lastUsedDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    if (QDir(directory).exists() == false) {
        directory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }

    QString filename = QFileDialog::getSaveFileName(this, QString("Save image to disk (*.tif)"), directory);
    if (filename.isEmpty() == false) {
        settings.setValue("LAUMemoryObject::lastUsedDirectory", QFileInfo(filename).absolutePath());
        if (filename.toLower().endsWith(".tif")) {
            filename.chop(4);
        } else if (filename.toLower().endsWith(".tiff")) {
            filename.chop(5);
        }

        bool okay = false;
        int startingIndex = QFileInfo(fileString).completeBaseName().toInt(&okay);
        if (okay == false) {
            startingIndex = QInputDialog::getInt(this, QString("Find Grid"), QString("Set the starting image number:"), settings.value("LAUFindGridDialog::startingIndex", 0).toInt(), 0, 100000, 1, &okay);
        } else {
            startingIndex *= (widget->rows() * widget->cols());
        }
        if (okay) {
            settings.setValue("LAUFindGridDialog::startingIndex", startingIndex);
            for (int index = 0; index < static_cast<int>(widget->rows() * widget->cols()); index++) {
                int ind = index + startingIndex;
                QString filenameWithIndex = QString("%1").arg(ind);
                while (filenameWithIndex.length() < 5) {
                    filenameWithIndex.prepend('0');
                }
                filenameWithIndex.prepend(QString("%1_").arg(filename));
                filenameWithIndex.append(QString(".tif"));
                widget->result(index).save(filenameWithIndex);
            }
            QDialog::accept();
        }
    }
}
