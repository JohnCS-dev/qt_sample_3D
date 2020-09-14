#include "basescene3d.h"

#include <QPainter>
#include <QMouseEvent>
#include <QtMath>
#include <QMetaProperty>
#include <QDesktopWidget>
#include <QLabel>
#include "varianteditor.h"

#define LOGICAL_COEF 100.0f
#define EPSILON 0.00001f

const QColor BackgroundColor = Qt::white;
const QColor ScaleMarkColor = Qt::black;

void BaseScene3D::qgluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
    const GLdouble ymax = zNear * tan(fovy * M_PI / 360.0);
    const GLdouble ymin = -ymax;
    const GLdouble xmin = ymin * aspect;
    const GLdouble xmax = ymax * aspect;
    glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
}

void BaseScene3D::updateXScaleValues(float start, float end, float step, int precision)
{
    fScalesSettings[slX].start = start;
    fScalesSettings[slX].length = end - start;
    fScalesSettings[slX].step = step;
    fScalesSettings[slX].precision = precision;

    {
        QPainter painter;
        int imageH = LOGICAL_COEF * fSpaceData.xLength;
        int w = LOGICAL_COEF;

        QStringList viList;
        float v = start;
        while (v <= end + EPSILON /*|| v <= end - EPSILON*/) {
            viList.append(QString("%1").arg(v, 0, 'f', precision));
            v += step;
        }

        QImage imageR(w, imageH, QImage::Format_ARGB32);
        imageR.fill(QColor(255,255,255,0));

        float h = imageH * (step / fScalesSettings[slX].length);
        painter.begin(&imageR);
        painter.setFont(fScaleFont);
        painter.setPen(fScaleColor);

        for (int i = 0; i < viList.count(); i++) {
            float y = imageH - h * i;
            Qt::Alignment tAlign = Qt::AlignRight;
            if (i == 0) {
                tAlign |= Qt::AlignBottom;
                y -= h;
            }
            else if (i == viList.count() - 1) {
                tAlign |= Qt::AlignTop;
            }
            else {
                tAlign |= Qt::AlignVCenter;
                y -= (h / 2);
            }

            QRectF r(0, y, w - 2, h);
            painter.drawText(r, tAlign, viList[i]);
        }
        painter.end();

        QImage imageL(w, imageH, QImage::Format_ARGB32);
        imageL.fill(QColor(255,255,255,0));
        painter.begin(&imageL);
        painter.setFont(fScaleFont);
        painter.setPen(fScaleColor);
        for (int i = 0; i < viList.count(); i++) {
            float y = imageH - h * i;
            Qt::Alignment tAlign = Qt::AlignLeft;
            if (i == 0) {
                tAlign |= Qt::AlignBottom;
                y -= h;
            }
            else if (i == viList.count() - 1) {
                tAlign |= Qt::AlignTop;
            }
            else {
                tAlign |= Qt::AlignVCenter;
                y -= (h / 2);
            }

            QRectF r(2, y, w - 2, h);
            painter.drawText(r, tAlign, viList[i]);
        }
        painter.end();

        QImage imageIR(w, imageH, QImage::Format_ARGB32);
        imageIR.fill(QColor(255,255,255,0));
        painter.begin(&imageIR);
        painter.setFont(fScaleFont);
        painter.setPen(fScaleColor);
        for (int i = 0; i < viList.count(); i++) {
            float y = h * i;
            Qt::Alignment tAlign = Qt::AlignRight;
            if (i == 0) {
                tAlign |= Qt::AlignTop;
            }
            else if (i == viList.count() - 1) {
                tAlign |= Qt::AlignBottom;
                y -= h;
            }
            else {
                tAlign |= Qt::AlignVCenter;
                y -= (h / 2);
            }

            QRectF r(0, y, w - 2, h);
            painter.drawText(r, tAlign, viList[i]);
        }
        painter.end();

        QImage imageIL(w, imageH, QImage::Format_ARGB32);
        imageIL.fill(QColor(255,255,255,0));
        painter.begin(&imageIL);
        painter.setFont(fScaleFont);
        painter.setPen(fScaleColor);
        for (int i = 0; i < viList.count(); i++) {
            float y = h * i;
            Qt::Alignment tAlign = Qt::AlignLeft;
            if (i == 0) {
                tAlign |= Qt::AlignTop;
            }
            else if (i == viList.count() - 1) {
                tAlign |= Qt::AlignBottom;
                y -= h;
            }
            else {
                tAlign |= Qt::AlignVCenter;
                y -= (h / 2);
            }

            QRectF r(2, y, w - 2, h);
            painter.drawText(r, tAlign, viList[i]);
        }
        painter.end();

        if (xScaleTextureRight) {
            xScaleTextureRight->destroy();
            delete xScaleTextureRight;
        }

        xScaleTextureRight = new QOpenGLTexture(imageR.mirrored(), QOpenGLTexture::DontGenerateMipMaps);
        xScaleTextureRight->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::LinearMipMapLinear);

        if (xScaleTextureLeft) {
            xScaleTextureLeft->destroy();
            delete xScaleTextureLeft;
        }

        xScaleTextureLeft = new QOpenGLTexture(imageL.mirrored(), QOpenGLTexture::DontGenerateMipMaps);
        xScaleTextureLeft->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::LinearMipMapLinear);

        if (xInverseScaleTextureRight) {
            xInverseScaleTextureRight->destroy();
            delete xInverseScaleTextureRight;
        }

        xInverseScaleTextureRight = new QOpenGLTexture(imageIR.mirrored(), QOpenGLTexture::DontGenerateMipMaps);
        xInverseScaleTextureRight->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::LinearMipMapLinear);

        if (xInverseScaleTextureLeft) {
            xInverseScaleTextureLeft->destroy();
            delete xInverseScaleTextureLeft;
        }

        xInverseScaleTextureLeft = new QOpenGLTexture(imageIL.mirrored(), QOpenGLTexture::DontGenerateMipMaps);
        xInverseScaleTextureLeft->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::LinearMipMapLinear);
    }
}

void BaseScene3D::updateYScaleValues()
{
    updateYScaleValues(fScalesSettings[slY].start, fScalesSettings[slY].start + fScalesSettings[slY].length, fScalesSettings[slY].step, fScalesSettings[slY].precision);
}

