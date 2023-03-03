#include "config.h"

#include <QSettings>
#include <QApplication>


QMap<QString, QString> Config::data;
bool Config::is_loaded = false;

void Config::save()
{
    QSettings settings(QApplication::applicationDirPath() + "/edgba.ini", QSettings::IniFormat);
    settings.clear();
    settings.beginGroup("User");
    QMap<QString, QString>::const_iterator i = data.constBegin();
    while (i != data.constEnd())
    {
         settings.setValue(i.key(), i.value());
         ++i;
     }
    settings.endGroup();
}

void Config::load()
{
    is_loaded = true;
    data.clear();

    QSettings settings(QApplication::applicationDirPath() + "/edgba.ini", QSettings::IniFormat);
    settings.beginGroup("User");
    QStringList keys = settings.childKeys();
    foreach (QString key, keys)
    {
         data.insert(key, settings.value(key).toString());
    }
    settings.endGroup();
}

void Config::set(const QString& key, const QString& value)
{
    data.insert(key, value);
}

void Config::remove(const QString& key)
{
    data.remove(key);
}

QString Config::get(const QString& key)
{
    if(!is_loaded)
    {
        load();
    }

    QMap<QString, QString>::const_iterator i = data.find(key);

    if(i != data.constEnd())
    {
        return *i;
    }
    return "";
}
