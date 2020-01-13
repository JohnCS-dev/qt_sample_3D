#ifndef BASESCENE3D_H
#define BASESCENE3D_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>

#include "ui_basesettingswindow.h"
#include "gl_primitives.h"

class BaseSettings : public QObject
{
    Q_OBJECT
public:
    enum SettingsType {
        stSettings,
        stGroup
    };

    struct SettingsItem {
        QVariant value;
        void *source;
    };

    BaseSettings(QString name, QString title, QObject *parent = nullptr) : QObject(parent), fTitle(title) { setObjectName(name); }
    BaseSettings(const BaseSettings & settings) {
        setObjectName(settings.name());
        fTitle = settings.title();
        fItems = settings.fItems;
        fChildSettings = settings.fChildSettings;
    }

    BaseSettings(const BaseSettings & settings, QString name, QString title) {
        setObjectName(name);
        fTitle = title;
        fItems = settings.fItems;
        fChildSettings = settings.fChildSettings;
    }

    BaseSettings& operator=(const BaseSettings& src) {
        setObjectName(src.name());
        fType = src.fType;
        fTitle = src.fTitle;
        fItems = src.fItems;
        fChildSettings = src.fChildSettings;
        return * this;
    }

    QString name() const { return objectName(); }
    void setName(QString name) { setObjectName(name); }
    QString title() const { return fTitle; }
    void setTitle(QString title) { fTitle = title; }
    int getItemCount() const { return fItems.count(); }
    int getSettingsCount() const { return fChildSettings.count(); }
//    QVariant getItem(int index) const { return fItems.value(fItems.keys()[index]); }
    QVariant getItem(QString name) const { return fItems.value(name).value; }
    QPair<QString,SettingsItem> getItem(int index) const { return qMakePair(fItems.keys()[index], fItems.value(fItems.keys()[index])); }
    BaseSettings * getSettings(int index) { return &(fChildSettings[index]); }
    SettingsType type() const { return fType; }
    void addSettingsItem(BaseSettings settings) { settings.setName(name() + "::" + settings.name()); fChildSettings.append(settings); }
    void clear();

    void addSettingsItem(QString name, float *value);
    void addSettingsItem(QString name, double *value);
    void addSettingsItem(QString name, int *value);
    void addSettingsItem(QString name, bool *value);
    void addSettingsItem(QString name, QFont *value);
    void addSettingsItem(QString name, QColor *value);

private:
    QString fTitle;
    SettingsType fType;
    QMap<QString,SettingsItem> fItems;
    QList<BaseSettings> fChildSettings;

    void addSettingsItem(QString name, QVariant value, void *source);
};

class BaseScene3D : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    enum MoveMode {
        mmObjectMode,
        mmFirstPersonMode
    };

   BaseScene3D(QWidget* pwgt = 0);
   ~BaseScene3D();

   enum ScaleLines {
       slX = 0,
       slY = 1,
       slZ = 2,
       slCount = 3
   };
   enum ScalePlanes {
       spXY = 0,
       spYZ = 1,
       spXZ = 2,
       spCount = 3
   };

   struct ScalePlaneSettings {
       bool visible;
       float offset;
       ScalePlaneSettings() : visible(false), offset(1) {}
   };

   struct ScaleSettings {
       float start;
       float length;
       float step;
       int precision;
       ScaleSettings() : start(1), length(1), step(1) {}
   };

   struct SpaceData {
       float x;
       float xLength;
       float y;
       float yLength;
       float z;
       float zLength;
       SpaceData() : x(0), y(0), z(0), xLength(3), yLength(3), zLength(3) {}
       SpaceData(float x, float y, float z, float xLen, float yLen, float zLen) : x(x), y(y), z(z), xLength(xLen), yLength(yLen), zLength(zLen) {}
       SpaceData& operator=(const SpaceData& right) {
           x = right.x;
           xLength = right.xLength;
           y = right.y;
           yLength = right.yLength;
           z = right.z;
           zLength = right.zLength;
           return *this;
       }
   };

   bool shaderIsAvailable() { return fShaderAvailable; }
   bool vertexBufferIsAvailable() { return fVertexBufferAvailable; }
   void qgluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);
   void setXScaleRange(GLfloat start, GLfloat end);
   void setMoveMode(MoveMode mode) { fMoveMode = mode; }
   void updateXScaleValues();
   void updateXScaleValues(float start, float end, float step, int precision);
   void updateYScaleValues();
   void updateYScaleValues(float start, float end, float step, int precision);
   void updateZScaleValues();
   void updateZScaleValues(float start, float end, float step, int precision);
   float normalizeAngle(float angle);
   void setSpaceData(float x, float y, float z, float xLen, float yLen, float zLen) { fSpaceData = SpaceData(x, y, z, xLen, yLen, zLen); update(); }
   void setCamTarget(float x, float y, float z);

