#include "gl_primitives.h"
#include <QtMath>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>

Primitive::~Primitive()
{
    if (points)
        delete points;
    if (surfaceIndexes)
        delete surfaceIndexes;
    if (wireFrameIndexes)
        delete wireFrameIndexes;
}

void Primitive::draw()
{
    glVertexPointer(3, GL_FLOAT, 0, points);
    glEnableClientState(GL_VERTEX_ARRAY);

    switch (drawType()) {
    case dtSurface: {
        glDrawElements(GL_TRIANGLES, surfaceIndexCount, GL_UNSIGNED_INT, surfaceIndexes);
    }
        break;
    case dtTriangleWireFrame: {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, surfaceIndexCount, GL_UNSIGNED_INT, surfaceIndexes);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
        break;
    case dtWireFrame: {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glDrawElements(GL_LINES, wireFrameIndexCount, GL_UNSIGNED_INT, wireFrameIndexes);
        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_BLEND);
    }
        break;
    default: {
        glPointSize(3);
        glEnable(GL_POINT_SMOOTH);
        glDrawArrays(GL_POINTS, 0, pointCount);
        glDisable(GL_POINT_SMOOTH);
    }
        break;
    }
    glDisableClientState(GL_VERTEX_ARRAY);
}

void Primitive::createVertexBuffer()
{
    if (!points)
        return;

    if (vertexBuffer.isCreated())
        vertexBuffer.destroy();

    vertexBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    if (!vertexBuffer.create())
        qDebug() << "Cannot create 1 QOpenGLBuffer!";

    vertexBuffer.bind();
    vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexBuffer.allocate(points, 3 * sizeof(GLfloat) * pointCount);
    vertexBuffer.release();
}

void Primitive::updateMatrix()
{
    trMatrix = QMatrix4x4();
    QQuaternion q = QQuaternion::rotationTo(QVector3D(0,0,1), pDirection);
    trMatrix.translate(pPosition);
    trMatrix.rotate(q);
}

PrimitiveSphere::PrimitiveSphere(const int segments, const float radiusX, const float radiusY, const float radiusZ, QVector3D direction) : Primitive(direction)
{
    pointCount = segments * (segments - 1) + 2;
    points = new GLfloat[pointCount * 3];
    const float fiStep = 3.1415926f / segments;
    const float feStep = 2.0f * 3.1415926f / segments;
    float fi = fiStep;
    float fe = 0;
    GLfloat3 *buf = reinterpret_cast<GLfloat3*>(points);
    buf->x = 0;
    buf->y = 0;
    buf->z = radiusZ;
    buf++;
    buf->x = 0;
    buf->y = 0;
    buf->z = -radiusZ;
    buf++;

    for (int i = 1; i < segments; i++) {
        for (int j = 0; j < segments; j++) {
            buf->x = radiusX * qSin(fi) * qCos(fe);
            buf->y = radiusY * qSin(fi) * qSin(fe);
            buf->z = radiusZ * qCos(fi);
            buf++;
            fe += feStep;
        }
        fi += fiStep;
        fe = 0;
    }

    wireFrameIndexCount = (segments * segments + segments * (segments - 1)) * 2;
    wireFrameIndexes = new GLuint[wireFrameIndexCount];

    GLuint *wfIBuf = wireFrameIndexes;
    // top and bottom cap
    for (int si = 0; si < segments; si++) {
        wfIBuf[0] = 0;
        wfIBuf[1] = 2 + si;
        wfIBuf += 2;
        wfIBuf[0] = 1;
        wfIBuf[1] = 2 + segments * (segments - 2) + si;
        wfIBuf += 2;
    }

    // body
    for (int vi = 0; vi < segments - 1; vi++)
        for (int hi = 0; hi < segments; hi++) {
            // vertical line
            if (vi + 1 < segments - 1) {
                wfIBuf[0] = 2 + vi * segments + hi;
                wfIBuf[1] = 2 + (vi + 1) * segments + hi;
                wfIBuf += 2;
            }
            // horizontal line
            wfIBuf[0] = 2 + vi * segments + hi;
            wfIBuf[1] = 2 + vi * segments + (hi + 1 >= segments ? 0 : hi + 1);
            wfIBuf += 2;
        }


    surfaceIndexCount = segments * 2 * 3 + segments * 2 * 3 * (segments - 2);
    surfaceIndexes = new GLuint[surfaceIndexCount];
    wfIBuf = surfaceIndexes;
    // top and bottom cap
    for (int si = 0; si < segments; si++) {
        wfIBuf[0] = 0;
        wfIBuf[1] = 2 + si;
        wfIBuf[2] = 2 + (si + 1 >= segments ? 0 : si + 1);
        wfIBuf += 3;
        wfIBuf[1] = 1;
        wfIBuf[0] = 2 + segments * (segments - 2) + si;
        wfIBuf[2] = 2 + segments * (segments - 2) + (si + 1 >= segments ? 0 : si + 1);
        wfIBuf += 3;
    }

    // body
    for (int vi = 0; vi < segments - 1; vi++)
        for (int hi = 0; hi < segments; hi++) {
            if (vi + 1 < segments - 1) {
                wfIBuf[0] = 2 + vi * segments + hi;
                wfIBuf[1] = 2 + vi * segments + (hi + 1 >= segments ? 0 : hi + 1);
                wfIBuf[2] = 2 + (vi + 1) * segments + (hi + 1 >= segments ? 0 : hi + 1);
                wfIBuf += 3;

                wfIBuf[0] = 2 + vi * segments + hi;
                wfIBuf[1] = 2 + (vi + 1) * segments + (hi + 1 >= segments ? 0 : hi + 1);
                wfIBuf[2] = 2 + (vi + 1) * segments + hi;
                wfIBuf += 3;
            }
        }

    updateMatrix();
}

