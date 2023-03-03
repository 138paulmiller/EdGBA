#include "cgen.h"
#include <QStringList>

static QString TYPE_TO_STR[] = {
    "struct",
    "char",
    "short",
    "int",
    "unsigned char",
    "unsigned short",
    "unsigned int",

    "char*",
    "short*",
    "int*",
    "unsigned char*",
    "unsigned short*",
    "unsigned int*",

    "const struct",
    "const char",
    "const short",
    "const int",
    "const unsigned char",
    "const unsigned short",
    "const unsigned int",

    "const char*",
    "const short*",
    "const int*",
    "const unsigned char*",
    "const unsigned short*",
    "const unsigned int*",
};

static int TYPE_TO_FILL[] = {
    0,
    2,4,8,
    2,4,8,

    2,4,8,
    2,4,8,

    0,
    2,4,8,
    2,4,8,

    2,4,8,
    2,4,8,
};

static bool writeHexValue(QTextStream& out, CGen::Type type, int value)
{
    int prev_width  = out.fieldWidth();
    QChar prev_pad_char = out.padChar();

    int w = TYPE_TO_FILL[type];

    out << "0x";
    out.setFieldWidth(w);
    out.setPadChar('0');
    out << right << hex << value;
    out.setFieldWidth(prev_width);
    out.setPadChar(prev_pad_char);

    return true;
}

bool CGen::readMacro(QTextStream& in, QString& id, int& value)
{
    QString line = in.readLine();
    if (line.size() == 0)
    {
        return false;
    }

    QTextStream in_line(&line);
    QString def;
    value = 1;
    in_line >> def;
    if(def != "#define")
    {
        return false;
    }

    in_line >> id >> value;
    return in_line.status() == QTextStream::Status::Ok;
}

bool CGen::writeMacro(QTextStream& out, const QString& id, int value)
{
    out << "#define "<< id << " " << value << endl;
    return true;
}

bool CGen::readMacro(QTextStream& in, QString& id, QString& value)
{
    QString line = in.readLine();
    if (line.size() == 0)
    {
        return false;
    }

    QTextStream in_line(&line);
    QString def;
    value = "";
    in_line >> def;
    if(def != "#define")
    {
        return false;
    }

    in_line >> id >> value;
    return in_line.status() == QTextStream::Status::Ok;
}

bool CGen::writeMacro(QTextStream& out, const QString& id, const QString& value)
{
    out << "#define "<< id << " " << value << endl;
    return true;
}

bool CGen::readCommentMetadata(QTextStream& in, QMap<QString, QString>& metadata)
{
    QString line = in.readLine();
    if (line.size() == 0)
    {
        return false;
    }

    QTextStream in_line(&line);
    QString comment;
    in_line >> comment;
    if(comment != "//")
    {
        return false;
    }

    QString keyvalue;
    in_line >> keyvalue;
    while(keyvalue.size())
    {
        QStringList keyvalue_list = keyvalue.split(":");
        QString key, value;
        if(keyvalue_list.size() > 0)
        {
            key = keyvalue_list.at(0);
        }
        if(keyvalue_list.size() > 1)
        {
            value = keyvalue_list.at(1);
        }
        metadata.insert(key, value);
        in_line >> keyvalue;
    }
    return true;
}

bool CGen::writeCommentMetadata(QTextStream& out, const QMap<QString, QString>& metadata)
{
    if(metadata.size() == 0)
    {
        return false;
    }

    QMapIterator<QString, QString> it(metadata);
    out << "// ";
    while (it.hasNext())
    {
        it.next();
        out << it.key() << ":" << it.value() << " ";
    }
    out << endl;
    return true;
}

bool CGen::writeArrayDecl(QTextStream& out, Type type, const QString& id)
{
    out << "extern " << TYPE_TO_STR[type] << " " << id << " [];" << endl;
    return true;
}

bool CGen::writeArrayDecl(QTextStream& out, QString type, const QString& id)
{
    out << "extern " << TYPE_TO_STR[Type::STRUCT] << " " << type << " " << id << " [];" << endl;
    return true;
}

bool CGen::readArrayDecl(QTextStream& in, QString& out_type, QString& out_id)
{
    QString line = in.readLine();
    QTextStream in_line(&line);

    QString decl, array_brackets;
    in_line >> decl;
    if(decl != "extern")
        return false;

    QString str;
    in_line >> str;
    out_type = str;
    while(str == "const" || str == "unsigned")
    {
        in_line >> str;
        out_type += " " + str;
    }

    in_line >> out_id >> array_brackets;
    if(array_brackets != "[];")
        return false;

    return true;
}