void BaseScene3D::updateYScaleValues(float start, float end, float step, int precision)
{
    fScalesSettings[slY].start = start;
    fScalesSettings[slY].length = end - start;
    fScalesSettings[slY].step = step;
    fScalesSettings[slY].precision = precision;

    {
        QPainter painter;
        int imageH = LOGICAL_COEF * fSpaceData.yLength;
        int w = LOGICAL_COEF;
        QStringList vList;
        QStringList viList;
        float v = start;
        while (v <= end + EPSILON) {
            viList.append(QString("%1").arg(v, 0, 'f', precision));
            v += step;
        }

        QImage imageR(w, imageH, QImage::Format_ARGB32);
        imageR.fill(QColor(255,255,255,0));
        float h = imageH * (step / fScalesSettings[slY].length);
        painter.begin(&imageR);
        painter.setFont(fScaleFont);
        painter.setPen(fScaleColor);
        for (int i = 0; i < viList.count(); i++) {
            float y = imageH - h * i;
            Qt::Alignment tAlign = Qt::AlignRight;
            if (i == 0) {
                tAlign |= Qt::AlignBottom;
                y -= h;
            }
            else if (i == viList.count() - 1) {
                tAlign |= Qt::AlignTop;
            }
            else {
                tAlign |= Qt::AlignVCenter;
                y -= (h / 2);
            }

            QRectF r(0, y, w - 2, h);
            painter.drawText(r, tAlign, viList[i]);
        }
        painter.end();

        QImage imageL(w, imageH, QImage::Format_ARGB32);
        imageL.fill(QColor(255,255,255,0));
        painter.begin(&imageL);
        painter.setFont(fScaleFont);
        painter.setPen(fScaleColor);
        for (int i = 0; i < viList.count(); i++) {
            float y = imageH - h * i;
            Qt::Alignment tAlign = Qt::AlignLeft;
            if (i == 0) {
                tAlign |= Qt::AlignBottom;
                y -= h;
            }
            else if (i == viList.count() - 1) {
                tAlign |= Qt::AlignTop;
            }
            else {
                tAlign |= Qt::AlignVCenter;
                y -= (h / 2);
            }

            QRectF r(2, y, w - 2, h);
            painter.drawText(r, tAlign, viList[i]);
        }
        painter.end();

        QImage imageIR(w, imageH, QImage::Format_ARGB32);
        imageIR.fill(QColor(255,255,255,0));
        painter.begin(&imageIR);
        painter.setFont(fScaleFont);
        painter.setPen(fScaleColor);
        for (int i = 0; i < viList.count(); i++) {
            float y = /*delta +*/ h * i;
            Qt::Alignment tAlign = Qt::AlignRight;
            if (i == 0)
                tAlign |= Qt::AlignTop;
            else if (i == viList.count() - 1) {
                tAlign |= Qt::AlignBottom;
                y -= h;
            }
            else {
                tAlign |= Qt::AlignVCenter;
                y -= (h / 2);
            }

            QRectF r(0, y, w - 2, h);
            painter.drawText(r, tAlign, viList[i]);
        }
        painter.end();

        QImage imageIL(w, imageH, QImage::Format_ARGB32);
        imageIL.fill(QColor(255,255,255,0));
        painter.begin(&imageIL);
        painter.setFont(fScaleFont);
        painter.setPen(fScaleColor);
        for (int i = 0; i < viList.count(); i++) {
            float y = h * i;
            Qt::Alignment tAlign = Qt::AlignLeft;
            if (i == 0)
                tAlign |= Qt::AlignTop;
            else if (i == viList.count() - 1) {
                tAlign |= Qt::AlignBottom;
                y -= h;
            }
            else {
                tAlign |= Qt::AlignVCenter;
                y -= (h / 2);
            }

            QRectF r(2, y, w - 2, h);
            painter.drawText(r, tAlign, viList[i]);
        }
        painter.end();

        if (yScaleTextureRight)
            delete yScaleTextureRight;

        yScaleTextureRight = new QOpenGLTexture(imageR.mirrored(), QOpenGLTexture::DontGenerateMipMaps);
        yScaleTextureRight->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::LinearMipMapLinear);

        if (yScaleTextureLeft)
            delete yScaleTextureLeft;

        yScaleTextureLeft = new QOpenGLTexture(imageL.mirrored(), QOpenGLTexture::DontGenerateMipMaps);
        yScaleTextureLeft->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::LinearMipMapLinear);

        if (yInverseScaleTextureRight)
            delete yInverseScaleTextureRight;

        yInverseScaleTextureRight = new QOpenGLTexture(imageIR.mirrored(), QOpenGLTexture::DontGenerateMipMaps);
        yInverseScaleTextureRight->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::LinearMipMapLinear);

        if (yInverseScaleTextureLeft)
            delete yInverseScaleTextureLeft;

        yInverseScaleTextureLeft = new QOpenGLTexture(imageIL.mirrored(), QOpenGLTexture::DontGenerateMipMaps);
        yInverseScaleTextureLeft->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::LinearMipMapLinear);
    }
}

void BaseScene3D::updateZScaleValues()
{
    updateZScaleValues(fScalesSettings[slZ].start, fScalesSettings[slZ].start + fScalesSettings[slZ].length, fScalesSettings[slZ].step, fScalesSettings[slZ].precision);
}

void BaseScene3D::updateZScaleValues(float start, float end, float step, int precision)
{
    fScalesSettings[slZ].start = start;
    fScalesSettings[slZ].length = end - start;
    fScalesSettings[slZ].step = step;
    fScalesSettings[slZ].precision = precision;

    {
        QPainter painter;
        int imageH = LOGICAL_COEF * fSpaceData.zLength;
        int w = LOGICAL_COEF;
        QStringList vList;
        float v = start;
        while (v <= end + EPSILON) {
            vList.append(QString("%1").arg(v, 0, 'f', precision));
            v += step;
        }

        QImage image(w, imageH, QImage::Format_ARGB32);
        image.fill(QColor(255,255,255,0));
        float h = imageH * (step / fScalesSettings[slZ].length);
        painter.begin(&image);
        painter.setFont(fScaleFont);
        painter.setPen(fScaleColor);
        for (int i = 0; i < vList.count(); i++) {
            float y = imageH - h * i;
            Qt::Alignment tAlign = Qt::AlignRight;
            if (i == 0) {
                tAlign |= Qt::AlignBottom;
                y -= h;
            }
            else if (i == vList.count() - 1) {
                tAlign |= Qt::AlignTop;
            }
            else {
                tAlign |= Qt::AlignVCenter;
                y -= (h / 2);
            }

            QRectF r(0, y, w - 2, h);
            painter.drawText(r, tAlign, vList[i]);
        }
        painter.end();

        QImage inverseImage(w, imageH, QImage::Format_ARGB32);
        inverseImage.fill(QColor(255,255,255,0));
        painter.begin(&inverseImage);
        painter.setFont(fScaleFont);
        painter.setPen(fScaleColor);
        for (int i = 0; i < vList.count(); i++) {
            float y = imageH - h * i;
            Qt::Alignment tAlign = Qt::AlignLeft;
            if (i == 0) {
                tAlign |= Qt::AlignBottom;
                y -= h;
            }
            else if (i == vList.count() - 1) {
                tAlign |= Qt::AlignTop;
            }
            else {
                tAlign |= Qt::AlignVCenter;
                y -= (h / 2);
            }

            QRectF r(2, y, w - 2, h);
            painter.drawText(r, tAlign, vList[i]);
        }
        painter.end();

        if (zScaleTextureRight)
            delete zScaleTextureRight;

        zScaleTextureRight = new QOpenGLTexture(image.mirrored(), QOpenGLTexture::DontGenerateMipMaps);
        zScaleTextureRight->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::LinearMipMapLinear);
        if (zScaleTextureLeft)
            delete zScaleTextureLeft;

        zScaleTextureLeft = new QOpenGLTexture(inverseImage.mirrored(), QOpenGLTexture::DontGenerateMipMaps);
        zScaleTextureLeft->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::LinearMipMapLinear);
    }
}

void BaseScene3D::setXScaleRange(GLfloat start, GLfloat end)
{
    axisXStart = start;
    axisXEnd = end;
    if (fArrowX) {
        fArrowX->setStart(axisXEnd, 0, 0);
        fArrowX->setLength(axisXEnd - axisXStart);
    }
}

void BaseScene3D::updateXScaleValues()
{
    updateXScaleValues(fScalesSettings[slX].start, fScalesSettings[slX].start + fScalesSettings[slX].length, fScalesSettings[slX].step, fScalesSettings[slX].precision);
}

BaseScene3D::BaseScene3D(QWidget* pwgt) : QOpenGLWidget(pwgt), fSettings("view3DSettings", QStringLiteral("3D view settings")), fScaleFont("Arial",10),
	xRotate(-45), yRotate(0), zRotate(45), zTransl(0), nScale(1), xTransl(0), yTransl(0), zCam(-6), axisXStart(-3.0f), axisXEnd(3.0f)
{    
    rBut = false;    
    light = false;
    fSmooth = false;
    pManager = nullptr;
    fArrowX = nullptr;
    fMoveMode = mmObjectMode;

    setFocusPolicy(Qt::StrongFocus);

    xScaleTextureRight = nullptr;
    xScaleTextureLeft = nullptr;
    xInverseScaleTextureRight = nullptr;
    xInverseScaleTextureLeft = nullptr;

    yScaleTextureRight = nullptr;
    yScaleTextureLeft = nullptr;
    yInverseScaleTextureRight = nullptr;
    yInverseScaleTextureLeft = nullptr;

    zScaleTextureRight = nullptr;
    zScaleTextureLeft = nullptr;

    fScalesPlaneSettings[spXY].visible = true;
    fScalesPlaneSettings[spXY].offset = 2.995f;

    fScalesPlaneSettings[spYZ].visible = true;
    fScalesPlaneSettings[spYZ].offset = 5.995f;

    fScalesPlaneSettings[spXZ].visible = true;
    fScalesPlaneSettings[spXZ].offset = 2.995f;

    fScalesSettings[slX].start = -3.0f;
    fScalesSettings[slX].length = 6.0f;
    fScalesSettings[slX].step = 0.25f;

    fScalesSettings[slY].start = -3.0f;
    fScalesSettings[slY].length = 6.0f;
    fScalesSettings[slY].step = 0.25f;

    fScalesSettings[slZ].start = -3.0f;
    fScalesSettings[slZ].length = 6.0f;
    fScalesSettings[slZ].step = 0.25f;

    setSpaceData(-3, -3, -3, 6, 6, 6);
}

BaseScene3D::~BaseScene3D()
{
    if (pManager)
        delete pManager;

    if (xScaleTextureRight)
        delete xScaleTextureRight;
    if (xScaleTextureLeft)
        delete xScaleTextureLeft;

    if (xInverseScaleTextureRight)
        delete xInverseScaleTextureRight;
    if (xInverseScaleTextureLeft)
        delete xInverseScaleTextureLeft;

    if (yScaleTextureRight)
        delete yScaleTextureRight;
    if (yScaleTextureLeft)
        delete yScaleTextureLeft;

    if (yInverseScaleTextureRight)
        delete yInverseScaleTextureRight;
    if (yInverseScaleTextureLeft)
        delete yInverseScaleTextureLeft;

    if (zScaleTextureRight)
        delete zScaleTextureRight;
    if (zScaleTextureLeft)
        delete zScaleTextureLeft;
}