PrimitiveCone::PrimitiveCone(const int segments, const float height, const float radius, QVector3D direction) : Primitive(direction)
{
    pointCount = segments + 2;
    points = new GLfloat[pointCount * 3];
    const float fiStep = 2.0f * 3.1415926f / segments;
    float fi = 0;
    GLfloat3 *buf = (GLfloat3 *)points;
    buf->x = 0;
    buf->y = 0;
    buf->z = height;
    buf++;
    buf->x = 0;
    buf->y = 0;
    buf->z = 0;
    buf++;

    for (int i = 0; i < segments; i++) {
        buf->x = radius * qCos(fi);
        buf->y = radius * qSin(fi);
        buf->z = 0;
        buf++;
        fi += fiStep;
    }

    wireFrameIndexCount = segments * 3 * 2;
    wireFrameIndexes = new GLuint[wireFrameIndexCount];

    GLuint *wfIBuf = wireFrameIndexes;
    // top and bottom cap
    for (int si = 0; si < segments; si++) {
        wfIBuf[0] = 0;
        wfIBuf[1] = 2 + si;
        wfIBuf += 2;
        wfIBuf[0] = 1;
        wfIBuf[1] = 2 + si;
        wfIBuf += 2;
        wfIBuf[0] = 2 + si;
        wfIBuf[1] = 2 + (si + 1 >= segments ? 0 : si + 1);
        wfIBuf += 2;
    }


    surfaceIndexCount = segments * 2 * 3;
    surfaceIndexes = new GLuint[surfaceIndexCount];
    wfIBuf = surfaceIndexes;
    // top and bottom cap
    for (int si = 0; si < segments; si++) {
        wfIBuf[0] = 0;
        wfIBuf[1] = 2 + si;
        wfIBuf[2] = 2 + (si + 1 >= segments ? 0 : si + 1);
        wfIBuf += 3;
        wfIBuf[1] = 1;
        wfIBuf[0] = 2 + si;
        wfIBuf[2] = 2 + (si + 1 >= segments ? 0 : si + 1);
        wfIBuf += 3;
    }
    updateMatrix();
}

