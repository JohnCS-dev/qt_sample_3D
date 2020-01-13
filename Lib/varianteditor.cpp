#include "varianteditor.h"

#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QFontDialog>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QDebug>
VariantEditor *VariantEditor::createEditor(QString valueName, QVariant value, void *source)
{
    switch (value.type()) {
    case QVariant::Bool:
        return new BoolVariantEditor(valueName, value, source);

    case QVariant::Int:
        return new IntVariantEditor(valueName, value, source);

    case QMetaType::Float:
        return new FloatVariantEditor(valueName, value, source);

    case QVariant::Double:
        return new DoubleVariantEditor(valueName, value, source);

    case QVariant::Color:
        return new ColorVariantEditor(valueName, value, source);

    case QVariant::Font:
        return new FontVariantEditor(valueName, value, source);

    default:
        Q_ASSERT(false);
    }
    return nullptr;
}

VariantEditor::VariantEditor(QString valueName, QVariant value, void *source) : fValue(value), fSource(source)
{
    fHBoxLayout = new QHBoxLayout();
    fHBoxLayout->setMargin(1);
    setLayout(fHBoxLayout);

    int paramIndex = valueName.indexOf("@");
    fValueName = valueName.left(valueName.indexOf("@"));
    if (paramIndex >= 0)
        paramIndex++;

    QStringList params = valueName.mid(paramIndex).split(';');
    foreach (QString str, params) {
        int separator = str.indexOf("=");
        QString key = str.left(separator);
        QString value = str.mid(separator + 1);
        fParams.insert(key.trimmed(),value.trimmed());
    }
}

FontVariantEditor::FontVariantEditor(QString valueName, QVariant value, void *source) : VariantEditor(valueName, value, source)
{
    fFont = value.value<QFont>();
    label = new QLabel(QString("%1, %2").arg(fFont.family()).arg(fFont.pointSize()));
    QPushButton *b = new QPushButton("...");
    b->setMaximumHeight(20);
    b->setMaximumWidth(20);
    b->setToolTip(tr("Set font"));

    label->setBuddy(b);

    fHBoxLayout->addWidget(label);
    fHBoxLayout->addWidget(b);

    connect(b, SIGNAL(clicked(bool)), this, SLOT(showFontDialog()));
}

void FontVariantEditor::showFontDialog()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, fFont, nullptr, QStringLiteral("Select font"));
    if (ok) {
        fFont = font;
        fValue = QVariant::fromValue(fFont);
        label->setText(QString("%1, %2").arg(fFont.family()).arg(fFont.pointSize()));
        QFont *v = (QFont*)fSource;
        *v = fFont;
        valueChanged(fValueName, QVariant::fromValue(fFont));
    }
}

ColorVariantEditor::ColorVariantEditor(QString valueName, QVariant value, void *source) : VariantEditor(valueName, value, source)
{
    fColor = value.value<QColor>();
    label = new QLabel();
    label->setFrameShape(QFrame::Box);
    QPixmap pix(36,18);
    pix.fill(fColor);
    label->setPixmap(pix);

    QPushButton *b = new QPushButton("...");
    b->setMaximumHeight(20);
    b->setMaximumWidth(20);
    b->setToolTip(tr("Set color"));

    label->setBuddy(b);

    fHBoxLayout->addWidget(label);
    fHBoxLayout->addWidget(b);

    connect(b, SIGNAL(clicked(bool)), this, SLOT(showColorDialog()));
}

void ColorVariantEditor::showColorDialog()
{
    QColor color = QColorDialog::getColor(fColor, nullptr, "Select color");
    if (color.isValid()) {
        fColor = color;
        QPixmap pix(36,18);
        pix.fill(fColor);
        label->setPixmap(pix);
        QColor *v = (QColor*)fSource;
        *v = fColor;
        valueChanged(fValueName, QVariant::fromValue(fColor));
    }
}

