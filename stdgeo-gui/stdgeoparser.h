#ifndef STDGEOPARSER_H
#define STDGEOPARSER_H

#include <QObject>
#include <QString>

/**
 * @brief Qt wrapper for the stdgeo-parser Rust library
 * 
 * This class provides a Qt-friendly interface to the stdgeo-parser library,
 * managing the C FFI calls and converting between C strings and QString.
 */
class StdGeoParser : public QObject
{
    Q_OBJECT

public:
    explicit StdGeoParser(QObject *parent = nullptr);
    ~StdGeoParser();

    /**
     * @brief Execute a command and return the result
     * @param command Command string to execute
     * @return Result message, or empty string if no output
     */
    QString executeCommand(const QString &command);

    /**
     * @brief Check if the last command was successful
     * @return true if the last command succeeded
     */
    bool lastCommandSucceeded() const;

    /**
     * @brief Get the error message from the last command
     * @return Error message, or empty string if no error
     */
    QString lastError() const;

    /**
     * @brief Get the number of geometries in the context
     * @return Number of geometries
     */
    int geometryCount() const;

    /**
     * @brief Clear all geometries from the context
     */
    void clearContext();

    /**
     * @brief Check if the parser is valid and ready to use
     * @return true if the parser is initialized
     */
    bool isValid() const;

signals:
    /**
     * @brief Emitted when a command is executed
     * @param command The command that was executed
     * @param result The result of the command
     * @param success Whether the command succeeded
     */
    void commandExecuted(const QString &command, const QString &result, bool success);

private:
    void* m_handle;  // Opaque pointer to ParserHandle
    bool m_lastSuccess;
    QString m_lastError;
    
    void updateLastResult(bool success, const QString &error = QString());
};

#endif // STDGEOPARSER_H