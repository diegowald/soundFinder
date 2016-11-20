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

#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <QMainWindow>
#include <QMediaRecorder>
#include <QUrl>

QT_BEGIN_NAMESPACE
namespace Ui { class AudioRecorder; }
class QAudioRecorder;
class QAudioProbe;
class QAudioBuffer;
QT_END_NAMESPACE

class QAudioLevel;

class AudioRecorder : public QMainWindow
{
    Q_OBJECT

public:
    AudioRecorder(QWidget *parent = 0);
    ~AudioRecorder();

private:
    void leerConfiguracion();
    QString calcularNombreDeArchivo() const;

    static void codegen(const QString &file);

    /*
     * Idea para el nombre del archivo
     * leer de alguna configuracion la direccion de los servers
     * Leer de alguna configuracion el codigo de canal que se esta capturando
     * El nombre del archivo sera NombreCanal-YYYY-MM-DD-hh-mm-ss.ogg
     * Leer de alguna configuracion el intervalo de grabacion
     * Tener un timer que en cada intervalo de grabacion, calcule el nombre del archivo,
     * genere el archivo nuevo, registre el archivo realizado en la base de datos, etc etc
     */


public slots:
    void processBuffer(const QAudioBuffer&);

private slots:

    void updateStatus(QMediaRecorder::Status);
    void onStateChanged(QMediaRecorder::State);
    void updateProgress(qint64 pos);
    void displayErrorMessage();
    void onTimer();

private:
    void clearAudioLevels();

    Ui::AudioRecorder *ui;

    QAudioRecorder *audioRecorder1;
    QAudioRecorder *audioRecorder2;
    QAudioProbe *probe1;
    QAudioProbe *probe2;
    QList<QAudioLevel*> audioLevels;

    int _recorderInUse;
    QString _codigoCanal;
    QString _formatoNombreArchivo;
    int _intervalo;
    QString _pathSonidos;
    static QString _echoprint_codeGen;

};

#endif // AUDIORECORDER_H
