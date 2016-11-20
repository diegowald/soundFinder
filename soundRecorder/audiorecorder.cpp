/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QAudioProbe>
#include <QAudioRecorder>
#include <QDir>
#include <QFileDialog>
#include <QMediaRecorder>

#include "audiorecorder.h"
#include "qaudiolevel.h"

#include "ui_audiorecorder.h"

#include <QTimer>
#include <QDateTime>
#include <QtDebug>

#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QProcess>


static qreal getPeakValue(const QAudioFormat &format);
static QVector<qreal> getBufferLevels(const QAudioBuffer &buffer);

template <class T>
static QVector<qreal> getBufferLevels(const T *buffer, int frames, int channels);

/*
 * Idea para el nombre del archivo
 * leer de alguna configuracion la direccion de los servers
 * Leer de alguna configuracion el codigo de canal que se esta capturando
 * El nombre del archivo sera NombreCanal-YYYY-MM-DD-hh-mm-ss.ogg
 * Leer de alguna configuracion el intervalo de grabacion
 * Tener un timer que en cada intervalo de grabacion, calcule el nombre del archivo,
 * genere el archivo nuevo, registre el archivo realizado en la base de datos, etc etc
 */

QString AudioRecorder::_echoprint_codeGen = "";

AudioRecorder::AudioRecorder(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AudioRecorder)
{
    ui->setupUi(this);

    leerConfiguracion();

    audioRecorder1 = new QAudioRecorder(this);
    probe1 = new QAudioProbe;
    connect(probe1, SIGNAL(audioBufferProbed(QAudioBuffer)),
            this, SLOT(processBuffer(QAudioBuffer)));
    probe1->setSource(audioRecorder1);

    audioRecorder2 = new QAudioRecorder(this);
    probe2 = new QAudioProbe;
    connect(probe2, SIGNAL(audioBufferProbed(QAudioBuffer)),
            this, SLOT(processBuffer(QAudioBuffer)));
    probe2->setSource(audioRecorder2);

    //audio devices
    ui->audioDeviceBox->addItem(tr("Default"), QVariant(QString()));
    foreach (const QString &device, audioRecorder1->audioInputs()) {
        ui->audioDeviceBox->addItem(device, QVariant(device));
    }

    //audio codecs
    ui->audioCodecBox->addItem(tr("Default"), QVariant(QString()));
    foreach (const QString &codecName, audioRecorder1->supportedAudioCodecs()) {
        ui->audioCodecBox->addItem(codecName, QVariant(codecName));
    }

    //containers
    ui->containerBox->addItem(tr("Default"), QVariant(QString()));
    foreach (const QString &containerName, audioRecorder1->supportedContainers()) {
        ui->containerBox->addItem(containerName, QVariant(containerName));
    }

    //sample rate
    ui->sampleRateBox->addItem(tr("Default"), QVariant(0));
    foreach (int sampleRate, audioRecorder1->supportedAudioSampleRates()) {
        ui->sampleRateBox->addItem(QString::number(sampleRate), QVariant(
                sampleRate));
    }

    //channels
    ui->channelsBox->addItem(tr("Default"), QVariant(-1));
    ui->channelsBox->addItem(QStringLiteral("1"), QVariant(1));
    ui->channelsBox->addItem(QStringLiteral("2"), QVariant(2));
    ui->channelsBox->addItem(QStringLiteral("4"), QVariant(4));

    //quality
    ui->qualitySlider->setRange(0, int(QMultimedia::VeryHighQuality));
    ui->qualitySlider->setValue(int(QMultimedia::NormalQuality));

    //bitrates:
    ui->bitrateBox->addItem(tr("Default"), QVariant(0));
    ui->bitrateBox->addItem(QStringLiteral("32000"), QVariant(32000));
    ui->bitrateBox->addItem(QStringLiteral("64000"), QVariant(64000));
    ui->bitrateBox->addItem(QStringLiteral("96000"), QVariant(96000));
    ui->bitrateBox->addItem(QStringLiteral("128000"), QVariant(128000));

    connect(audioRecorder1, SIGNAL(durationChanged(qint64)), this,
            SLOT(updateProgress(qint64)));
    connect(audioRecorder1, SIGNAL(statusChanged(QMediaRecorder::Status)), this,
            SLOT(updateStatus(QMediaRecorder::Status)));
    connect(audioRecorder1, SIGNAL(stateChanged(QMediaRecorder::State)),
            this, SLOT(onStateChanged(QMediaRecorder::State)));
    connect(audioRecorder1, SIGNAL(error(QMediaRecorder::Error)), this,
            SLOT(displayErrorMessage()));

    connect(audioRecorder2, SIGNAL(durationChanged(qint64)), this,
            SLOT(updateProgress(qint64)));
    connect(audioRecorder2, SIGNAL(statusChanged(QMediaRecorder::Status)), this,
            SLOT(updateStatus(QMediaRecorder::Status)));
    connect(audioRecorder2, SIGNAL(stateChanged(QMediaRecorder::State)),
            this, SLOT(onStateChanged(QMediaRecorder::State)));
    connect(audioRecorder2, SIGNAL(error(QMediaRecorder::Error)), this,
            SLOT(displayErrorMessage()));

    _recorderInUse = 1;
    onTimer();
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &AudioRecorder::onTimer);
    timer->start(1000 * _intervalo);
}