protected:
    ScaleSettings fScalesSettings[slCount];
    ScalePlaneSettings fScalesPlaneSettings[spCount];
    BaseSettings fSettings;
    QFont fScaleFont;
    QColor fScaleColor;
    QColor fGridColor;

    void initializeGL();                     // метод для проведения инициализаций, связанных с OpenGL
    void resizeGL(int nWidth, int nHeight);  // метод вызывается при изменении размеров окна виджета
    void paintGL();                          // метод, чтобы заново перерисовать содержимое виджета
    virtual void paintData(const QMatrix4x4 &pmvMatrix) { Q_UNUSED(pmvMatrix); }
    virtual void drawText() {}

    virtual void drawAxis(const QMatrix4x4 &pmvMatrix);          // построить оси координат
    void drawScales(const QMatrix4x4&);

    void prepareView();
    void setWorldTransform();
    void setInverseWorldTransform();
    void renderText(float x, float y, float z, QString text, QFont &font, QColor color, Qt::Alignment textAlignment = Qt::AlignCenter, bool scaled = false);
    void contextMenuEvent(QContextMenuEvent *event) override;
    virtual void createViewSettings();


    void mousePressEvent(QMouseEvent* pe);   // методы обработки события мыши при нажатии клавиши мыши
    void mouseMoveEvent(QMouseEvent* pe);    // методы обработки события мыши при перемещении мыши
    void mouseReleaseEvent(QMouseEvent* pe); // методы обработки событий мыши при отжатии клавиши мыши
    void wheelEvent(QWheelEvent* pe);        // метод обработки событий колесика мыши
    void keyPressEvent(QKeyEvent* pe);       // методы обработки события при нажатии определенной клавиши

private:
      MoveMode fMoveMode;
      GLfloat xRotate;   // переменная хранит угол поворота вокруг оси X
      GLfloat yRotate;   // переменная хранит угол поворота вокруг оси Y
      GLfloat zRotate;   // переменная хранит угол поворота вокруг оси Z
      GLfloat xTransl;   // переменная хранит величину трансляции оси X
      GLfloat yTransl;   // переменная хранит величину трансляции оси Y
      GLfloat zTransl;   // переменная хранит величину трансляции оси Z
      GLfloat nScale;    // переменная отвечает за масштабирование обьекта
      GLfloat zCam;
      bool light;
      bool fSmooth;
      GLfloat axisXStart;
      GLfloat axisXEnd;

      bool fShaderAvailable;
      bool fVertexBufferAvailable;

      QOpenGLTexture *xScaleTextureRight;
      QOpenGLTexture *xScaleTextureLeft;
      QOpenGLTexture *xInverseScaleTextureRight;
      QOpenGLTexture *xInverseScaleTextureLeft;

      QOpenGLTexture *yScaleTextureRight;
      QOpenGLTexture *yScaleTextureLeft;
      QOpenGLTexture *yInverseScaleTextureRight;
      QOpenGLTexture *yInverseScaleTextureLeft;

      QOpenGLTexture *zScaleTextureRight;
      QOpenGLTexture *zScaleTextureLeft;

//	  QOpenGLShader *vertexShader;
//	  QOpenGLShader *fragmentShader;
//	  QOpenGLShaderProgram *program;

      QPoint ptrMousePosition; // переменная хранит координату указателя мыши в момент нажатия
      bool rBut;
      bool isMouseMove;
      PrimitiveManager *pManager;
      PrimitiveSimpleArrow *fArrowX;

//      void doSelect2(int x, int y, bool multiSelect = false);

      SpaceData fSpaceData;

      void scale_plus();       // приблизить сцену
      void scale_minus();      // удалиться от сцены
      void rotate_up();        // повернуть сцену вверх
      void rotate_down();      // повернуть сцену вниз
      void rotate_left();      // повернуть сцену влево
      void rotate_right();     // повернуть сцену вправо
      void rotate_forward();   // повернуть сцену вперёд
      void rotate_backward();  // повернуть сцену назад
      void translate_down();   // транслировать сцену вниз
      void translate_up();     // транслировать сцену вверх
      void translate_left();   // транслировать сцену вниз
      void translate_right();   // транслировать сцену вверх
      void translate_forward(); // транслировать сцену вниз
      void translate_backward();// транслировать сцену вверх
      void defaultScene();      // наблюдение сцены по умолчанию

private slots:
      void update3DView();

signals:
      void settingsChanged(QString,QVariant);
};

class BaseSettingsWin : public QMainWindow
{
    Q_OBJECT
public:
    BaseSettingsWin(BaseSettings *settings, QWidget * parent = nullptr);
    bool alwaysVisible() { return  true; }

private:
    Ui::BaseSettingsWindow ui;

    QList<QWidget*> fStackWidgetList;

    QTreeWidgetItem *createTopItem(BaseSettings *settings);
    QWidget *createSettingsWidget(BaseSettings *settings);
    QWidget *createVariantEditor(QVariant value) { return nullptr; }

private slots:
    void currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

signals:
    void settingsChanged(QString,QVariant);
};

#endif // BASESCENE3D_H
