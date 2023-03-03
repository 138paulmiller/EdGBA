#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

class Config
{
public:
    static void save();
    static void load();

    static void remove(const QString& key);
    static void set(const QString& key, const QString& value);
    static QString get(const QString& key);

private:
    static QMap<QString, QString> data;
    static bool is_loaded;
};

#endif // CONFIG_H