void BaseScene3D::initializeGL() // инициализация
{
   initializeOpenGLFunctions();

   QColor clearColor = BackgroundColor;
   glClearColor(clearColor.redF(),clearColor.greenF(),clearColor.blueF(),clearColor.alphaF());         // цвет для очистки буфера изображения - здесь просто фон окна
   glEnable(GL_DEPTH_TEST);          // устанавливает режим проверки глубины объектов   
   glFrontFace(GL_CW);               // обход вершин против часовой стрелки (GL_CCW - по часовой)

   fShaderAvailable = hasOpenGLFeature(QOpenGLFunctions::Shaders);
   fVertexBufferAvailable = hasOpenGLFeature(QOpenGLFunctions::Buffers);

#ifdef QT_DEBUG
   if (!fVertexBufferAvailable)
       qDebug() << "!!! Vertex and index buffer functions are NOT available!!!";

   if (!hasOpenGLFeature(QOpenGLFunctions::FixedFunctionPipeline))
       qDebug() << "!!! The fixed function pipeline is NOT available.";

   if (!fShaderAvailable)
       qDebug() << "!!! Shader functions are NOT available!!!";

   qDebug() << format();
   QString vendor, renderer, version, glslVersion;
   const GLubyte *p;
   if ((p = glGetString(GL_VENDOR)))
       vendor = QString::fromLatin1(reinterpret_cast<const char *>(p));
   if ((p = glGetString(GL_RENDERER)))
       renderer = QString::fromLatin1(reinterpret_cast<const char *>(p));
   if ((p = glGetString(GL_VERSION)))
       version = QString::fromLatin1(reinterpret_cast<const char *>(p));
   if ((p = glGetString(GL_SHADING_LANGUAGE_VERSION)))
       glslVersion = QString::fromLatin1(reinterpret_cast<const char *>(p));
   qDebug() << "vendor:" << vendor;
   qDebug() << "renderer:" << renderer;
   qDebug() << "version:" << version;
   qDebug() << "glslVersion:" << glslVersion;
#endif

   if (fShaderAvailable && fVertexBufferAvailable) {
       pManager = new PrimitiveManager();
       pManager->compileShaders(":/BaseShaders/Lib/base_vsh.vert", ":/BaseShaders/Lib/base_fsh.frag");

       fArrowX = dynamic_cast<PrimitiveSimpleArrow*>(pManager->addSimpleArrow(6,  axisXEnd - axisXStart, 0.20f, 0.05f, QVector3D(1,0,0)));
       fArrowX->setPos(QVector3D(axisXStart,0,0));
       fArrowX->setColor(Qt::gray);

       Primitive *p = pManager->addSimpleArrow(6, 6, 0.20f, 0.05f, QVector3D(0,1,0));
       p->setPos(QVector3D(0,-3,0));
       p->setColor(Qt::gray);
       p->setDrawType(dtWireFrame);

       p = pManager->addSimpleArrow(6, 6, 0.20f, 0.05f);
       p->setPos(QVector3D(0,0,-3));
       p->setColor(Qt::gray);
       p->setDrawType(dtWireFrame);
   }
   updateXScaleValues(-6, 6, 0.5f, 2);
   updateYScaleValues(-3, 3, 0.25f, 2);
   updateZScaleValues(-3, 3, 0.25f, 2);
}

void BaseScene3D::resizeGL(int nWidth, int nHeight) // окно виджета
{
   glMatrixMode(GL_PROJECTION);                     // устанавливает текущей проекционную матрицу
   glLoadIdentity();                                // присваивает проекционной матрице единичную матрицу

   GLfloat aspect = (GLfloat)nWidth / (GLfloat)nHeight;

   // поле просмотра
   glViewport(0, 0, (GLint)nWidth, (GLint)nHeight);
   qgluPerspective(60.0, aspect, 1.0, 250.0);
}

void BaseScene3D::paintGL()
{
    {
        QPainter painter(this);
        painter.beginNativePainting();

        prepareView();

        QMatrix4x4 pmvMatrix;
        pmvMatrix.perspective(60.0, (GLfloat)width() / (GLfloat)height(), 1.0, 250.0);
        pmvMatrix.translate(0.0f, 0.0f, zCam);
        pmvMatrix.rotate(xRotate, 1.0f, 0.0f, 0.0f);            // поворот вокруг оси X
        pmvMatrix.rotate(yRotate, 0.0f, 1.0f, 0.0f);            // поворот вокруг оси Y
        pmvMatrix.rotate(zRotate, 0.0f, 0.0f, 1.0f);            // поворот вокруг оси Z
        pmvMatrix.translate(xTransl, yTransl, zTransl);
        pmvMatrix.scale(nScale, nScale, nScale);

        drawAxis(pmvMatrix);                                      // рисование осей координат
        drawScales(pmvMatrix);

        paintData(pmvMatrix);

        drawScales(pmvMatrix);

    }
   QFont font("Arial", 14);
   QColor textColor = ScaleMarkColor;
   renderText(axisXEnd + 0.1f, 0, 0, "X", font, textColor);
   renderText(0, 3 + 0.1f, 0, "Y", font, textColor);
   renderText(0, 0, 3 + 0.2f, "Z", font, textColor);

   drawText();
}

void BaseScene3D::mousePressEvent(QMouseEvent* pe)
{
    ptrMousePosition = pe->pos();

    isMouseMove = false;

    if (pe->button() == Qt::LeftButton)
        rBut = false;
    else if (pe->button() == Qt::RightButton)
        rBut = true;
}

void BaseScene3D::mouseReleaseEvent(QMouseEvent* pe)
{
   Q_UNUSED(pe);
}

void BaseScene3D::mouseMoveEvent(QMouseEvent* pe)
{
    if (rBut){
        xRotate += 180 / nScale * (GLfloat)(pe->y() - ptrMousePosition.y()) / height();
        zRotate += 180 / nScale * (GLfloat)(pe->x() - ptrMousePosition.x()) / width();
        ptrMousePosition = pe->pos();

        if (xRotate > 0.0f)
            xRotate = 0.0f;
        else if (xRotate < -180.0f)
            xRotate = -180.0f;
    }
    isMouseMove = true;

   update();
}

void BaseScene3D::wheelEvent(QWheelEvent* pe)
{
    int delta = (pe->modifiers() & Qt::ControlModifier) ? 10 : 1;

   if (pe->delta() > 0)
       zCam = zCam + 0.10 * delta;
   else if ((pe->delta()) < 0)
       zCam = zCam - 0.10 * delta;

   update();
}

void BaseScene3D::keyPressEvent(QKeyEvent* pe)
{
   switch (pe -> key())
   {
         case Qt::Key_Plus:
            scale_plus();     // приблизить сцену
         break;

         case Qt::Key_Equal:
            scale_plus();     // приблизить сцену
         break;

         case Qt::Key_Minus:
            scale_minus();    // удалиться от сцены
         break;

         case Qt::Key_Up:
            rotate_up();      // повернуть сцену вверх
         break;

         case Qt::Key_Down:
            rotate_down();    // повернуть сцену вниз
         break;

         case Qt::Key_Left:
            rotate_left();    // повернуть сцену влево
         break;

         case Qt::Key_Right:
            rotate_right();   // повернуть сцену вправо
         break;

         case Qt::Key_Z:
            rotate_forward(); // повернуть сцену вперёд
         break;

         case Qt::Key_X:
            rotate_backward(); // повернуть сцену назад
         break;

         case Qt::Key_W:
            translate_up();
         break;

         case Qt::Key_S:
            translate_down();
         break;

         case Qt::Key_A:
            translate_left();
         break;

         case Qt::Key_D:
            translate_right();
         break;

         case Qt::Key_Q:
            translate_forward();
         break;

         case Qt::Key_E:
            translate_backward();
         break;

         case Qt::Key_Space:  // клавиша пробела
            defaultScene();   // возвращение значений по умолчанию
         break;

         case Qt::Key_L:
            light=!light;
            if (!light)
                glDisable(GL_LIGHTING);  // Запрет освещения
            else
                glEnable(GL_LIGHTING);
         break;

        case Qt::Key_M:
            fSmooth = !fSmooth;
        break;
   }

   update();
}

void BaseScene3D::scale_plus() // приблизить сцену
{
   nScale = nScale * 1.1f;
}

void BaseScene3D::scale_minus() // удалиться от сцены
{
   nScale = nScale / 1.1f;
}