AudioRecorder::~AudioRecorder()
{
    delete audioRecorder1;
    delete probe1;
    delete audioRecorder2;
    delete probe2;
}

void AudioRecorder::updateProgress(qint64 duration)
{
    QAudioRecorder *recorderInUse = (_recorderInUse == 1) ? audioRecorder2 : audioRecorder1;
    if (recorderInUse->error() != QMediaRecorder::NoError || duration < 2000)
        return;

    ui->statusbar->showMessage(tr("Recorded %1 sec").arg(duration / 1000));
}

void AudioRecorder::updateStatus(QMediaRecorder::Status status)
{
    QAudioRecorder *recorderInUse = (_recorderInUse == 1) ? audioRecorder2 : audioRecorder1;
    QString statusMessage;

    switch (status) {
    case QMediaRecorder::RecordingStatus:
        statusMessage = tr("Recording to %1").arg(recorderInUse->actualLocation().toString());
        break;
    case QMediaRecorder::PausedStatus:
        clearAudioLevels();
        statusMessage = tr("Paused");
        break;
    case QMediaRecorder::UnloadedStatus:
    case QMediaRecorder::LoadedStatus:
        clearAudioLevels();
        statusMessage = tr("Stopped");
    default:
        break;
    }

    if (recorderInUse->error() == QMediaRecorder::NoError)
        ui->statusbar->showMessage(statusMessage);
    else
        qDebug() << recorderInUse->errorString();
}

void AudioRecorder::onStateChanged(QMediaRecorder::State state)
{
    switch (state) {
    case QMediaRecorder::RecordingState:
        break;
    case QMediaRecorder::PausedState:
        break;
    case QMediaRecorder::StoppedState:
        break;
    }

}

static QVariant boxValue(const QComboBox *box)
{
    int idx = box->currentIndex();
    if (idx == -1)
        return QVariant();

    return box->itemData(idx);
}


void AudioRecorder::displayErrorMessage()
{
    QAudioRecorder *recorderInUse = (_recorderInUse == 1) ? audioRecorder2 : audioRecorder1;
    ui->statusbar->showMessage(recorderInUse->errorString());
}

void AudioRecorder::clearAudioLevels()
{
    for (int i = 0; i < audioLevels.size(); ++i)
        audioLevels.at(i)->setLevel(0);
}

// This function returns the maximum possible sample value for a given audio format
qreal getPeakValue(const QAudioFormat& format)
{
    // Note: Only the most common sample formats are supported
    if (!format.isValid())
        return qreal(0);

    if (format.codec() != "audio/pcm")
        return qreal(0);

    switch (format.sampleType()) {
    case QAudioFormat::Unknown:
        break;
    case QAudioFormat::Float:
        if (format.sampleSize() != 32) // other sample formats are not supported
            return qreal(0);
        return qreal(1.00003);
    case QAudioFormat::SignedInt:
        if (format.sampleSize() == 32)
            return qreal(INT_MAX);
        if (format.sampleSize() == 16)
            return qreal(SHRT_MAX);
        if (format.sampleSize() == 8)
            return qreal(CHAR_MAX);
        break;
    case QAudioFormat::UnSignedInt:
        if (format.sampleSize() == 32)
            return qreal(UINT_MAX);
        if (format.sampleSize() == 16)
            return qreal(USHRT_MAX);
        if (format.sampleSize() == 8)
            return qreal(UCHAR_MAX);
        break;
    }

    return qreal(0);
}

// returns the audio level for each channel
QVector<qreal> getBufferLevels(const QAudioBuffer& buffer)
{
    QVector<qreal> values;

    if (!buffer.format().isValid() || buffer.format().byteOrder() != QAudioFormat::LittleEndian)
        return values;

    if (buffer.format().codec() != "audio/pcm")
        return values;

    int channelCount = buffer.format().channelCount();
    values.fill(0, channelCount);
    qreal peak_value = getPeakValue(buffer.format());
    if (qFuzzyCompare(peak_value, qreal(0)))
        return values;

    switch (buffer.format().sampleType()) {
    case QAudioFormat::Unknown:
    case QAudioFormat::UnSignedInt:
        if (buffer.format().sampleSize() == 32)
            values = getBufferLevels(buffer.constData<quint32>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 16)
            values = getBufferLevels(buffer.constData<quint16>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 8)
            values = getBufferLevels(buffer.constData<quint8>(), buffer.frameCount(), channelCount);
        for (int i = 0; i < values.size(); ++i)
            values[i] = qAbs(values.at(i) - peak_value / 2) / (peak_value / 2);
        break;
    case QAudioFormat::Float:
        if (buffer.format().sampleSize() == 32) {
            values = getBufferLevels(buffer.constData<float>(), buffer.frameCount(), channelCount);
            for (int i = 0; i < values.size(); ++i)
                values[i] /= peak_value;
        }
        break;
    case QAudioFormat::SignedInt:
        if (buffer.format().sampleSize() == 32)
            values = getBufferLevels(buffer.constData<qint32>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 16)
            values = getBufferLevels(buffer.constData<qint16>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 8)
            values = getBufferLevels(buffer.constData<qint8>(), buffer.frameCount(), channelCount);
        for (int i = 0; i < values.size(); ++i)
            values[i] /= peak_value;
        break;
    }

    return values;
}

