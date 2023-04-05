#ifndef ROMCOMPILER_H
#define ROMCOMPILER_H

#include <QDebug>
#include <QString>
#include <QThread>

#define BUILD_CC       "BUILD_CC"
#define BUILD_AS       "BUILD_AS"
#define BUILD_LD       "BUILD_LD"
#define BUILD_OBJCOPY  "BUILD_OBJCOPY"
#define BUILD_FIX      "BUILD_FIX"
#define BUILD_CFLAGS   "BUILD_CFLAGS"
#define BUILD_LDFLAGS  "BUILD_LDFLAGS"
#define BUILD_LIBS     "BUILD_LIBS"
#define BUILD_INCLUDES "BUILD_INCLUDES"
#define BUILD_ARCH     "BUILD_ARCH"
#define BUILD_ROM      "BUILD_ROM"
#define BUILD_STEP_COMPILE  "BUILD_STEP_CC"
#define BUILD_STEP_ASSEMBLE "BUILD_STEP_AS"
#define BUILD_STEP_LINK     "BUILD_STEP_LD"
#define BUILD_STEP_OBJCOPY  "BUILD_STEP_OBJCOPY"
#define BUILD_STEP_FIX      "BUILD_STEP_FIX"
#define BUILD_CUSTOM      "BUILD_CUSTOM"

// TODO: create options to manually link against existing libs
struct RomCompileArgs
{
    // Variables
    QMap<QString, QString> variables;
    QStringList sourcefiles;   // %{SOURCES}, also provides %{SOURCE} and %{OBJECT} for link and compile

    QString compile_step;
    QString assemble_step;
    QString link_step;
    QString objcopy_step;
    QString fix_step;
    QString custom_step;
};

Q_DECLARE_METATYPE(RomCompileArgs);

class RomCompilerWorker : public QObject
{
    Q_OBJECT
private:

    RomCompileArgs args;

public:
    ~RomCompilerWorker();

signals:
    void finished(bool success, QString rom_file);
    void log(QString category, QString log);
    void warning(QString category, QString log);
    void error(QString category, QString log);

public slots:
    void run(RomCompileArgs buildData);

private:
    bool setup();

    bool compile(const QStringList& sourcefiles);
    bool assemble(const QStringList& sourcefiles);
    bool link();
    bool objcopy();
    bool fixup();

    bool customBuild();

    int runtool(QString program, const QStringList& args);

    bool getObjectFile(const QString& source_file, QString& out_object_file) const;
    QString expandVariable(QString variable) const;
    void buildProgramCommand(const QString& command, QString& out_program, QStringList& out_args);
};

class Game;
class RomCompiler : public QObject
{
    Q_OBJECT
private:
    friend class RomCompilerWorker;
    QThread thread;

public:
    ~RomCompiler();

    static void resetDefaults();
    static QString getConfig(QString key);
    static void setConfig(QString key, QString value);
    static void saveConfig();

    void build(Game* game);

private:
    void setup(Game* game, RomCompileArgs& args);

signals:
    void started(RomCompileArgs args);
    void finished(bool success, QString rom_file);

private slots:
    void on_workerLog(QString category, QString log);
    void on_workerWarning(QString category, QString log);
    void on_workerError(QString category, QString log);
    void on_workerFinished(bool success, QString rom_file);

};


#endif // ROMCOMPILER_H