void BaseScene3D::rotate_up() // повернуть сцену вверх
{
   xRotate += 1.0f;
   if (xRotate > 0.0f)
       xRotate = 0.0f;
}

void BaseScene3D::rotate_down() // повернуть сцену вниз
{
   xRotate -= 1.0f;
   if (xRotate < -180.0f)
       xRotate = -180.0f;
}

void BaseScene3D::rotate_forward() // повернуть сцену вперёд
{
   yRotate += 1.0f;
}

void BaseScene3D::rotate_backward() // повернуть сцену назад
{
   yRotate -= 1.0f;
}

void BaseScene3D::rotate_left() // повернуть сцену влево
{
   zRotate += 1.0f;
}

void BaseScene3D::rotate_right() // повернуть сцену вправо
{
   zRotate -= 1.0f;
}

void BaseScene3D::translate_down() // транслировать сцену вниз
{
    switch (fMoveMode) {
    case mmFirstPersonMode: {
        yTransl += 0.05f;
    }
        break;
    case mmObjectMode:
    default: {
        yTransl -= 0.05f;
    }
        break;
    }
}

void BaseScene3D::translate_up() // транслировать сцену вверх
{
    switch (fMoveMode) {
    case mmFirstPersonMode: {
        yTransl -= 0.05f;
    }
        break;
    case mmObjectMode:
    default: {
        yTransl += 0.05f;
    }
        break;
    }
}

void BaseScene3D::translate_left() // транслировать сцену вниз
{
    switch (fMoveMode) {
    case mmFirstPersonMode: {
        QQuaternion zRot(qCos(qDegreesToRadians(-zRotate) / 2.0), 0, 0, qSin(qDegreesToRadians(-zRotate) / 2.0));
        QQuaternion xRot(qCos(qDegreesToRadians(-xRotate) / 2.0), qSin(qDegreesToRadians(-xRotate) / 2.0),0,0);
        QVector3D p = xRot.rotatedVector(QVector3D(0.0f, 0.0f, zCam)); // this
        p = zRot.rotatedVector(p);
        QQuaternion vRot = QQuaternion::rotationTo(QVector3D(0, 1, 0), QVector3D(1, 0, 0));
        QVector3D pT = vRot.rotatedVector(-p).normalized() * 0.05f;
        xTransl -= pT.x();
        yTransl -= pT.y();
    }
        break;
    case mmObjectMode:
    default: {
        xTransl -= 0.05f;
    }
        break;
    }
}

void BaseScene3D::translate_right() // транслировать сцену вверх
{
    switch (fMoveMode) {
    case mmFirstPersonMode: {
        QQuaternion zRot(qCos(qDegreesToRadians(-zRotate) / 2.0), 0, 0, qSin(qDegreesToRadians(-zRotate) / 2.0));
        QQuaternion xRot(qCos(qDegreesToRadians(-xRotate) / 2.0), qSin(qDegreesToRadians(-xRotate) / 2.0),0,0);
        QVector3D p = xRot.rotatedVector(QVector3D(0.0f, 0.0f, zCam)); // this
        p = zRot.rotatedVector(p);
        QQuaternion vRot = QQuaternion::rotationTo(QVector3D(0, 1, 0), QVector3D(1, 0, 0));
        QVector3D pT = vRot.rotatedVector(-p).normalized() * 0.05f;
        xTransl += pT.x();
        yTransl += pT.y();
    }
        break;
    case mmObjectMode:
    default: {
        xTransl += 0.05f;
    }
        break;
    }
}
void BaseScene3D::translate_forward() // транслировать сцену вниз
{
   zTransl -= 0.05f;
}

void BaseScene3D::translate_backward() // транслировать сцену вверх
{
   zTransl += 0.05f;
}


void BaseScene3D::defaultScene() // наблюдение сцены по умолчанию
{
   xRotate = -45;
   yRotate = 0;
   zRotate = 45;
   zTransl = 0;
   nScale = 1;
   xTransl = 0;
   yTransl = 0;
   zCam = -6;
}

void BaseScene3D::update3DView()
{
    updateXScaleValues();
    updateYScaleValues();
    updateZScaleValues();
    update();
}

void BaseScene3D::drawAxis(const QMatrix4x4 &pmvMatrix) // построить оси координат
{
    if (pManager)
        pManager->drawPrimitives(pmvMatrix);
}

float BaseScene3D::normalizeAngle(float angle) {
    float a = angle;
    if (angle < 0.0f) {
        int m = -angle / 360.0f;
        a = 360.0f + (a + m * 360.0f);
    }
    else {
        int m = angle / 360.0f;
        a = a - m * 360.0f;
    }
    return a;
}

void BaseScene3D::setCamTarget(float x, float y, float z)
{
    xTransl = -x;
    yTransl = -y;
    zTransl = -z;
}

