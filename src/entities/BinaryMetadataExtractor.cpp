//
// Created by alexis on 6/8/18.
//
#include <QDebug>
#include <QProcess>
#include <QFileInfo>
#include <appimage/appimage.h>
#include <QtCore/QDateTime>

#include "BinaryMetadataExtractor.h"

BinaryMetadataExtractor::BinaryMetadataExtractor(const QString &target)
        : target(target) {}

QVariantMap BinaryMetadataExtractor::getMetadata() {
    QVariantMap data;
    data["architecture"] = getBinaryArch();
    data["sha512checksum"] = getSha512CheckSum();
    data["size"] = getFileSize();
    data["type"] = getAppImageType();
    data["date"] = getTime();

    return data;
}

QDateTime BinaryMetadataExtractor::getTime() const {
    QFileInfo f(target);
    auto date = f.birthTime();
    return date;
}

qint64 BinaryMetadataExtractor::getFileSize() const {
    QFileInfo fileInfo(target);
    qint64 size = fileInfo.size();
    return size;
}

QString BinaryMetadataExtractor::getSha512CheckSum() const {
    auto process = new QProcess();
    QStringList arguments{target};
    process->start("sha512sum", arguments);

    process->waitForFinished();
    if (process->exitCode() != 0)
        throw std::runtime_error(process->errorString().toStdString());
    QString rawOutput = process->readAll();
    QString sha512checksum = rawOutput.section(" ", 0, 0).trimmed();

    process->deleteLater();
    return sha512checksum;
}

QString BinaryMetadataExtractor::getBinaryArch() const {
    auto process = new QProcess();
    QStringList arguments;
    arguments << "-b" << "-e" << "ascii" << "-e" << "apptype" << "-e" << "encoding" <<
              "-e" << "elf" << "-e" << "tokens" << "-e" << "tar" << "-e" << "cdf" <<
              "-e" << "compress" << target;

    process->start("file", arguments);
    process->waitForFinished();
    if (process->exitCode() != 0)
        throw std::runtime_error(process->errorString().toStdString());
    QString rawOutput = process->readAll();
    QString arch = rawOutput.section(",", 1, 1).trimmed();

    process->deleteLater();
    return arch;
}

int BinaryMetadataExtractor::getAppImageType() const {
    return appimage_get_type(target.toStdString().c_str(), false);
}