PrimitiveCylinder::PrimitiveCylinder(const int segments, const float height, const float radius, QVector3D direction) : Primitive(direction)
{
    pointCount = segments * 2 + 2;
    points = new GLfloat[pointCount * 3];
    const float fiStep = 2.0f * 3.1415926f / segments;
    float fi = 0;
    GLfloat3 *buf = (GLfloat3 *)points;
    buf->x = 0;
    buf->y = 0;
    buf->z = height;
    buf++;
    buf->x = 0;
    buf->y = 0;
    buf->z = 0;
    buf++;

    for (int i = 0; i < segments; i++) {
        buf->x = radius * qCos(fi);
        buf->y = radius * qSin(fi);
        buf->z = height;
        buf++;
        fi += fiStep;
    }

    fi = 0;
    for (int i = 0; i < segments; i++) {
        buf->x = radius * qCos(fi);
        buf->y = radius * qSin(fi);
        buf->z = 0;
        buf++;
        fi += fiStep;
    }

    wireFrameIndexCount = segments * 5 * 2;
    wireFrameIndexes = new GLuint[wireFrameIndexCount];

    GLuint *wfIBuf = wireFrameIndexes;
    // top and bottom cap
    for (int si = 0; si < segments; si++) {
        wfIBuf[0] = 0;
        wfIBuf[1] = 2 + si;
        wfIBuf += 2;

        wfIBuf[0] = 1;
        wfIBuf[1] = 2 + segments + si;
        wfIBuf += 2;

        wfIBuf[0] = 2 + si;
        wfIBuf[1] = 2 + segments + si;
        wfIBuf += 2;

        wfIBuf[0] = 2 + si;
        wfIBuf[1] = 2 + (si + 1 >= segments ? 0 : si + 1);
        wfIBuf += 2;

        wfIBuf[0] = 2 + segments + si;
        wfIBuf[1] = 2 + segments + (si + 1 >= segments ? 0 : si + 1);
        wfIBuf += 2;
    }

    surfaceIndexCount = segments * 4 * 3;
    surfaceIndexes = new GLuint[surfaceIndexCount];
    wfIBuf = surfaceIndexes;
    // top and bottom cap
    for (int si = 0; si < segments; si++) {
        wfIBuf[0] = 0;
        wfIBuf[1] = 2 + si;
        wfIBuf[2] = 2 + (si + 1 >= segments ? 0 : si + 1);
        wfIBuf += 3;

        wfIBuf[1] = 1;
        wfIBuf[0] = 2 + segments + si;
        wfIBuf[2] = 2 + segments + (si + 1 >= segments ? 0 : si + 1);
        wfIBuf += 3;

        wfIBuf[0] = 2 + si;
        wfIBuf[1] = 2 + (si + 1 >= segments ? 0 : si + 1);
        wfIBuf[2] = 2 + segments + (si + 1 >= segments ? 0 : si + 1);
        wfIBuf += 3;

        wfIBuf[0] = 2 + si;
        wfIBuf[1] = 2 + segments + (si + 1 >= segments ? 0 : si + 1);
        wfIBuf[2] = 2 + segments + si;
        wfIBuf += 3;
    }
    updateMatrix();
}


PrimitiveSimpleArrow::PrimitiveSimpleArrow(const int segments, const float height, const float arrowHeight, const float radius, QVector3D direction) : Primitive(direction)
{
    fArrowLenth = height;
    pointCount = segments + 2 + 1;
    points = new GLfloat[pointCount * 3];
    const float fiStep = 2.0f * 3.1415926f / segments;
    float fi = 0;
    GLfloat3 *buf = (GLfloat3 *)points;
    buf->x = 0;
    buf->y = 0;
    buf->z = height;
    buf++;
    buf->x = 0;
    buf->y = 0;
    buf->z = height - arrowHeight;
    buf++;

    for (int i = 0; i < segments; i++) {
        buf->x = radius * qCos(fi);
        buf->y = radius * qSin(fi);
        buf->z = height - arrowHeight;
        buf++;
        fi += fiStep;
    }

    buf->x = 0;
    buf->y = 0;
    buf->z = 0;

    wireFrameIndexCount = segments * 3 * 2 + 2;
    wireFrameIndexes = new GLuint[wireFrameIndexCount];

    GLuint *wfIBuf = wireFrameIndexes;
    // top and bottom cap
    for (int si = 0; si < segments; si++) {
        wfIBuf[0] = 0;
        wfIBuf[1] = 2 + si;
        wfIBuf += 2;
        wfIBuf[0] = 1;
        wfIBuf[1] = 2 + si;
        wfIBuf += 2;
        wfIBuf[0] = 2 + si;
        wfIBuf[1] = 2 + (si + 1 >= segments ? 0 : si + 1);
        wfIBuf += 2;
    }

    wfIBuf[0] = 1;
    wfIBuf[1] = pointCount - 1;

    updateMatrix();
}

