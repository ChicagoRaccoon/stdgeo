#include "stdgeoparser.h"
#include "../stdgeo-parser/stdgeo_parser.h"
#include <QDebug>

StdGeoParser::StdGeoParser(QObject *parent)
    : QObject(parent)
    , m_handle(nullptr)
    , m_lastSuccess(false)
{
    m_handle = stdgeo_parser_create();
    if (!m_handle) {
        qWarning() << "Failed to create stdgeo parser handle";
    }
}

StdGeoParser::~StdGeoParser()
{
    if (m_handle) {
        stdgeo_parser_destroy(static_cast<ParserHandle*>(m_handle));
    }
}

QString StdGeoParser::executeCommand(const QString &command)
{
    if (!m_handle) {
        updateLastResult(false, "Parser not initialized");
        return QString();
    }
    
    QByteArray commandBytes = command.toUtf8();
    CommandResultC result = stdgeo_parser_execute(
        static_cast<ParserHandle*>(m_handle), 
        commandBytes.constData()
    );
    
    QString resultMessage;
    bool success = false;
    
    switch (result.status) {
        case 0: // Success
            success = true;
            if (result.message) {
                resultMessage = QString::fromUtf8(result.message);
                stdgeo_parser_free_string(result.message);
            }
            break;
            
        case 1: // Error
            success = false;
            if (result.message) {
                resultMessage = QString::fromUtf8(result.message);
                stdgeo_parser_free_string(result.message);
            }
            break;
            
        case 2: // No output
            success = true;
            resultMessage = QString();
            break;
            
        default:
            success = false;
            resultMessage = "Unknown result status";
            break;
    }
    
    updateLastResult(success, success ? QString() : resultMessage);
    emit commandExecuted(command, resultMessage, success);
    
    return resultMessage;
}

bool StdGeoParser::lastCommandSucceeded() const
{
    return m_lastSuccess;
}

QString StdGeoParser::lastError() const
{
    return m_lastError;
}

int StdGeoParser::geometryCount() const
{
    if (!m_handle) {
        return -1;
    }
    
    return stdgeo_parser_geometry_count(static_cast<ParserHandle*>(m_handle));
}

void StdGeoParser::clearContext()
{
    if (m_handle) {
        stdgeo_parser_clear_context(static_cast<ParserHandle*>(m_handle));
    }
}

bool StdGeoParser::isValid() const
{
    return m_handle != nullptr;
}

void StdGeoParser::updateLastResult(bool success, const QString &error)
{
    m_lastSuccess = success;
    m_lastError = error;
}