void BaseScene3D::drawScales(const QMatrix4x4 &pvmMatrix)
{
    Q_UNUSED(pvmMatrix);

    bool drawPanels = true;

    float zRot = normalizeAngle(zRotate);
    float xRot = normalizeAngle(xRotate);

    float xMin = qMin(fSpaceData.x, fSpaceData.x + fSpaceData.xLength);
    float xMax = qMax(fSpaceData.x, fSpaceData.x + fSpaceData.xLength);

    float yMin = qMin(fSpaceData.y, fSpaceData.y + fSpaceData.yLength);
    float yMax = qMax(fSpaceData.y, fSpaceData.y + fSpaceData.yLength);

    float zMin = qMin(fSpaceData.z, fSpaceData.z + fSpaceData.zLength);
    float zMax = qMax(fSpaceData.z, fSpaceData.z + fSpaceData.zLength);

    { //drawLines
        glBegin(GL_LINES);
        glLineWidth(1);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

        glColor3f(fGridColor.redF(), fGridColor.greenF(), fGridColor.blueF());

        float xStep = fScalesSettings[slX].step / fScalesSettings[slX].length * fSpaceData.xLength;
        float yStep = fScalesSettings[slY].step / fScalesSettings[slY].length * fSpaceData.yLength;
        float zStep = fScalesSettings[slZ].step / fScalesSettings[slZ].length * fSpaceData.zLength;

        if (fScalesPlaneSettings[spXZ].visible) {
            // along X from Y
            float x = fSpaceData.x;
            while (x <= xMax  + EPSILON) { // vertucal lines
                if (zRot < 90.0f || zRot > 270.0f) {
                    glVertex3f(x, yMax, zMin);
                    glVertex3f(x, yMax, zMax);
                }
                else {
                    glVertex3f(x, yMin, zMin);
                    glVertex3f(x, yMin, zMax);
                }
                x += xStep;
            }

            float z = fSpaceData.z;
            while (z <= zMax  + EPSILON) { // horizontal lines
                if (zRot < 90.0f || zRot > 270.0f) {
                    glVertex3f(xMin, yMax, z);
                    glVertex3f(xMax, yMax, z);
                }
                else {
                    glVertex3f(xMin, yMin, z);
                    glVertex3f(xMax, yMin, z);
                }
                z += zStep;
            }
        }

        if (fScalesPlaneSettings[spYZ].visible) {
            float y = fSpaceData.y;
            while (fScalesSettings[slY].step > 0.0f ? y <= yMax + EPSILON : y >= yMin + EPSILON) { // vertucal lines
                if (zRot < 180.0f) {
                    glVertex3f(xMax, y, zMin);
                    glVertex3f(xMax, y, zMax);
                }
                else {
                    glVertex3f(xMin, y, zMin);
                    glVertex3f(xMin, y, zMax);
                }
                y += yStep;
            }

            float z = fSpaceData.z;
            while (z <= zMax + EPSILON) { // horizontal lines
                if (zRot < 180.0f) {
                    glVertex3f(xMax, yMin, z);
                    glVertex3f(xMax, yMax, z);
                }
                else {
                    glVertex3f(xMin, yMin, z);
                    glVertex3f(xMin, yMax, z);
                }
                z += zStep;
            }
        }

        if (fScalesPlaneSettings[spXY].visible) {
            float y = fSpaceData.y;
            while (fScalesSettings[slY].step > 0.0f ? y <= yMax + EPSILON : y >= yMin + EPSILON) { // vertucal lines
                if (xRot < 90.0f || xRot > 270.0f) {
                    glVertex3f(xMin, y, zMin);
                    glVertex3f(xMax, y, zMin);
                }
                else {
                    glVertex3f(xMin, y, zMax);
                    glVertex3f(xMax, y, zMax);
                }
                y += yStep;
            }

            float x = fSpaceData.x;
            while (x <= xMax + EPSILON) { // horizontal lines
                if (xRot < 90.0f || xRot > 270.0f)  {
                    glVertex3f(x, yMin, zMin);
                    glVertex3f(x, yMax, zMin);
                }
                else {
                    glVertex3f(x, yMin, zMax);
                    glVertex3f(x, yMax, zMax);
                }
                x += xStep;
            }
        }

        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_BLEND);
        glEnd();
    }


    if (drawPanels && xScaleTextureRight) {
        glEnable(GL_TEXTURE_2D);
        glShadeModel(GL_SMOOTH/*GL_FLAT*/);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
        glColor3f(fScaleColor.redF(), fScaleColor.greenF(), fScaleColor.blueF());
        bool viewFromTopToBottom = (xRot == 0.0f || xRot >= 270.0f);
        float delta = 1.0f;

        if (fScalesPlaneSettings[spXZ].visible)  {
            if (zRot < 90.0f) {
                if (viewFromTopToBottom) {
                    if (xScaleTextureRight) {
                        xScaleTextureRight->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(1,0); glVertex3f(xMin, yMax, zMax);
                        glTexCoord2f(0,0); glVertex3f(xMin, yMax, zMax + delta);
                        glTexCoord2f(0,1); glVertex3f(xMax, yMax, zMax + delta);
                        glTexCoord2f(1,1); glVertex3f(xMax, yMax, zMax);
                        glEnd();
                        xScaleTextureRight->release();
                    }
                }
                else {
                    if (xScaleTextureLeft) {
                        xScaleTextureLeft->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(0,0); glVertex3f(xMin, yMax, zMin);
                        glTexCoord2f(1,0); glVertex3f(xMin, yMax, zMin - delta);
                        glTexCoord2f(1,1); glVertex3f(xMax, yMax, zMin - delta);
                        glTexCoord2f(0,1); glVertex3f(xMax, yMax, zMin);
                        glEnd();
                        xScaleTextureLeft->release();
                    }
                }

                if (zScaleTextureRight) {
                    zScaleTextureRight->bind();
                    glBegin(GL_QUADS);
                    glTexCoord2f(1,0); glVertex3f(xMin,         yMax, zMin);
                    glTexCoord2f(0,0); glVertex3f(xMin - delta, yMax, zMin);
                    glTexCoord2f(0,1); glVertex3f(xMin - delta, yMax, zMax);
                    glTexCoord2f(1,1); glVertex3f(xMin,         yMax, zMax);
                    glEnd();
                    zScaleTextureRight->release();
                }
            }
            else if (zRot < 180.0f) {
                if (viewFromTopToBottom) {
                    if (xScaleTextureLeft) {
                        xScaleTextureLeft->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(0,0); glVertex3f(xMin, yMin, zMax);
                        glTexCoord2f(1,0); glVertex3f(xMin, yMin, zMax + delta);
                        glTexCoord2f(1,1); glVertex3f(xMax, yMin, zMax + delta);
                        glTexCoord2f(0,1); glVertex3f(xMax, yMin, zMax);
                        glEnd();
                        xScaleTextureLeft->release();
                    }
                }
                else {
                    if (xScaleTextureRight) {
                        xScaleTextureRight->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(1,0); glVertex3f(xMin, yMin, zMin);
                        glTexCoord2f(0,0); glVertex3f(xMin, yMin, zMin - delta);
                        glTexCoord2f(0,1); glVertex3f(xMax, yMin, zMin - delta);
                        glTexCoord2f(1,1); glVertex3f(xMax, yMin, zMin);
                        glEnd();
                        xScaleTextureRight->release();
                    }
                }

                if (zScaleTextureLeft) {
                    zScaleTextureLeft->bind();
                    glBegin(GL_QUADS);
                    glTexCoord2f(0,0); glVertex3f(xMin,         yMin, zMin);
                    glTexCoord2f(1,0); glVertex3f(xMin - delta, yMin, zMin);
                    glTexCoord2f(1,1); glVertex3f(xMin - delta, yMin, zMax);
                    glTexCoord2f(0,1); glVertex3f(xMin,         yMin, zMax);
                    glEnd();
                    zScaleTextureLeft->release();
                }
            }
            else if (zRot < 270.0f) {
                if (viewFromTopToBottom) {
                    if (xInverseScaleTextureRight) {
                        xInverseScaleTextureRight->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(1,1); glVertex3f(xMin, yMin, zMax);
                        glTexCoord2f(0,1); glVertex3f(xMin, yMin, zMax + delta);
                        glTexCoord2f(0,0); glVertex3f(xMax, yMin, zMax + delta);
                        glTexCoord2f(1,0); glVertex3f(xMax, yMin, zMax);
                        glEnd();
                        xInverseScaleTextureRight->release();
                    }
                }
                else {
                    if (xInverseScaleTextureLeft) {
                        xInverseScaleTextureLeft->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(0,1); glVertex3f(xMin, yMin, zMin);
                        glTexCoord2f(1,1); glVertex3f(xMin, yMin, zMin - delta);
                        glTexCoord2f(1,0); glVertex3f(xMax, yMin, zMin - delta);
                        glTexCoord2f(0,0); glVertex3f(xMax, yMin, zMin);
                        glEnd();
                        xInverseScaleTextureLeft->release();
                    }
                }

                if (zScaleTextureRight) {
                    zScaleTextureRight->bind();
                    glBegin(GL_QUADS);
                    glTexCoord2f(1,0); glVertex3f(xMax,         yMin, zMin);
                    glTexCoord2f(0,0); glVertex3f(xMax + delta, yMin, zMin);
                    glTexCoord2f(0,1); glVertex3f(xMax + delta, yMin, zMax);
                    glTexCoord2f(1,1); glVertex3f(xMax,         yMin, zMax);
                    glEnd();
                    zScaleTextureRight->release();
                }
            }
            else /*if (zRot < 360.0f)*/ {
                if (viewFromTopToBottom) {
                    if (xInverseScaleTextureLeft) {
                        xInverseScaleTextureLeft->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(0,1); glVertex3f(xMin, yMax, zMax);
                        glTexCoord2f(1,1); glVertex3f(xMin, yMax, zMax + delta);
                        glTexCoord2f(1,0); glVertex3f(xMax, yMax, zMax + delta);
                        glTexCoord2f(0,0); glVertex3f(xMax, yMax, zMax);
                        glEnd();
                        xInverseScaleTextureLeft->release();
                    }
                }
                else {
                    if (xInverseScaleTextureRight) {
                        xInverseScaleTextureRight->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(1,1); glVertex3f(xMin, yMax, zMin);
                        glTexCoord2f(0,1); glVertex3f(xMin, yMax, zMin - delta);
                        glTexCoord2f(0,0); glVertex3f(xMax, yMax, zMin - delta);
                        glTexCoord2f(1,0); glVertex3f(xMax, yMax, zMin);
                        glEnd();
                        xInverseScaleTextureRight->release();
                    }
                }

                if (zScaleTextureLeft) {
                    zScaleTextureLeft->bind();
                    glBegin(GL_QUADS);
                    glTexCoord2f(0,0); glVertex3f(xMax,         yMax, zMin);
                    glTexCoord2f(1,0); glVertex3f(xMax + delta, yMax, zMin);
                    glTexCoord2f(1,1); glVertex3f(xMax + delta, yMax, zMax);
                    glTexCoord2f(0,1); glVertex3f(xMax,         yMax, zMax);
                    glEnd();
                    zScaleTextureLeft->release();
                }
            }
        }

        if (fScalesPlaneSettings[spYZ].visible) {
            if (zRot < 90.0f) {
                if (viewFromTopToBottom) {
                    if (yScaleTextureLeft) {
                        yScaleTextureLeft->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(1,0); glVertex3f(xMax, yMin, zMax + delta);
                        glTexCoord2f(0,0); glVertex3f(xMax, yMin, zMax);
                        glTexCoord2f(0,1); glVertex3f(xMax, yMax, zMax);
                        glTexCoord2f(1,1); glVertex3f(xMax, yMax, zMax + delta);
                        glEnd();
                        yScaleTextureLeft->release();
                    }
                }
                else {
                    if (yScaleTextureRight) {
                        yScaleTextureRight->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(0,0); glVertex3f(xMax, yMin, zMin - delta);
                        glTexCoord2f(1,0); glVertex3f(xMax, yMin, zMin);
                        glTexCoord2f(1,1); glVertex3f(xMax, yMax, zMin);
                        glTexCoord2f(0,1); glVertex3f(xMax, yMax, zMin - delta);
                        glEnd();
                        yScaleTextureRight->release();
                    }
                }

                if (zScaleTextureLeft) {
                    zScaleTextureLeft->bind();
                    glBegin(GL_QUADS);
                    glTexCoord2f(0,0); glVertex3f(xMax, yMin,         zMin);
                    glTexCoord2f(1,0); glVertex3f(xMax, yMin - delta, zMin);
                    glTexCoord2f(1,1); glVertex3f(xMax, yMin - delta, zMax);
                    glTexCoord2f(0,1); glVertex3f(xMax, yMin,         zMax);
                    glEnd();
                    zScaleTextureLeft->release();
                }
            }
            else if (zRot < 180.0f) {
                if (viewFromTopToBottom) {
                    if (yInverseScaleTextureRight) {
                        yInverseScaleTextureRight->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(0,1); glVertex3f(xMax, yMin, zMax + delta);
                        glTexCoord2f(1,1); glVertex3f(xMax, yMin, zMax);
                        glTexCoord2f(1,0); glVertex3f(xMax, yMax, zMax);
                        glTexCoord2f(0,0); glVertex3f(xMax, yMax, zMax + delta);
                        glEnd();
                        yInverseScaleTextureRight->release();
                    }
                }
                else {
                    if (yInverseScaleTextureLeft) {
                        yInverseScaleTextureLeft->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(1,1); glVertex3f(xMax, yMin, zMin - delta);
                        glTexCoord2f(0,1); glVertex3f(xMax, yMin, zMin);
                        glTexCoord2f(0,0); glVertex3f(xMax, yMax, zMin);
                        glTexCoord2f(1,0); glVertex3f(xMax, yMax, zMin - delta);
                        glEnd();
                        yInverseScaleTextureLeft->release();
                    }
                }

                if (zScaleTextureRight) {
                    zScaleTextureRight->bind();
                    glBegin(GL_QUADS);
                    glTexCoord2f(1,0); glVertex3f(xMax, yMax,         zMin);
                    glTexCoord2f(0,0); glVertex3f(xMax, yMax + delta, zMin);
                    glTexCoord2f(0,1); glVertex3f(xMax, yMax + delta, zMax);
                    glTexCoord2f(1,1); glVertex3f(xMax, yMax,         zMax);
                    glEnd();
                    zScaleTextureRight->release();
                }
            }
            else if (zRot < 270.0f) {
                if (viewFromTopToBottom) {
                    if (yInverseScaleTextureLeft) {
                        yInverseScaleTextureLeft->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(1,1); glVertex3f(xMin, yMin, zMax + delta);
                        glTexCoord2f(0,1); glVertex3f(xMin, yMin, zMax);
                        glTexCoord2f(0,0); glVertex3f(xMin, yMax, zMax);
                        glTexCoord2f(1,0); glVertex3f(xMin, yMax, zMax + delta);
                        glEnd();
                        yInverseScaleTextureLeft->release();
                    }
                }
                else {
                    if (yInverseScaleTextureRight) {
                        yInverseScaleTextureRight->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(0,1); glVertex3f(xMin, yMin, zMin - delta);
                        glTexCoord2f(1,1); glVertex3f(xMin, yMin, zMin);
                        glTexCoord2f(1,0); glVertex3f(xMin, yMax, zMin);
                        glTexCoord2f(0,0); glVertex3f(xMin, yMax, zMin - delta);
                        glEnd();
                        yInverseScaleTextureRight->release();
                    }
                }

                if (zScaleTextureLeft) {
                    zScaleTextureLeft->bind();
                    glBegin(GL_QUADS);
                    glTexCoord2f(0,0); glVertex3f(xMin, yMax,         zMin);
                    glTexCoord2f(1,0); glVertex3f(xMin, yMax + delta, zMin);
                    glTexCoord2f(1,1); glVertex3f(xMin, yMax + delta, zMax);
                    glTexCoord2f(0,1); glVertex3f(xMin, yMax,         zMax);
                    glEnd();
                    zScaleTextureLeft->release();
                }
            }
            else {
                if (viewFromTopToBottom) {
                    if (yScaleTextureRight) {
                        yScaleTextureRight->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(0,0); glVertex3f(xMin, yMin, zMax + delta);
                        glTexCoord2f(1,0); glVertex3f(xMin, yMin, zMax);
                        glTexCoord2f(1,1); glVertex3f(xMin, yMax, zMax);
                        glTexCoord2f(0,1); glVertex3f(xMin, yMax, zMax + delta);
                        glEnd();
                        yScaleTextureRight->release();
                    }
                }
                else {
                    if (yScaleTextureLeft) {
                        yScaleTextureLeft->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(1,0); glVertex3f(xMin, yMin, zMin - delta);
                        glTexCoord2f(0,0); glVertex3f(xMin, yMin, zMin);
                        glTexCoord2f(0,1); glVertex3f(xMin, yMax, zMin);
                        glTexCoord2f(1,1); glVertex3f(xMin, yMax, zMin - delta);
                        glEnd();
                        yScaleTextureLeft->release();
                    }
                }

                if (zScaleTextureRight) {
                    zScaleTextureRight->bind();
                    glBegin(GL_QUADS);
                    glTexCoord2f(1,0); glVertex3f(xMin, yMin,         zMin);
                    glTexCoord2f(0,0); glVertex3f(xMin, yMin - delta, zMin);
                    glTexCoord2f(0,1); glVertex3f(xMin, yMin - delta, zMax);
                    glTexCoord2f(1,1); glVertex3f(xMin, yMin,         zMax);
                    glEnd();
                    zScaleTextureRight->release();
                }
            }
        }

        if (fScalesPlaneSettings[spXY].visible) {
            if (zRot < 90.0f) {
                if (viewFromTopToBottom) {
                    if (yScaleTextureRight) {
                        yScaleTextureRight->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(0,0); glVertex3f(xMin - delta, yMin, zMin);
                        glTexCoord2f(1,0); glVertex3f(xMin,         yMin, zMin);
                        glTexCoord2f(1,1); glVertex3f(xMin,         yMax, zMin);
                        glTexCoord2f(0,1); glVertex3f(xMin - delta, yMax, zMin);
                        glEnd();
                        yScaleTextureRight->release();
                    }
                    if (xScaleTextureLeft) {
                        xScaleTextureLeft->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(0,0); glVertex3f(xMin, yMin,         zMin);
                        glTexCoord2f(1,0); glVertex3f(xMin, yMin - delta, zMin);
                        glTexCoord2f(1,1); glVertex3f(xMax, yMin - delta, zMin);
                        glTexCoord2f(0,1); glVertex3f(xMax, yMin,         zMin);
                        glEnd();
                        xScaleTextureLeft->release();
                    }
                }
                else {
                    if (yInverseScaleTextureRight) {
                        yInverseScaleTextureRight->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(0,1); glVertex3f(xMin - delta, yMin, zMax);
                        glTexCoord2f(1,1); glVertex3f(xMin,         yMin, zMax);
                        glTexCoord2f(1,0); glVertex3f(xMin,         yMax, zMax);
                        glTexCoord2f(0,0); glVertex3f(xMin - delta, yMax, zMax);
                        glEnd();
                        yInverseScaleTextureRight->release();
                    }
                    if (xInverseScaleTextureLeft) {
                        xInverseScaleTextureLeft->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(0,1); glVertex3f(xMin, yMin,         zMax);
                        glTexCoord2f(1,1); glVertex3f(xMin, yMin - delta, zMax);
                        glTexCoord2f(1,0); glVertex3f(xMax, yMin - delta, zMax);
                        glTexCoord2f(0,0); glVertex3f(xMax, yMin,         zMax);
                        glEnd();
                        xInverseScaleTextureLeft->release();
                    }
                }
            }
            else if (zRot < 180.0f) {
                if (viewFromTopToBottom) {
                    if (yInverseScaleTextureLeft) {
                        yInverseScaleTextureLeft->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(1,1); glVertex3f(xMin - delta, yMin, zMin);
                        glTexCoord2f(0,1); glVertex3f(xMin,         yMin, zMin);
                        glTexCoord2f(0,0); glVertex3f(xMin,         yMax, zMin);
                        glTexCoord2f(1,0); glVertex3f(xMin - delta, yMax, zMin);
                        glEnd();
                        yInverseScaleTextureLeft->release();
                    }

                    if (xScaleTextureRight) {
                        xScaleTextureRight->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(1,0); glVertex3f(xMin, yMax,         zMin);
                        glTexCoord2f(0,0); glVertex3f(xMin, yMax + delta, zMin);
                        glTexCoord2f(0,1); glVertex3f(xMax, yMax + delta, zMin);
                        glTexCoord2f(1,1); glVertex3f(xMax, yMax,         zMin);
                        glEnd();
                        xScaleTextureRight->release();
                    }
                }
                else {
                    if (yScaleTextureLeft) {
                        yScaleTextureLeft->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(1,0); glVertex3f(xMin - delta, yMin, zMax);
                        glTexCoord2f(0,0); glVertex3f(xMin,         yMin, zMax);
                        glTexCoord2f(0,1); glVertex3f(xMin,         yMax, zMax);
                        glTexCoord2f(1,1); glVertex3f(xMin - delta, yMax, zMax);
                        glEnd();
                        yScaleTextureLeft->release();
                    }
                    if (xInverseScaleTextureRight) {
                        xInverseScaleTextureRight->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(1,1); glVertex3f(xMin, yMax,         zMax);
                        glTexCoord2f(0,1); glVertex3f(xMin, yMax + delta, zMax);
                        glTexCoord2f(0,0); glVertex3f(xMax, yMax + delta, zMax);
                        glTexCoord2f(1,0); glVertex3f(xMax, yMax,         zMax);
                        glEnd();
                        xInverseScaleTextureRight->release();
                    }
                }
            }
            else if (zRot < 270.0f) {
                if (viewFromTopToBottom) {
                    if (yInverseScaleTextureRight) {
                        yInverseScaleTextureRight->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(0,1); glVertex3f(xMax + delta, yMin, zMin);
                        glTexCoord2f(1,1); glVertex3f(xMax,         yMin, zMin);
                        glTexCoord2f(1,0); glVertex3f(xMax,         yMax, zMin);
                        glTexCoord2f(0,0); glVertex3f(xMax + delta, yMax, zMin);
                        glEnd();
                        yInverseScaleTextureRight->release();
                    }
                    if (xInverseScaleTextureLeft) {
                        xInverseScaleTextureLeft->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(0,1); glVertex3f(xMin, yMax,         zMin);
                        glTexCoord2f(1,1); glVertex3f(xMin, yMax + delta, zMin);
                        glTexCoord2f(1,0); glVertex3f(xMax, yMax + delta, zMin);
                        glTexCoord2f(0,0); glVertex3f(xMax, yMax,         zMin);
                        glEnd();
                        xInverseScaleTextureLeft->release();
                    }
                }
                else {
                    if (yScaleTextureRight) {
                        yScaleTextureRight->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(0,0); glVertex3f(xMax + delta, yMin, zMax);
                        glTexCoord2f(1,0); glVertex3f(xMax,         yMin, zMax);
                        glTexCoord2f(1,1); glVertex3f(xMax,         yMax, zMax);
                        glTexCoord2f(0,1); glVertex3f(xMax + delta, yMax, zMax);
                        glEnd();
                        yScaleTextureRight->release();
                    }
                    if (xScaleTextureLeft) {
                        xScaleTextureLeft->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(0,0); glVertex3f(xMin, yMax,         zMax);
                        glTexCoord2f(1,0); glVertex3f(xMin, yMax + delta, zMax);
                        glTexCoord2f(1,1); glVertex3f(xMax, yMax + delta, zMax);
                        glTexCoord2f(0,1); glVertex3f(xMax, yMax,         zMax);
                        glEnd();
                        xScaleTextureLeft->release();
                    }
                }
            }
            else {
                if (viewFromTopToBottom) {
                    if (yScaleTextureLeft) {
                        yScaleTextureLeft->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(1,0); glVertex3f(xMax + delta, yMin, zMin);
                        glTexCoord2f(0,0); glVertex3f(xMax,         yMin, zMin);
                        glTexCoord2f(0,1); glVertex3f(xMax,         yMax, zMin);
                        glTexCoord2f(1,1); glVertex3f(xMax + delta, yMax, zMin);
                        glEnd();
                        yScaleTextureLeft->release();
                    }
                    if (xInverseScaleTextureRight) {
                        xInverseScaleTextureRight->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(1,1); glVertex3f(xMin, yMin,         zMin);
                        glTexCoord2f(0,1); glVertex3f(xMin, yMin - delta, zMin);
                        glTexCoord2f(0,0); glVertex3f(xMax, yMin - delta, zMin);
                        glTexCoord2f(1,0); glVertex3f(xMax, yMin,         zMin);
                        glEnd();
                        xInverseScaleTextureRight->release();
                    }
                }
                else {
                    if (yInverseScaleTextureLeft) {
                        yInverseScaleTextureLeft->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(1,1); glVertex3f(xMax + delta, yMin, zMax);
                        glTexCoord2f(0,1); glVertex3f(xMax,         yMin, zMax);
                        glTexCoord2f(0,0); glVertex3f(xMax,         yMax, zMax);
                        glTexCoord2f(1,0); glVertex3f(xMax + delta, yMax, zMax);
                        glEnd();
                        yInverseScaleTextureLeft->release();
                    }
                    if (xScaleTextureRight) {
                        xScaleTextureRight->bind();
                        glBegin(GL_QUADS);
                        glTexCoord2f(1,0); glVertex3f(xMin, yMin,         zMax);
                        glTexCoord2f(0,0); glVertex3f(xMin, yMin - delta, zMax);
                        glTexCoord2f(0,1); glVertex3f(xMax, yMin - delta, zMax);
                        glTexCoord2f(1,1); glVertex3f(xMax, yMin,         zMax);
                        glEnd();
                        xScaleTextureRight->release();
                    }
                }
            }
        }

        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);
    }
}