template <class T>
QVector<qreal> getBufferLevels(const T *buffer, int frames, int channels)
{
    QVector<qreal> max_values;
    max_values.fill(0, channels);

    for (int i = 0; i < frames; ++i) {
        for (int j = 0; j < channels; ++j) {
            qreal value = qAbs(qreal(buffer[i * channels + j]));
            if (value > max_values.at(j))
                max_values.replace(j, value);
        }
    }

    return max_values;
}

void AudioRecorder::processBuffer(const QAudioBuffer& buffer)
{
    if (audioLevels.count() != buffer.format().channelCount()) {
        qDeleteAll(audioLevels);
        audioLevels.clear();
        for (int i = 0; i < buffer.format().channelCount(); ++i) {
            QAudioLevel *level = new QAudioLevel(ui->centralwidget);
            audioLevels.append(level);
            ui->levelsLayout->addWidget(level);
        }
    }

    QVector<qreal> levels = getBufferLevels(buffer);
    for (int i = 0; i < levels.count(); ++i)
        audioLevels.at(i)->setLevel(levels.at(i));
}


void AudioRecorder::leerConfiguracion()
{
    // Por el momento hard coded
    _codigoCanal = "XYZ";
    _formatoNombreArchivo = "%canal%-%YYYY%%MM%%DD%%hh%%mm%%ss%";
    _intervalo = 10;
    _pathSonidos = "/home/diego/QtProjects/soundFinder/bin/snd/";
    _echoprint_codeGen = "./echoprint-codegen";
}




QString AudioRecorder::calcularNombreDeArchivo() const
{
    QString nombreArchivo = _formatoNombreArchivo;
    QDateTime currentTime = QDateTime::currentDateTime();

    nombreArchivo = nombreArchivo.replace("%canal%", _codigoCanal)
            .replace("%YYYY%", QString::number(currentTime.date().year()).rightJustified(4, '0'))
            .replace("%MM%", QString::number(currentTime.date().month()).rightJustified(2, '0'))
            .replace("%DD%", QString::number(currentTime.date().day()).rightJustified(2, '0'))
            .replace("%hh%", QString::number(currentTime.time().hour()).rightJustified(2, '0'))
            .replace("%mm%", QString::number(currentTime.time().minute()).rightJustified(2, '0'))
            .replace("%ss%", QString::number(currentTime.time().second()).rightJustified(2, '0'));

    nombreArchivo = _pathSonidos + nombreArchivo;

    return nombreArchivo;
}


void AudioRecorder::onTimer()
{
    QAudioRecorder *recorderInUse = (_recorderInUse == 1) ? audioRecorder1 : audioRecorder2;

    if (recorderInUse->state() == QMediaRecorder::RecordingState)
    {
        recorderInUse->stop();
        QFuture<void> future = QtConcurrent::run(codegen, recorderInUse->outputLocation().toString());
    }


    QString nombreArchivo = calcularNombreDeArchivo();
    qDebug() << nombreArchivo;

    recorderInUse = (_recorderInUse == 1) ? audioRecorder2 : audioRecorder1;
    _recorderInUse = (_recorderInUse == 1) ? 2 : 1;

    recorderInUse->setOutputLocation(QUrl(nombreArchivo));

    qDebug() << recorderInUse->state();
    qDebug() << recorderInUse->outputLocation().url();
    if (recorderInUse->state() == QMediaRecorder::StoppedState)
    {
        recorderInUse->setAudioInput(boxValue(ui->audioDeviceBox).toString());

        QAudioEncoderSettings settings;
        settings.setCodec(boxValue(ui->audioCodecBox).toString());
        settings.setSampleRate(boxValue(ui->sampleRateBox).toInt());
        settings.setBitRate(boxValue(ui->bitrateBox).toInt());
        settings.setChannelCount(boxValue(ui->channelsBox).toInt());
        settings.setQuality(QMultimedia::EncodingQuality(ui->qualitySlider->value()));
        settings.setEncodingMode(ui->constantQualityRadioButton->isChecked() ?
                                 QMultimedia::ConstantQualityEncoding :
                                 QMultimedia::ConstantBitRateEncoding);

        QString container = boxValue(ui->containerBox).toString();

        recorderInUse->setEncodingSettings(settings, QVideoEncoderSettings(), container);
        recorderInUse->record();
    }
}

void AudioRecorder::codegen(const QString &file)
{
    QProcess process;
    QString command = _echoprint_codeGen + " " + file;
    process.start(command);
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    qDebug() << output;
}