FloatVariantEditor::FloatVariantEditor(QString valueName, QVariant value, void *source) : VariantEditor(valueName, value, source)
{
    fValue = value.value<float>();
    QDoubleSpinBox *spBox = new QDoubleSpinBox();
    spBox->setDecimals(2);

    if (fParams.contains("min")) {
        bool ok;
        float min = fParams.value("min").toFloat(&ok);
        if (ok)
            spBox->setMinimum(min);
    }
    if (fParams.contains("max")) {
        bool ok;
        float max = fParams.value("max").toFloat(&ok);
        if (ok)
            spBox->setMaximum(max);
    }
    if (fParams.contains("step")) {
        bool ok;
        float step = fParams.value("step").toFloat(&ok);
        if (ok)
            spBox->setSingleStep(step);
    }
    if (fParams.contains("digits")) {
        bool ok;
        int digits = fParams.value("digits").toInt(&ok);
        if (ok)
            spBox->setDecimals(digits);
    }
    spBox->setValue(fValue);

    fHBoxLayout->addWidget(spBox);

    connect(spBox, SIGNAL(valueChanged(double)), this, SLOT(setValue(double)));
}

void FloatVariantEditor::setValue(double value)
{
    if (fValue != value) {
        fValue = value;
        float *v = (float*)fSource;
        *v = fValue;
        valueChanged(fValueName, QVariant::fromValue(fValue));
    }
}

DoubleVariantEditor::DoubleVariantEditor(QString valueName, QVariant value, void *source) : VariantEditor(valueName, value, source)
{
    fValue = value.value<double>();
    QDoubleSpinBox *spBox = new QDoubleSpinBox();
    spBox->setDecimals(2);

    if (fParams.contains("min")) {
        bool ok;
        double min = fParams.value("min").toDouble(&ok);
        if (ok)
            spBox->setMinimum(min);
    }
    if (fParams.contains("max")) {
        bool ok;
        double max = fParams.value("max").toDouble(&ok);
        if (ok)
            spBox->setMaximum(max);
    }
    if (fParams.contains("step")) {
        bool ok;
        double step = fParams.value("step").toDouble(&ok);
        if (ok)
            spBox->setSingleStep(step);
    }
    if (fParams.contains("digits")) {
        bool ok;
        int digits = fParams.value("digits").toInt(&ok);
        if (ok)
            spBox->setDecimals(digits);
    }
    spBox->setValue(fValue);

    fHBoxLayout->addWidget(spBox);

    connect(spBox, SIGNAL(valueChanged(double)), this, SLOT(setValue(double)));
}

void DoubleVariantEditor::setValue(double value)
{
    if (fValue != value) {
        fValue = value;
        double *v = (double*)fSource;
        *v = fValue;
        valueChanged(fValueName, QVariant::fromValue(fValue));
    }
}

IntVariantEditor::IntVariantEditor(QString valueName, QVariant value, void *source) : VariantEditor(valueName, value, source)
{
    fValue = value.value<int>();
    QSpinBox *spBox = new QSpinBox();

    if (fParams.contains("min")) {
        bool ok;
        int min = fParams.value("min").toInt(&ok);
        if (ok)
            spBox->setMinimum(min);
    }
    if (fParams.contains("max")) {
        bool ok;
        int max = fParams.value("max").toInt(&ok);
        if (ok)
            spBox->setMaximum(max);
    }
    if (fParams.contains("step")) {
        bool ok;
        int step = fParams.value("step").toInt(&ok);
        if (ok)
            spBox->setSingleStep(step);
    }
    spBox->setValue(fValue);

    fHBoxLayout->addWidget(spBox);

    connect(spBox, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
}

void IntVariantEditor::setValue(int value)
{
    if (fValue != value) {
        fValue = value;
        int *v = (int*)fSource;
        *v = fValue;
        valueChanged(fValueName, QVariant::fromValue(fValue));
    }
}


BoolVariantEditor::BoolVariantEditor(QString valueName, QVariant value, void *source) : VariantEditor(valueName, value, source)
{
    fValue = value.value<bool>();
    QCheckBox *chBox = new QCheckBox();
    chBox->setChecked(fValue);

    fHBoxLayout->addWidget(chBox);

    connect(chBox, SIGNAL(clicked(bool)), this, SLOT(setValue(bool)));
}

void BoolVariantEditor::setValue(bool value)
{
    if (fValue != value) {
        fValue = value;
        bool *v = (bool*)fSource;
        *v = fValue;
        valueChanged(fValueName, QVariant::fromValue(fValue));
    }
}
