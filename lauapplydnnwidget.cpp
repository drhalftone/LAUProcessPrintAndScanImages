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
        if (frameBufferObjectE) {
            delete frameBufferObjectE;
        }
        if (frameBufferObjectF) {
            delete frameBufferObjectF;
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
        object = obj.crop(DWTSTEP * (obj.width() / DWTSTEP), obj.height());

        // MAKE SURE WE HAVE OUR FRAME BUFFER OBJECTS ALLOCATED ON THE GPU
        if (frameBufferObjectA && frameBufferObjectB && frameBufferObjectC) {
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

            // BIND FRAMEBUFFER A TO HOLD THE FIRST LEVEL WAVELET DECOMPOSITION
            if (frameBufferObjectA->bind()) {
                // NOW FOCUS ON THE LOW FREQUENCY COEFFICIENTS
                glViewport(0, 0, frameBufferObjectA->width(), frameBufferObjectA->height());
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                if (program.bind()) {
                    if (quadVertexBuffer.bind()) {
                        if (quadIndexBuffer.bind()) {
                            // SET THE ACTIVE TEXTURE ON THE GPU
                            glActiveTexture(GL_TEXTURE0);
                            objectTexture->bind();
                            program.setUniformValue("qt_texture", 0);

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
                frameBufferObjectA->release();
            }

            // PREPROCESS THE IMAGE BY LOWPASS FILTERING IN THE X AND Y DIRECTION
            dwtHighPassFiltering();
            boxCarLowPassFiltering();

            // NOW START PROCESSING IMAGES IN BLOCKS OF 128 ROWS
            for (unsigned int row = 0; row < frameBufferObjectA->height(); row += 128) {
                // COPY OVER THE NEXT 128 ROWS OF THE INPUT IMAGE TO FRAME BUFFER OBJECT D
                QOpenGLFramebufferObject::blitFramebuffer(frameBufferObjectD, QRect(0, 0, frameBufferObjectD->width(), 128), frameBufferObjectA, QRect(0, row, frameBufferObjectD->width(), 128));

                // NOW IMPLEMENT THE NETWORK LAYERS
                imageInputLayer();

                // CALL THE FIRST CONVOLUTIONAL LAYER
                convolutionLayer1();
                maxPoolLayer();
                convolutionLayer2();
                maxPoolLayer();
                convolutionLayer3();
                maxPoolLayer();
                convolutionLayer4();
                maxPoolLayer();

                //            fullyConnectorLayer1();
                //            fullyConnectorLayer2();

                QOpenGLFramebufferObject::blitFramebuffer(frameBufferObjectA, QRect(0, row, frameBufferObjectD->width(), 128), frameBufferObjectD, QRect(0, 0, frameBufferObjectD->width(), 128));
            }
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUApplyDNNGLWidget::dwtHighPassFiltering()
{
    for (int lvl = 0; lvl < DWTLEVELS; lvl++) {
        // CALCULATE WINDOW SCALE FACTOR
        int sclFactorB = 1 << lvl;
        int sclFactorA = sclFactorB << 1;

        // BIND FRAMEBUFFER A TO HOLD THE FIRST LEVEL WAVELET DECOMPOSITION
        if (frameBufferObjectB->bind()) {
            // ENABLE SCISSOR TEST SO WE CAN CLEAR JUST THE VIEWPORTS
            glEnable(GL_SCISSOR_TEST);

            // NOW FOCUS ON THE LOW FREQUENCY COEFFICIENTS
            glViewport(0, 0, frameBufferObjectB->width() / sclFactorA, frameBufferObjectB->height());
            glScissor(0, 0, frameBufferObjectB->width() / sclFactorA, frameBufferObjectB->height());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (progLoD.bind()) {
                // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
                if (quadVertexBuffer.bind()) {
                    if (quadIndexBuffer.bind()) {
                        // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, frameBufferObjectA->texture());
                        progLoD.setUniformValue("qt_texture", 0);
                        progLoD.setUniformValue("qt_width", frameBufferObjectA->width() / sclFactorB);

                        // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                        progLoD.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                        progLoD.enableAttributeArray("qt_vertex");

                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                        // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                        quadIndexBuffer.release();
                    }
                    quadVertexBuffer.release();
                }
                progLoD.release();
            }

            // NOW FOCUS ON THE LOW FREQUENCY COEFFICIENTS
            glViewport(frameBufferObjectB->width() / sclFactorA, 0, frameBufferObjectB->width() / sclFactorA, frameBufferObjectB->height());
            glScissor(frameBufferObjectB->width() / sclFactorA, 0, frameBufferObjectB->width() / sclFactorA, frameBufferObjectB->height());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (progHiD.bind()) {
                // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
                if (quadVertexBuffer.bind()) {
                    if (quadIndexBuffer.bind()) {
                        // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, frameBufferObjectA->texture());
                        progHiD.setUniformValue("qt_texture", 0);
                        progHiD.setUniformValue("qt_width", frameBufferObjectA->width() / sclFactorB);

                        // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                        progHiD.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                        progHiD.enableAttributeArray("qt_vertex");

                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                        // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                        quadIndexBuffer.release();
                    }
                    quadVertexBuffer.release();
                }
                progHiD.release();
            }
            frameBufferObjectB->release();

            // DISABLE SCISSOR FOR THE REST OF THIS CONTEXT
            glDisable(GL_SCISSOR_TEST);

            // COPY OVER FRAMEBUFFER A TO FRAMEBUFFER B
            QOpenGLFramebufferObject::blitFramebuffer(frameBufferObjectA, frameBufferObjectB);
        }
    }

    // BIND FRAMEBUFFER A TO HOLD THE FIRST LEVEL WAVELET DECOMPOSITION
    if (frameBufferObjectA->bind()) {
        // ENABLE SCISSOR TEST SO WE CAN CLEAR JUST THE VIEWPORTS
        glEnable(GL_SCISSOR_TEST);

        // NOW FOCUS ON THE LOW FREQUENCY COEFFICIENTS
        glViewport(0, 0, frameBufferObjectA->width() / (1 << DWTLEVELS), frameBufferObjectA->height());
        glScissor(0, 0, frameBufferObjectA->width() / (1 << DWTLEVELS), frameBufferObjectA->height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        frameBufferObjectA->release();

        // DISABLE SCISSOR FOR THE REST OF THIS CONTEXT
        glDisable(GL_SCISSOR_TEST);
    }

    for (int lvl = DWTLEVELS - 1; lvl > -1; lvl--) {
        // CALCULATE WINDOW SCALE FACTOR
        int sclFactorB = 1 << lvl;

        // BIND FRAMEBUFFER A TO HOLD THE FIRST LEVEL WAVELET DECOMPOSITION
        if (frameBufferObjectB->bind()) {
            // ENABLE SCISSOR TEST SO WE CAN CLEAR JUST THE VIEWPORTS
            glEnable(GL_SCISSOR_TEST);

            // NOW FOCUS ON THE LOW FREQUENCY COEFFICIENTS
            glViewport(0, 0, frameBufferObjectB->width() / sclFactorB, frameBufferObjectB->height());
            glScissor(0, 0, frameBufferObjectB->width() / sclFactorB, frameBufferObjectB->height());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (progRecon.bind()) {
                // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
                if (quadVertexBuffer.bind()) {
                    if (quadIndexBuffer.bind()) {
                        // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, frameBufferObjectA->texture());
                        progRecon.setUniformValue("qt_texture", 0);
                        progRecon.setUniformValue("qt_width", frameBufferObjectA->width() / sclFactorB);

                        // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                        progRecon.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                        progRecon.enableAttributeArray("qt_vertex");

                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                        // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                        quadIndexBuffer.release();
                    }
                    quadVertexBuffer.release();
                }
                progRecon.release();
            }
            frameBufferObjectB->release();
        }

        // DISABLE SCISSOR FOR THE REST OF THIS CONTEXT
        glDisable(GL_SCISSOR_TEST);

        // COPY OVER FRAMEBUFFER A TO FRAMEBUFFER B
        QOpenGLFramebufferObject::blitFramebuffer(frameBufferObjectA, frameBufferObjectB);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUApplyDNNGLWidget::boxCarLowPassFiltering()
{
    // BIND FRAMEBUFFER A TO HOLD THE FIRST LEVEL WAVELET DECOMPOSITION
    if (frameBufferObjectB->bind()) {
        // NOW FOCUS ON THE LOW FREQUENCY COEFFICIENTS
        glViewport(0, 0, frameBufferObjectB->width(), frameBufferObjectB->height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (progVrtBoxCar.bind()) {
            // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
            if (quadVertexBuffer.bind()) {
                if (quadIndexBuffer.bind()) {
                    // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, frameBufferObjectA->texture());
                    progVrtBoxCar.setUniformValue("qt_texture", 0);
                    progVrtBoxCar.setUniformValue("qt_radius", 16);

                    // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                    progVrtBoxCar.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                    progVrtBoxCar.enableAttributeArray("qt_vertex");

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                    // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                    quadIndexBuffer.release();
                }
                quadVertexBuffer.release();
            }
            progVrtBoxCar.release();
        }
        frameBufferObjectB->release();

        // COPY OVER FRAMEBUFFER A TO FRAMEBUFFER B
        QOpenGLFramebufferObject::blitFramebuffer(frameBufferObjectA, frameBufferObjectB);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUApplyDNNGLWidget::imageInputLayer()
{
    // IN THIS LAYER, WE DIVIDE THE IMAGE INTO SWATHS OF 128 CONSECUTIVE COLUMNS
    // SUBTRACTING OUT THE MEAN VALUE FROM EACH SWATCH SO THAT MEAN VALUE IS 0

    // FIRST, WE NEED TO CALCULATE THE MEAN VALUE INSIDE SWATHS OF 128 PIXELS
    if (frameBufferObjectE->bind()) {
        // NOW FOCUS ON THE LOW FREQUENCY COEFFICIENTS
        glViewport(0, 0, frameBufferObjectE->width(), frameBufferObjectE->height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (progHorBoxCar.bind()) {
            // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
            if (quadVertexBuffer.bind()) {
                if (quadIndexBuffer.bind()) {
                    // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, frameBufferObjectD->texture());
                    progHorBoxCar.setUniformValue("qt_texture", 0);
                    progHorBoxCar.setUniformValue("qt_width", 128);

                    // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                    progHorBoxCar.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                    progHorBoxCar.enableAttributeArray("qt_vertex");

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                    // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                    quadIndexBuffer.release();
                }
                quadVertexBuffer.release();
            }
            progHorBoxCar.release();
        }
        frameBufferObjectE->release();
    }

    // NOW WE NEED TO SUBTRACT THE MEAN VALUE FROM EACH RUN OF 128 CONSECUTIVE PIXELS
    if (frameBufferObjectF->bind()) {
        // NOW FOCUS ON THE LOW FREQUENCY COEFFICIENTS
        glViewport(0, 0, frameBufferObjectF->width(), frameBufferObjectF->height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (progSubtractMeanFromSwath.bind()) {
            // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
            if (quadVertexBuffer.bind()) {
                if (quadIndexBuffer.bind()) {
                    // BIND THE INPUT TEXTURE FROM THE FRAME BUFFER OBJECT
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, frameBufferObjectD->texture());
                    progSubtractMeanFromSwath.setUniformValue("qt_textureA", 0);

                    // BIND THE MEAN TEXTURE FROM THE FRAME BUFFER OBJECT
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, frameBufferObjectE->texture());
                    progSubtractMeanFromSwath.setUniformValue("qt_textureB", 1);

                    progSubtractMeanFromSwath.setUniformValue("qt_width", 128);

                    // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                    progSubtractMeanFromSwath.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                    progSubtractMeanFromSwath.enableAttributeArray("qt_vertex");

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                    // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                    quadIndexBuffer.release();
                }
                quadVertexBuffer.release();
            }
            progSubtractMeanFromSwath.release();
        }
        frameBufferObjectF->release();

        // COPY OVER FRAMEBUFFER A TO FRAMEBUFFER B
        QOpenGLFramebufferObject::blitFramebuffer(frameBufferObjectD, frameBufferObjectF);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUApplyDNNGLWidget::convolutionLayer1()
{
    // THIS CONVOLUTIONAL LAYERS TAKES 1 ROW IN AND OUTPUTS 8 ROWS
    // THIS CONVOLUTIONAL LAYERS TAKES 1 ROW IN AND OUTPUTS 8 ROWS
    // THIS CONVOLUTIONAL LAYERS TAKES 1 ROW IN AND OUTPUTS 8 ROWS

    // BIND FRAMEBUFFER A TO HOLD THE FIRST LEVEL WAVELET DECOMPOSITION
    if (frameBufferObjectE->bind()) {
        // CLEAR THE ENTIRE OUTPUT BUFFER
        glViewport(0, 0, frameBufferObjectE->width(), frameBufferObjectE->height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (progConv1.bind()) {
            // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
            if (quadVertexBuffer.bind()) {
                if (quadIndexBuffer.bind()) {
                    // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, frameBufferObjectD->texture());
                    progConv1.setUniformValue("qt_texture", 0);

                    // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                    progConv1.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                    progConv1.enableAttributeArray("qt_vertex");

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                    // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                    quadIndexBuffer.release();
                }
                quadVertexBuffer.release();
            }
            progConv1.release();
        }
        frameBufferObjectE->release();

        // COPY OVER FRAMEBUFFER A TO FRAMEBUFFER B
        QOpenGLFramebufferObject::blitFramebuffer(frameBufferObjectD, frameBufferObjectE);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUApplyDNNGLWidget::convolutionLayer2()
{
    // THIS CONVOLUTIONAL LAYERS TAKES 8 ROWS IN AND OUTPUTS 1 ROW
    // THIS CONVOLUTIONAL LAYERS TAKES 8 ROWS IN AND OUTPUTS 1 ROW
    // THIS CONVOLUTIONAL LAYERS TAKES 8 ROWS IN AND OUTPUTS 1 ROW

    // BIND FRAMEBUFFER A TO HOLD THE FIRST LEVEL WAVELET DECOMPOSITION
    if (frameBufferObjectE->bind()) {
        // CLEAR THE ENTIRE OUTPUT BUFFER
        glViewport(0, 0, frameBufferObjectE->width(), frameBufferObjectE->height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // NOW FOCUS ON JUST THE FIRST 1/8th ROWS
        glViewport(0, 0, frameBufferObjectE->width(), frameBufferObjectE->height() / 8);

        if (progConv2.bind()) {
            // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
            if (quadVertexBuffer.bind()) {
                if (quadIndexBuffer.bind()) {
                    // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, frameBufferObjectD->texture());
                    progConv2.setUniformValue("qt_texture", 0);

                    // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                    progConv2.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                    progConv2.enableAttributeArray("qt_vertex");

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                    // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                    quadIndexBuffer.release();
                }
                quadVertexBuffer.release();
            }
            progConv2.release();
        }
        frameBufferObjectE->release();

        // COPY OVER FRAMEBUFFER A TO FRAMEBUFFER B
        QOpenGLFramebufferObject::blitFramebuffer(frameBufferObjectD, frameBufferObjectE);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUApplyDNNGLWidget::convolutionLayer3()
{
    // THIS CONVOLUTIONAL LAYERS TAKES 1 ROW IN AND OUTPUTS 1 ROW
    // THIS CONVOLUTIONAL LAYERS TAKES 1 ROW IN AND OUTPUTS 1 ROW
    // THIS CONVOLUTIONAL LAYERS TAKES 1 ROW IN AND OUTPUTS 1 ROW

    // BIND FRAMEBUFFER A TO HOLD THE FIRST LEVEL WAVELET DECOMPOSITION
    if (frameBufferObjectE->bind()) {
        // CLEAR THE FIRST 1/8TH OF THE OUTPUT BUFFER
        glViewport(0, 0, frameBufferObjectE->width(), frameBufferObjectE->height() / 8);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (progConv3.bind()) {
            // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
            if (quadVertexBuffer.bind()) {
                if (quadIndexBuffer.bind()) {
                    // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, frameBufferObjectD->texture());
                    progConv3.setUniformValue("qt_texture", 0);

                    // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                    progConv3.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                    progConv3.enableAttributeArray("qt_vertex");

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                    // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                    quadIndexBuffer.release();
                }
                quadVertexBuffer.release();
            }
            progConv3.release();
        }
        frameBufferObjectE->release();

        // COPY OVER FRAMEBUFFER A TO FRAMEBUFFER B
        QOpenGLFramebufferObject::blitFramebuffer(frameBufferObjectD, frameBufferObjectE);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUApplyDNNGLWidget::convolutionLayer4()
{
    // THIS CONVOLUTIONAL LAYERS TAKES 1 ROW IN AND OUTPUTS 1 ROW
    // THIS CONVOLUTIONAL LAYERS TAKES 1 ROW IN AND OUTPUTS 1 ROW
    // THIS CONVOLUTIONAL LAYERS TAKES 1 ROW IN AND OUTPUTS 1 ROW

    // BIND FRAMEBUFFER A TO HOLD THE FIRST LEVEL WAVELET DECOMPOSITION
    if (frameBufferObjectE->bind()) {
        // CLEAR THE FIRST 1/8TH OF THE OUTPUT BUFFER
        glViewport(0, 0, frameBufferObjectE->width(), frameBufferObjectE->height() / 8);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (progConv4.bind()) {
            // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
            if (quadVertexBuffer.bind()) {
                if (quadIndexBuffer.bind()) {
                    // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, frameBufferObjectD->texture());
                    progConv4.setUniformValue("qt_texture", 0);

                    // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                    progConv4.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                    progConv4.enableAttributeArray("qt_vertex");

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                    // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                    quadIndexBuffer.release();
                }
                quadVertexBuffer.release();
            }
            progConv4.release();
        }
        frameBufferObjectE->release();

        // COPY OVER FRAMEBUFFER A TO FRAMEBUFFER B
        QOpenGLFramebufferObject::blitFramebuffer(frameBufferObjectD, frameBufferObjectE);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUApplyDNNGLWidget::maxPoolLayer()
{
    // BIND FRAMEBUFFER A TO HOLD THE FIRST LEVEL WAVELET DECOMPOSITION
    if (frameBufferObjectE->bind()) {
        // CLEAR FRAME BUFFER OBJECT E
        glViewport(0, 0, frameBufferObjectE->width(), frameBufferObjectE->height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // NOW LETS FOCUS IN ON THE LEFT HALF FOR THE MAX POOL OPERATION
        glViewport(0, 0, frameBufferObjectE->width() / 2, frameBufferObjectE->height());

        if (progMaxPool.bind()) {
            // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
            if (quadVertexBuffer.bind()) {
                if (quadIndexBuffer.bind()) {
                    // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, frameBufferObjectD->texture());
                    progMaxPool.setUniformValue("qt_texture", 0);

                    // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                    progMaxPool.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                    progMaxPool.enableAttributeArray("qt_vertex");

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                    // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                    quadIndexBuffer.release();
                }
                quadVertexBuffer.release();
            }
            progMaxPool.release();
        }
        frameBufferObjectE->release();

        // COPY OVER FRAMEBUFFER A TO FRAMEBUFFER B
        QOpenGLFramebufferObject::blitFramebuffer(frameBufferObjectD, frameBufferObjectE);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUApplyDNNGLWidget::fullyConnectorLayer1()
{
    ;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUApplyDNNGLWidget::fullyConnectorLayer2()
{
    ;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUApplyDNNGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

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
    objectTexture->setSize(DWTSTEP * ((int)object.width() / DWTSTEP), (int)object.height());
    objectTexture->setFormat(QOpenGLTexture::RGBA32F);
    objectTexture->setWrapMode(QOpenGLTexture::ClampToBorder);
    objectTexture->setMinificationFilter(QOpenGLTexture::Linear);
    objectTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    objectTexture->allocateStorage();

    // CREATE A FORMAT OBJECT FOR CREATING THE FRAME BUFFER
    frameBufferObjectFormat.setTextureTarget(GL_TEXTURE_2D);
    frameBufferObjectFormat.setInternalTextureFormat(GL_RGBA32F);

    // CREATE NEW FRAME BUFFER OBJECTS
    frameBufferObjectA = new QOpenGLFramebufferObject(DWTSTEP * ((int)object.width() / DWTSTEP), (int)object.height(), frameBufferObjectFormat);
    frameBufferObjectA->release();
    frameBufferObjectB = new QOpenGLFramebufferObject(DWTSTEP * ((int)object.width() / DWTSTEP), (int)object.height(), frameBufferObjectFormat);
    frameBufferObjectB->release();
    frameBufferObjectC = new QOpenGLFramebufferObject(DWTSTEP * ((int)object.width() / DWTSTEP), (int)object.height(), frameBufferObjectFormat);
    frameBufferObjectC->release();

    // CREATE FRAME BUFFER OBJECTS WHEN PROCESSING 128 INPUT IMAGE ROWS AT A TIME
    frameBufferObjectD = new QOpenGLFramebufferObject(DWTSTEP * ((int)object.width() / DWTSTEP), 8 * 128, frameBufferObjectFormat);
    frameBufferObjectD->release();
    frameBufferObjectE = new QOpenGLFramebufferObject(DWTSTEP * ((int)object.width() / DWTSTEP), 8 * 128, frameBufferObjectFormat);
    frameBufferObjectE->release();
    frameBufferObjectF = new QOpenGLFramebufferObject(DWTSTEP * ((int)object.width() / DWTSTEP), 8 * 128, frameBufferObjectFormat);
    frameBufferObjectF->release();

    // CREATE SHADERS
    setlocale(LC_NUMERIC, "C");
    if (object.colors() == 1) {
        program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Shaders/displayGRAYVideo.vert");
        program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Shaders/displayGRAYVideo.frag");
        program.link();
    } else {
        program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Shaders/displayRGBAVideo.vert");
        program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Shaders/displayRGBAVideo.frag");
        program.link();
    }

    progLoD.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Shaders/DNN/waveletLODecompFilter.vert");
    progLoD.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Shaders/DNN/waveletLODecompFilter.frag");
    progLoD.link();

    progHiD.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Shaders/DNN/waveletHIDecompFilter.vert");
    progHiD.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Shaders/DNN/waveletHIDecompFilter.frag");
    progHiD.link();

    progRecon.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Shaders/DNN/waveletReconstFilter.vert");
    progRecon.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Shaders/DNN/waveletReconstFilter.frag");
    progRecon.link();

    progHorBoxCar.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Shaders/DNN/horBoxCarFilter.vert");
    progHorBoxCar.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Shaders/DNN/horBoxCarFilter.frag");
    progHorBoxCar.link();

    progVrtBoxCar.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Shaders/DNN/vrtBoxCarFilter.vert");
    progVrtBoxCar.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Shaders/DNN/vrtBoxCarFilter.frag");
    progVrtBoxCar.link();

    progSubtractMeanFromSwath.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Shaders/DNN/subMeanFromSwath.vert");
    progSubtractMeanFromSwath.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Shaders/DNN/subMeanFromSwath.frag");
    progSubtractMeanFromSwath.link();

    progConv1.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Shaders/DNN/convLayer1Filter.vert");
    progConv1.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Shaders/DNN/convLayer1Filter.frag");
    progConv1.link();

    progConv2.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Shaders/DNN/convLayer2Filter.vert");
    progConv2.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Shaders/DNN/convLayer2Filter.frag");
    progConv2.link();

    progConv3.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Shaders/DNN/convLayer3Filter.vert");
    progConv3.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Shaders/DNN/convLayer3Filter.frag");
    progConv3.link();

    progConv4.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Shaders/DNN/convLayer4Filter.vert");
    progConv4.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Shaders/DNN/convLayer4Filter.frag");
    progConv4.link();

    progMaxPool.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Shaders/DNN/maxPoolFilter.vert");
    progMaxPool.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Shaders/DNN/maxPoolFilter.frag");
    progMaxPool.link();

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

    if (frameBufferObjectA) {
        if (program.bind()) {
            if (quadVertexBuffer.bind()) {
                if (quadIndexBuffer.bind()) {
                    // SET THE ACTIVE TEXTURE ON THE GPU
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, frameBufferObjectA->texture());
                    program.setUniformValue("qt_texture", 0);

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
    } else if (objectTexture) {
        if (program.bind()) {
            if (quadVertexBuffer.bind()) {
                if (quadIndexBuffer.bind()) {
                    // SET THE ACTIVE TEXTURE ON THE GPU
                    glActiveTexture(GL_TEXTURE0);
                    objectTexture->bind();
                    program.setUniformValue("qt_texture", 0);

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
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObject LAUApplyDNNGLWidget::result()
{
    // MAKE SURE WE HAVE AN FBO TO READ FROM
    if (frameBufferObjectA) {
        // MAKE THIS THE CURRENT OPENGL CONTEXT
        makeCurrent();

        // CREATE A MEMORY OBJECT AND DOWNLOAD FRAME BUFFER OBJECT TO IT
        LAUMemoryObject obj(frameBufferObjectA->width(), frameBufferObjectA->height(), 4, sizeof(float));
        glBindTexture(GL_TEXTURE_2D, frameBufferObjectA->texture());
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, obj.constPointer());

        // RETURN OUR CPU MEMORY TO THE USER
        return (obj);
    }
    return (LAUMemoryObject());
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
