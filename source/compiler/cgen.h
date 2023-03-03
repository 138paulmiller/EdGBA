#ifndef CGEN_H
#define CGEN_H

#include <iostream>
#include <QMap>
#include <QString>
#include <QVector>
#include <QTextStream>

namespace CGen
{
    enum Type
    {
        STRUCT = 0,
        CHAR,
        SHORT,
        INT,
        UNSIGNED_CHAR,
        UNSIGNED_SHORT,
        UNSIGNED_INT,

        PTR_CHAR,
        PTR_SHORT,
        PTR_INT,
        PTR_UNSIGNED_CHAR,
        PTR_UNSIGNED_SHORT,
        PTR_UNSIGNED_INT,

        CONST_STRUCT,
        CONST_CHAR,
        CONST_SHORT,
        CONST_INT,
        CONST_UNSIGNED_CHAR,
        CONST_UNSIGNED_SHORT,
        CONST_UNSIGNED_INT,

        CONST_PTR_CHAR,
        CONST_PTR_SHORT,
        CONST_PTR_INT,
        CONST_PTR_UNSIGNED_CHAR,
        CONST_PTR_UNSIGNED_SHORT,
        CONST_PTR_UNSIGNED_INT,
    };

    // Read/write a macro to int literal
    bool readMacro(QTextStream& in, QString& id, int& value);
    bool writeMacro(QTextStream& out, const QString& id, int value);

    // Read/Write a macro to a variable identifier
    bool readMacro(QTextStream& in, QString& id, QString& value);
    bool writeMacro(QTextStream& out, const QString& id, const QString& value);

    // key:value key:value etc...
    bool readCommentMetadata(QTextStream& in, QMap<QString, QString>& metadata);
    bool writeCommentMetadata(QTextStream& out, const QMap<QString, QString>& metadata);

    bool writeArrayDecl(QTextStream& out, Type type, const QString& id);
    bool writeArrayDecl(QTextStream& out, QString type, const QString& id);
    bool readArrayDecl(QTextStream& out, QString& out_type, QString& out_id);

    bool writeStructDecl(QTextStream& out, QString type, const QString& id);
    bool writeStructDef(QTextStream& out, const QString& type, const QList<QPair<Type, QString>>& fields);

    bool writeStruct(QTextStream& out, QString type, QString id, const QList<QString>& field_data);
    bool readStruct(QTextStream& in, QString type, QString& id, QList<QString>& field_data);

    struct ArrayWriter
    {
    private:
        QTextStream& out;
        Type type;
        int column;

    public:
        ArrayWriter(QTextStream& out);
        void begin(Type type, const QString& id);
        void begin(QString type, const QString& id);
        void end();
        bool writeValue(int value);
        bool writeValue(const QString& value);

        template <typename ElementType>
        bool writeAllValues(Type type, const QString& id, QVector<ElementType>& values)
        {
            begin(type, id);
            for(int i =0; i < values.size(); ++i)
            {
                writeValue(values[i]);
            }
            end();
            return true;
        }

        template <typename ElementType>
        bool writeAllValues(QString type, const QString& id, QVector<ElementType>& values)
        {
            begin(type, id);
            for(int i =0; i < values.size(); ++i)
            {
                writeValue(values[i]);
            }
            end();
            return true;
        }
    };

    struct ArrayReader
    {
    private:
        QTextStream& in;
        bool terminated;

    public:
        ArrayReader(QTextStream& in);
        bool begin(Type type, QString& id);
        bool end();
        int readValue(bool &success);

        template <typename ElementType>
        bool readAllValues(Type type, QString& id, QVector<ElementType>& values)
        {
            if(!begin(type, id))
            {
                return false;
            }
            bool success;
            ElementType value = readValue(success);
            while(success)
            {
                values.append(value);
                value = readValue(success);
            }
            return end();
        }
    };

}

#endif // CGEN_H