void PrimitiveSimpleArrow::setLength(float length)
{
    GLfloat3 *buf = (GLfloat3 *)points;
    buf->z = length;
    buf++;
    buf->z = length - fArrowLenth;
    buf++;

    for (unsigned int i = 0; i < pointCount - 1; i++) {
        buf->z = length - fArrowLenth;
        buf++;
    }

    buf->x = 0;
    buf->y = 0;
    buf->z = 0;
}

PrimitiveManager::PrimitiveManager()
{
    vertexShader = new QOpenGLShader(QOpenGLShader::Vertex);
    fragmentShader = new QOpenGLShader(QOpenGLShader::Fragment);
    program = new QOpenGLShaderProgram(nullptr);
}

PrimitiveManager::PrimitiveManager(const PrimitiveManager &pm)
{
    vertexShader = new QOpenGLShader(QOpenGLShader::Vertex);
    fragmentShader = new QOpenGLShader(QOpenGLShader::Fragment);
    program = new QOpenGLShaderProgram(nullptr);
    compileShaders(pm.vertexShader->sourceCode(), pm.fragmentShader->sourceCode());
//    primitives = pm.primitives;

}

PrimitiveManager::~PrimitiveManager()
{
    for (int i = 0; i < primitives.count(); i++)
        delete primitives[i];

    delete vertexShader;
    delete fragmentShader;
    delete program;
}

void PrimitiveManager::compileShaders(QString vertexShaderPath, QString fragmentShaderPath)
{
    vertexShader->compileSourceFile(vertexShaderPath);
    fragmentShader->compileSourceFile(fragmentShaderPath);
    linkShaders();
}

void PrimitiveManager::compileShaders(QByteArray &vertexShaderCode, QByteArray &fragmentShaderCode)
{
    vertexShader->compileSourceCode(vertexShaderCode);
    fragmentShader->compileSourceCode(fragmentShaderCode);
    linkShaders();
}

void PrimitiveManager::drawPrimitives(const QMatrix4x4 &pmvMatrix)
{
    program->bind();

    for (int i = 0; i < primitives.count(); i++) {
        QColor c = primitives[i]->color();
        QVector3D vColor(c.redF(), c.greenF(), c.blueF());
        program->setUniformValue("color", vColor);
        program->setUniformValue("Matrix", pmvMatrix * primitives[i]->matrix());
        primitives[i]->draw();
    }

    program->release();
}

Primitive *PrimitiveManager::addSphere(const int segments, const float radiusX, const float radiusY, const float radiusZ, QVector3D direction)
{
    Primitive *newSphere = new PrimitiveSphere(segments, radiusX, radiusY, radiusZ, direction);
    primitives.append(newSphere);
    return newSphere;
}

Primitive *PrimitiveManager::addCone(const int segments, const float height, const float radius, QVector3D direction)
{
    Primitive *newCone = new PrimitiveCone(segments, height, radius, direction);
    primitives.append(newCone);
    return newCone;
}

Primitive *PrimitiveManager::addCylinder(const int segments, const float height, const float radius, QVector3D direction)
{
    Primitive *newCylinder = new PrimitiveCylinder(segments, height, radius, direction);
    primitives.append(newCylinder);
    return newCylinder;
}

Primitive *PrimitiveManager::addSimpleArrow(const int segments, const float height, const float arrowHeight, const float radius, QVector3D direction)
{
    Primitive *newSimpleArrow = new PrimitiveSimpleArrow(segments, height, arrowHeight, radius, direction);
    primitives.append(newSimpleArrow);
    return newSimpleArrow;
}

void PrimitiveManager::linkShaders()
{
    qDebug() << "VertexShader:" << vertexShader->log();
    qDebug() << "FragmentShader:" << fragmentShader->log();

    program->addShader(vertexShader);
    program->addShader(fragmentShader);
    program->link();
}
