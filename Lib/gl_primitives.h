#ifndef GL_PRIMITIVES_H
#define GL_PRIMITIVES_H

#include <QVector3D>
#include <QColor>
#include <QOpenGLBuffer>
#include <QOpenGLShader>
//#include <GL/gl.h>

union GLfloat3 {
    GLfloat m[3];
    struct { GLfloat x,y,z; };
};

enum DrawType {
    dtWireFrame,
    dtTriangleWireFrame,
    dtSurface,
    dtPoints
};

class Primitive
{
public:
    Primitive(QVector3D direction = QVector3D(0.0f, 0.0f, 1.0f)) : pDirection(direction), dType(dtSurface), fColor(Qt::black), points(nullptr), wireFrameIndexes(nullptr), surfaceIndexes(nullptr) {}
    ~Primitive();
    virtual void draw();
    void setDrawType(DrawType type) { dType = type; }
    virtual inline DrawType drawType() { return dType; }
    void setColor(QColor color) { fColor = color; }
    QColor color() { return fColor; }
    void bindBuffer() { vertexBuffer.bind(); }
    void releaseBuffer() { vertexBuffer.release(); }
    GLuint *wFrameIndexes() { return wireFrameIndexes; }
    quint32 wFrameIndexCount() { return wireFrameIndexCount; }
    QMatrix4x4 matrix() { return trMatrix; }
    void setPos(QVector3D pos) { pPosition = pos; updateMatrix(); }
    void setPos(float x, float y, float z) { pPosition = QVector3D(x, y, z); updateMatrix(); }
    QVector3D pos() { return pPosition; }
    void setDirection(QVector3D direction) { pDirection = direction; updateMatrix();}

protected:
    GLfloat *points; // size 3 * pointCount
    quint32 pointCount;
    GLuint *wireFrameIndexes;
    quint32 wireFrameIndexCount;
    GLuint *surfaceIndexes;
    quint32 surfaceIndexCount;

    void createVertexBuffer();
    void updateMatrix();

private:
    DrawType dType;
    QColor fColor;
    QOpenGLBuffer vertexBuffer;
    QMatrix4x4 trMatrix;
    QVector3D pPosition;
    QVector3D pDirection;
};

class PrimitiveSphere : public Primitive
{
public:
    PrimitiveSphere(const int segments, const float radiusX, const float radiusY, const float radiusZ, QVector3D direction = QVector3D(0.0f, 0.0f, 1.0f));
};

class PrimitiveCone : public Primitive
{
public:
    PrimitiveCone(const int segments, const float height, const float radius, QVector3D direction = QVector3D(0.0f, 0.0f, 1.0f));
};

class PrimitiveCylinder : public Primitive
{
public:
    PrimitiveCylinder(const int segments, const float height, const float radius, QVector3D direction = QVector3D(0.0f, 0.0f, 1.0f));
};

class PrimitiveSimpleArrow : public Primitive
{
public:
    PrimitiveSimpleArrow(const int segments, const float height, const float arrowHeight, const float radius, QVector3D direction = QVector3D(0.0f, 0.0f, 1.0f));
    DrawType drawType() override { return dtWireFrame; }
    void setStart(float x, float y, float z) { setPos(x,y,z); }
    void setLength(float length);

private:
    float fArrowLenth;
};

class PrimitiveManager
{
public:
    PrimitiveManager();
    PrimitiveManager( const PrimitiveManager& pm);
    ~PrimitiveManager();
    void compileShaders(QString vertexShaderPath, QString fragmentShaderPath);
    void compileShaders(QByteArray &vertexShaderCode, QByteArray &fragmentShaderCode);
    void drawPrimitives(const QMatrix4x4 &pmvMatrix);

    Primitive * addSphere(const int segments, const float radiusX, const float radiusY, const float radiusZ, QVector3D direction = QVector3D(0.0f, 0.0f, 1.0f));
    Primitive * addCone(const int segments, const float height, const float radius, QVector3D direction = QVector3D(0.0f, 0.0f, 1.0f));
    Primitive * addCylinder(const int segments, const float height, const float radius, QVector3D direction = QVector3D(0.0f, 0.0f, 1.0f));
    Primitive * addSimpleArrow(const int segments, const float height, const float arrowHeight, const float radius, QVector3D direction = QVector3D(0.0f, 0.0f, 1.0f));

private:
    void linkShaders();
    QOpenGLShader *vertexShader;
    QOpenGLShader *fragmentShader;
    QOpenGLShaderProgram *program;

    QList<Primitive*> primitives;    
};

#endif // GL_PRIMITIVES_H