void BaseScene3D::prepareView()
{
    glMatrixMode(GL_PROJECTION);                     // устанавливает текущей проекционную матрицу
    glLoadIdentity();                                // присваивает проекционной матрице единичную матрицу
    GLfloat aspect = (GLfloat)width() / (GLfloat)height();

    // поле просмотра
    glViewport(0, 0, (GLint)width(), (GLint)height());
    qgluPerspective(60.0, aspect, 1.0, 250.0);

    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // очистка буфера изображения текущим цветом очистки и глубины

    glMatrixMode(GL_MODELVIEW);                         // устанавливает положение и ориентацию матрице моделирования
    setWorldTransform();
}

void BaseScene3D::setWorldTransform()
{
    glLoadIdentity();                                   // загружает единичную матрицу моделирования

    // последовательные преобразования
    glTranslatef(0.0f, 0.0f, zCam);

    glRotatef(xRotate, 1.0f, 0.0f, 0.0f);            // поворот вокруг оси X
    glRotatef(yRotate, 0.0f, 1.0f, 0.0f);            // поворот вокруг оси Y
    glRotatef(zRotate, 0.0f, 0.0f, 1.0f);            // поворот вокруг оси Z

    glTranslatef(xTransl, yTransl, zTransl);         // трансляция
    glScalef(nScale, nScale, nScale);
}

