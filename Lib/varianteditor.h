#ifndef VARIANTEDITOR_H
#define VARIANTEDITOR_H

#include <QWidget>
#include <QVariant>

class QHBoxLayout;

class VariantEditor : public QWidget
{
    Q_OBJECT
public:
    static VariantEditor *createEditor(QString valueName, QVariant value, void *source);

protected:
    VariantEditor(QString valueName, QVariant value, void *source);

    QHBoxLayout *fHBoxLayout;
    QString fValueName;
    QVariant fValue;
    void *fSource;

    QMap<QString,QString> fParams;

signals:
    void valueChanged(QString, QVariant);
};

class QLabel;
class FontVariantEditor : public VariantEditor
{
    Q_OBJECT
public:
    FontVariantEditor(QString valueName, QVariant value, void *source);

private slots:
    void showFontDialog();
private:
    QFont fFont;
    QLabel *label;
};

class ColorVariantEditor : public VariantEditor
{
    Q_OBJECT
public:
    ColorVariantEditor(QString valueName, QVariant value, void *source);

private slots:
    void showColorDialog();
private:
    QColor fColor;
    QLabel *label;
};

class FloatVariantEditor : public VariantEditor
{
    Q_OBJECT
public:
    FloatVariantEditor(QString valueName, QVariant value, void *source);

private slots:
    void setValue(double value);
private:
    float fValue;
};

class DoubleVariantEditor : public VariantEditor
{
    Q_OBJECT
public:
    DoubleVariantEditor(QString valueName, QVariant value, void *source);

private slots:
    void setValue(double value);
private:
    double fValue;
};

class IntVariantEditor : public VariantEditor
{
    Q_OBJECT
public:
    IntVariantEditor(QString valueName, QVariant value, void *source);

private slots:
    void setValue(int value);
private:
    int fValue;
};

class BoolVariantEditor : public VariantEditor
{
    Q_OBJECT
public:
    BoolVariantEditor(QString valueName, QVariant value, void *source);

private slots:
    void setValue(bool value);
private:
    bool fValue;
};

#endif // VARIANTEDITOR_H