bool CGen::writeStructDecl(QTextStream& out, QString type, const QString& id)
{
    out << "extern " << TYPE_TO_STR[Type::STRUCT] << " " << type << " " << id << ";" << endl;
    return true;
}

bool CGen::writeStructDef(QTextStream& out, const QString& type, const QList<QPair<Type, QString>>& fields)
{
    out << "typedef struct " << type << endl;
    out << "{" << endl;
    foreach(auto field, fields)
    {
        Type type = field.first;
        QString id = field.second;
        out << "\t" << TYPE_TO_STR[type] << " " << id << ";" << endl;
    }
    out << "} " << type << ";" << endl;
    return true;
}

bool CGen::writeStruct(QTextStream& out, QString type, QString id, const QList<QString>& field_data)
{
    out << type << " " << id << " ={" << endl;
    foreach(QString value, field_data)
    {
        out << "\t" << value << "," << endl;
    }
    out << "};" << endl;
    return true;
}

bool CGen::readStruct(QTextStream& in, QString type, QString& id, QList<QString>& field_data)
{
    qint64 pos = in.pos();
    QTextStream::Status status = in.status();

    QString line = in.readLine();
    QTextStream in_line(&line);

    QString line_type;
    in_line >> line_type;

    if(type != line_type)
    {
        in.setStatus(status);
        in.seek(pos);
        return false;
    }

    in_line >> id;

    QString assignment;
    while(in_line.status() == QTextStream::Status::Ok)
    {
        QString t;
        in_line >> t;
        assignment += t;
    }

    if(assignment != "={")
        return false;

    bool success = true;
    while(success)
    {
        QString str;
        in >> str;
        if (str.size() == 0)
        {
            success = false;
            break;
        }
        str.remove(',');
        if(str.size() && str == "};")
        {
            break;
        }

        field_data.push_back(str);
    }
    return success;
}

namespace CGen
{
    ArrayWriter::ArrayWriter(QTextStream& out):
        out(out),
        column(0)
    {}

    void ArrayWriter::begin(Type type, const QString& id)
    {
        this->type = type;
        column = 0;

        out << TYPE_TO_STR[type] << " " << id;
        out << " []={" << endl;
        out << "    ";
    }

    void ArrayWriter::begin(QString type_str, const QString& id)
    {
        this->type = Type::STRUCT;
        column = 0;

        out <<  TYPE_TO_STR[type] << " " << type_str << " " << id;
        out << " []={" << endl;
    }

    void ArrayWriter::end()
    {
        out << endl << "};" << endl;
    }

    bool ArrayWriter::writeValue(int value)
    {
        writeHexValue(out, type, value);
        out << ", ";

        column++;
        if (column >= 9)
        {
            column = 0;
            out << endl << "    ";
        }
        return true;
    }

    bool ArrayWriter::writeValue(const QString& value)
    {
        out << value << "," << endl;
        return true;
    }

    ArrayReader::ArrayReader(QTextStream& in)
        :in(in)
        ,terminated(false)
    {
    }

    bool ArrayReader::begin(Type type, QString& id)
    {
        terminated = false;
        QString line = "";
        while(line == "")
        {
            if (!in.readLineInto(&line))
            {
                return false;
            }
        }

        QTextStream in_line(&line);
        QString str;
        in_line >> str;

        QString type_str = str;
        while(str == "const" || str == "unsigned")
        {
            in_line >> str;
            type_str += " " + str;
        }
        if(type_str != TYPE_TO_STR[type])
        {
            return false;
        }
        in_line >> id;

        QString assignment;
        while(in_line.status() == QTextStream::Status::Ok)
        {
            QString t;
            in_line >> t;
            assignment += t;
        }

        if(assignment != "[]={")
        {
            return false;
        }
        return true;
    }

    bool ArrayReader::end()
    {
        if(terminated)
        {
            return true;
        }
        QString line;
        while(in.readLineInto(&line))
        {
            if(line == "};")
            {
                return true;
            }
        }
        return false;
    }

    int ArrayReader::readValue(bool &success)
    {
        if(terminated)
        {
            success = false;
            return 0;
        }
        QString str;
        in >> str;
        if (str.size() == 0)
        {
            success = false;
            return 0;
        }
        str.remove(',');
        if(str == "};")
        {
            terminated = true;
            success = false;
            return 0;
        }
        return str.toInt(&success, 16);
    }
}