void BaseScene3D::setInverseWorldTransform()
{
    glRotatef(-zRotate, 0.0f, 0.0f, 1.0f);            // поворот вокруг оси Z
    glRotatef(-yRotate, 0.0f, 1.0f, 0.0f);            // поворот вокруг оси Y
    glRotatef(-xRotate, 1.0f, 0.0f, 0.0f);            // поворот вокруг оси X
}

static void __gluMultMatrixVecd(const GLdouble matrix[16], const GLdouble in[4],
              GLdouble out[4])
{
    int i;
    for (i=0; i<4; i++) {
    out[i] =
        in[0] * matrix[0*4+i] +
        in[1] * matrix[1*4+i] +
        in[2] * matrix[2*4+i] +
        in[3] * matrix[3*4+i];
    }
}

int gluProject(GLdouble objx, GLdouble objy, GLdouble objz,
          const GLdouble modelMatrix[16],
          const GLdouble projMatrix[16],
              const GLint viewport[4],
          GLdouble *winx, GLdouble *winy, GLdouble *winz)
{
    double in[4];
    double out[4];
    in[0]=objx;
    in[1]=objy;
    in[2]=objz;
    in[3]=1.0;
    __gluMultMatrixVecd(modelMatrix, in, out);
    __gluMultMatrixVecd(projMatrix, out, in);
    if (in[3] == 0.0) return(GL_FALSE);
    in[0] /= in[3];
    in[1] /= in[3];
    in[2] /= in[3];
    /* Map x, y and z to range 0-1 */
    in[0] = in[0] * 0.5 + 0.5;
    in[1] = in[1] * 0.5 + 0.5;
    in[2] = in[2] * 0.5 + 0.5;
    /* Map x,y to viewport */
    in[0] = in[0] * viewport[2] + viewport[0];
    in[1] = in[1] * viewport[3] + viewport[1];
    *winx=in[0];
    *winy=in[1];
    *winz=in[2];
    return(GL_TRUE);
}

void BaseScene3D::renderText(float x, float y, float z, QString text, QFont &font, QColor color, Qt::Alignment textAlignment, bool scaled)
{
    Q_UNUSED(textAlignment);
    GLint viewport[4];
    GLdouble mvmatrix[16], projmatrix[16];
    GLdouble wx,wy,wz;
    glGetIntegerv(GL_VIEWPORT,viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX,mvmatrix);
    glGetDoublev(GL_PROJECTION_MATRIX,projmatrix);
    gluProject (x, y, z, mvmatrix, projmatrix, viewport, &wx, &wy, &wz);
    wy=viewport[3]-wy;

    glDisable(GL_DEPTH_TEST);

    if (scaled) {
        QPainter painter(this);
        painter.beginNativePainting();
        glPushMatrix();
        glTranslatef(x, y, z);
        setInverseWorldTransform();

        glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        font.setStyleStrategy(QFont::PreferBitmap);
        QPainterPath path;
        path.addText(QPointF(0, 0), font, text);
        QList<QPolygonF> poly = path.toSubpathPolygons();
        for (QList<QPolygonF>::iterator i = poly.begin(); i != poly.end(); ++i){
            glBegin(GL_LINE_LOOP);
            for (QPolygonF::iterator p = (*i).begin(); p != i->end(); ++p)
                glVertex3f(p->rx()*0.1f, -p->ry()*0.1f, 0);
            glEnd();
        }

        glPopMatrix();
        painter.endNativePainting();
    }
    else {
        QPainter painter(this);
        painter.setPen(color);
        painter.setFont(font);
        int h = painter.fontMetrics().height();
        int w = painter.fontMetrics().width(text);
        QImage image(w,h,QImage::Format_ARGB32);
        {
            image.fill(QColor::fromRgbF(0.00f, 0.00f, 0.00f, 0.0f));
            QPainter pi(&image);
            pi.setPen(color);
            pi.setFont(font);
            pi.drawText(0,h,text);
        }
        painter.drawImage(wx,wy,image);
        painter.end();
    }
    glEnable(GL_DEPTH_TEST);
}

void BaseScene3D::contextMenuEvent(QContextMenuEvent *event)
{
    if (rBut && !isMouseMove) {
        BaseSettingsWin * w = nullptr;
        for (int i = 0; i < children().count(); i++) {
            w = qobject_cast<BaseSettingsWin*>(children()[i]);
            if (w)
                break;
        }
        if (!w) {
            createViewSettings();
            w = new BaseSettingsWin(&fSettings, this);

            QRect r = w->geometry();
            r.moveTopLeft(event->pos());
            connect(w, SIGNAL(settingsChanged(QString,QVariant)), this, SLOT(update3DView()));
            connect(w, SIGNAL(settingsChanged(QString,QVariant)), this, SIGNAL(settingsChanged(QString,QVariant)));
            w->show();
        }

        w->activateWindow();
    }
}

void BaseScene3D::createViewSettings()
{
    fSettings.clear();

    BaseSettings scalesGroup("scales", "Scales");
    scalesGroup.addSettingsItem("Font", &fScaleFont);
    scalesGroup.addSettingsItem("Font color", &fScaleColor);
    scalesGroup.addSettingsItem("Grid color", &fGridColor);
    scalesGroup.addSettingsItem("Plane XY visible", &fScalesPlaneSettings[spXY].visible);
    scalesGroup.addSettingsItem("Plane XZ visible", &fScalesPlaneSettings[spXZ].visible);
    scalesGroup.addSettingsItem("Plane YZ visible", &fScalesPlaneSettings[spYZ].visible);

    BaseSettings xScale("xScale", "Scale X");
    xScale.addSettingsItem("Value start@min=-100;max=100;step=0.5;digits=2", &fScalesSettings[slX].start);
    xScale.addSettingsItem("Value length@min=-100.0;max=100.0;step=0.5;digits=2", &fScalesSettings[slX].length);
    xScale.addSettingsItem("Value step@min=-5;max=5;step=0.01;digits=2", &fScalesSettings[slX].step);

    BaseSettings yScale("yScale", "Scale Y");
    yScale.addSettingsItem("Value start@min=-100.0;max=100;step=0.5;digits=2", &fScalesSettings[slY].start);
    yScale.addSettingsItem("Value length@min=-100.0;max=100;step=0.5;digits=2", &fScalesSettings[slY].length);
    yScale.addSettingsItem("Value step@min=-5;max=5;step=0.01;digits=2", &fScalesSettings[slY].step);

    BaseSettings zScale("zScale", "Scale Z");
    zScale.addSettingsItem("Value start@min=-100;max=100;step=0.5;digits=2", &fScalesSettings[slZ].start);
    zScale.addSettingsItem("Value length@min=0.0;max=100;step=0.5;digits=2", &fScalesSettings[slZ].length);
    zScale.addSettingsItem("Value step@min=0.01;max=2;step=0.01;digits=2", &fScalesSettings[slZ].step);

    scalesGroup.addSettingsItem(xScale);
    scalesGroup.addSettingsItem(yScale);
    scalesGroup.addSettingsItem(zScale);

    fSettings.addSettingsItem(scalesGroup);
}

BaseSettingsWin::BaseSettingsWin(BaseSettings *settings, QWidget *parent) :
    QMainWindow(parent, Qt::Tool)
{
    ui.setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(settings->title());

    for (int i = 0; i < settings->getSettingsCount(); i++) {
        BaseSettings *s = settings->getSettings(i);
        QTreeWidgetItem *topItem = createTopItem(s);
        ui.categoryTreeWidget->addTopLevelItem(topItem);
    }

    ui.settingsWidget->setCurrentIndex(0);
    ui.categoryTreeWidget->expandAll();

    connect(ui.categoryTreeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
}

QTreeWidgetItem *BaseSettingsWin::createTopItem(BaseSettings *settings)
{
    QTreeWidgetItem *topItem = new QTreeWidgetItem();
    topItem->setText(0, settings->title());
    topItem->setData(0, Qt::UserRole, settings->name());

    for (int i = 0; i < settings->getSettingsCount(); i++) {
        BaseSettings *s = settings->getSettings(i);
        QTreeWidgetItem *childItem = createTopItem(s);
        topItem->addChild(childItem);
    }

    if (settings->getItemCount()) {
        QWidget *w = createSettingsWidget(settings);
        ui.settingsWidget->addWidget(w);
        fStackWidgetList.append(w);
    }

    return topItem;
}

QWidget *BaseSettingsWin::createSettingsWidget(BaseSettings *settings)
{
    QFrame *w = nullptr;
    for (int i = 0; i < settings->getItemCount(); i++) {
        QPair<QString,BaseSettings::SettingsItem> item = settings->getItem(i);
        if (!w) {
            w = new QFrame(this);
            w->setObjectName(settings->name());
            w->setFrameShape(QFrame::StyledPanel);
            QVBoxLayout *vl = new QVBoxLayout;
            w->setLayout(vl);
            w->setVisible(true);
        }
        QHBoxLayout *hl = new QHBoxLayout;
        hl->addWidget(new QLabel(item.first.left(item.first.indexOf("@")) + ":"));
        VariantEditor *editor = VariantEditor::createEditor(settings->name() + "#" + item.first, item.second.value, item.second.source);
        connect(editor, SIGNAL(valueChanged(QString,QVariant)), this, SIGNAL(settingsChanged(QString,QVariant)));
        hl->addWidget(editor);
        hl->addStretch();
        ((QVBoxLayout*)w->layout())->addLayout(hl);
    }

    ((QVBoxLayout*)w->layout())->addStretch();
    return w;
}

void BaseSettingsWin::currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous);
    if (!current) return;

    QString name = current->data(0, Qt::UserRole).toString();

    foreach (QWidget *w, fStackWidgetList) {
        if (name == w->objectName()) {
            ui.settingsWidget->setCurrentWidget(w);
            return;
        }
    }
}



void BaseSettings::clear()
{
    fItems.clear();
    fChildSettings.clear();
}

void BaseSettings::addSettingsItem(QString name, float *value)
{
    QVariant v = QVariant::fromValue(*value);
    addSettingsItem(name, v, value);
}

void BaseSettings::addSettingsItem(QString name, double *value)
{
    QVariant v = QVariant::fromValue(*value);
    addSettingsItem(name, v, value);
}

void BaseSettings::addSettingsItem(QString name, int *value)
{
    QVariant v = QVariant::fromValue(*value);
    addSettingsItem(name, v, value);
}

void BaseSettings::addSettingsItem(QString name, bool *value)
{
    QVariant v = QVariant::fromValue(*value);
    addSettingsItem(name, v, value);
}

void BaseSettings::addSettingsItem(QString name, QFont *value)
{
    QVariant v = QVariant::fromValue(*value);
    addSettingsItem(name, v, value);
}

void BaseSettings::addSettingsItem(QString name, QColor *value)
{
    QVariant v = QVariant::fromValue(*value);
    addSettingsItem(name, v, value);
}

void BaseSettings::addSettingsItem(QString name, QVariant value, void *source)
{
    SettingsItem item;
    item.value = value;
    item.source = source;
    fItems.insert(name, item);
}

