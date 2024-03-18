#include "romcompiler.h"
#include <common.h>
#include <msglog.h>
#include <defines.h>
#include <config.h>
#include <gba/game.h>

#include <initializer_list>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QTextStream>
#include <QApplication>

#define COMPILE_CATEGORY "ROM"

// Tools
#if _WIN32
const char* default_cc      = "win32/devkitpro/devkitARM/bin/arm-none-eabi-gcc";
const char* default_ld      = "win32/devkitpro/devkitARM/bin/arm-none-eabi-gcc";
const char* default_as      = "win32/devkitpro/devkitARM/bin/arm-none-eabi-as";
const char* default_objcopy = "win32/devkitpro/devkitARM/bin/arm-none-eabi-objcopy";
const char* default_fix     = "win32/devkitpro/tools/bin/gbafix";
#elif __linux__
const char* default_cc      = "/opt/devkitpro/devkitARM/bin/arm-none-eabi-gcc";
const char* default_ld      = "/opt/devkitpro/devkitARM/bin/arm-none-eabi-gcc";
const char* default_as      = "/opt/devkitpro/devkitARM/bin/arm-none-eabi-as";
const char* default_objcopy = "/opt/devkitpro/devkitARM/bin/arm-none-eabi-objcopy";
const char* default_fix     = "/opt/devkitpro/tools/bin/gbafix";
#endif

// Flags
const char* default_cflags    = "-Wall -fomit-frame-pointer -ffast-math -O3";
const char* default_ldflags   = "-specs=gba.specs";
const char* default_libs      = "-lm";
const char* default_includes  = "-I%{GENERATED} -I%{CODE}";
const char* default_arch      = "-mcpu=arm7tdmi";
const char* default_rom       = "%{PROJECT}%{GAME}.gba";

// Steps
const char* default_compile_step = "%{CC} %{ARCH} %{INCLUDES} %{CFLAGS} -c %{SOURCE} -o %{OBJECT}";
const char* default_assemble_step = "%{AS} %{ARCH} %{SOURCE} -o %{OBJECT}";
const char* default_link_step    = "%{LD} %{ARCH} %{LDFLAGS} %{OBJECTS} -o %{TEMP}%{GAME}.elf %{LIBS}";
const char* default_objcopy_step = "%{OBJCOPY} -O binary %{TEMP}%{GAME}.elf %{ROM}";
const char* default_fix_step     = "%{FIX} %{ROM}";

QString getDefaultValue(QString key)
{
    static QMap<QString, QString> build_defaults;
    if(build_defaults.size() == 0)
    {
        build_defaults[BUILD_CC] = default_cc;
        build_defaults[BUILD_AS] = default_as;
        build_defaults[BUILD_LD] = default_ld;
        build_defaults[BUILD_OBJCOPY] = default_objcopy;
        build_defaults[BUILD_FIX] = default_fix;
        build_defaults[BUILD_CFLAGS] = default_cflags;
        build_defaults[BUILD_LDFLAGS] = default_ldflags;
        build_defaults[BUILD_LIBS] = default_libs;
        build_defaults[BUILD_INCLUDES] = default_includes;
        build_defaults[BUILD_ARCH] = default_arch;
        build_defaults[BUILD_ROM] = default_rom;

        build_defaults[BUILD_STEP_COMPILE] = default_compile_step;
        build_defaults[BUILD_STEP_ASSEMBLE] = default_assemble_step;
        build_defaults[BUILD_STEP_LINK] = default_link_step;
        build_defaults[BUILD_STEP_OBJCOPY] = default_objcopy_step;
        build_defaults[BUILD_STEP_FIX] = default_fix_step;
    }
    if(build_defaults.contains(key))
        return build_defaults[key];
    return "";
}


void RomCompiler::resetDefaults()
{
    Config::remove(BUILD_CC);
    Config::remove(BUILD_AS);
    Config::remove(BUILD_LD);
    Config::remove(BUILD_OBJCOPY);
    Config::remove(BUILD_FIX);

    Config::remove(BUILD_CFLAGS);
    Config::remove(BUILD_LDFLAGS);
    Config::remove(BUILD_LIBS);
    Config::remove(BUILD_INCLUDES);
    Config::remove(BUILD_ARCH);
    Config::remove(BUILD_ROM);

    Config::remove(BUILD_STEP_COMPILE);
    Config::remove(BUILD_STEP_ASSEMBLE);
    Config::remove(BUILD_STEP_LINK);
    Config::remove(BUILD_STEP_OBJCOPY);
    Config::remove(BUILD_STEP_FIX);
    Config::remove(BUILD_CUSTOM);

    Config::save();
}

QString RomCompiler::getConfig(QString key)
{
    QString value = Config::get(key);
    if(value.size())
        return value;
    return getDefaultValue(key);
}

void RomCompiler::setConfig(QString key, QString value)
{
    QString default_value = getDefaultValue(key);
    if(value == default_value)
        Config::remove(key);
    else
        Config::set(key, value);
}

void RomCompiler::saveConfig()
{
    Config::save();
}

void RomCompilerWorker::run(RomCompileArgs args)
{
    this->args = args;

    QString rom_file = expandVariable("%{ROM}");
    QDir(expandVariable("%{TEMP}")).mkpath(".");

    if(args.custom_step.size())
    {
        emit log(COMPILE_CATEGORY, "Custom build...\n");
        if(!customBuild())
            emit finished(false, rom_file);

        emit finished(true, rom_file);
        return;
    }

    emit log(COMPILE_CATEGORY, "Assembling code...\n");

    if(!assemble(args.sourcefiles))
    {
        emit finished(false, rom_file);
        return;
    }

    emit log(COMPILE_CATEGORY, "Compiling code...\n");

    if(!compile(args.sourcefiles))
    {
        emit finished(false, rom_file);
        return;
    }

    emit log(COMPILE_CATEGORY, "Linking objects...\n");

    if(!link())
    {
        emit finished(false, rom_file);
        return;
    }

    emit log(COMPILE_CATEGORY, "Creating ROM...\n");

    if(!objcopy())
    {
        emit finished(false, rom_file);
        return;
    }

    emit log(COMPILE_CATEGORY, "Fixing ROM...\n");

    if(!fixup())
    {
        emit finished(false, rom_file);
        return;
    }

    emit finished(true, rom_file);
}

RomCompilerWorker::~RomCompilerWorker()
{
    QDir(expandVariable("%{TEMP}")).removeRecursively();
}

int RomCompilerWorker::runtool(QString program, const QStringList& program_args)
{
    QString programPath = program;
    QFileInfo file_info(programPath);
#ifdef _WIN32
    if(!file_info.exists() && file_info.suffix() != "exe")
    {
        if(programPath[programPath.size()-1] != '.')
            programPath += '.';
        programPath += "exe";
        file_info = QFileInfo(programPath);
    }
#endif

    if(!file_info.exists())
    {
        programPath = QApplication::applicationDirPath() + "/" + program;
        file_info = QFileInfo(programPath);
        programPath = file_info.absoluteFilePath();
    }

    const QString programName = file_info.completeBaseName();
    if(!file_info.exists())
    {
        emit error(COMPILE_CATEGORY, "Toolchain failed to find " + programName);
        emit error(COMPILE_CATEGORY, "Can not find program (" + program + ")");
        emit finished(false, expandVariable("%{ROM}"));
        return -1;
    }

#if LOG_VERBOSE
    QString cmd = program;
    foreach(const QString& arg, program_args)
    {
        cmd += " " + arg;
    }
    emit log(COMPILE_CATEGORY, cmd);
#endif

    QProcess* process = new QProcess(this);
    process->start(program, program_args);

    process->waitForFinished();

    QString programOutput = QString(process->readAllStandardOutput());
    if(programOutput.size())
    {
        emit log(programName, programOutput);
    }

    QString programError = QString(process->readAllStandardError());
    if(programError.size())
    {
        emit error(programName, programError);
    }

    int exit_code = process->exitCode();
    process->closeReadChannel(QProcess::StandardOutput);
    process->closeReadChannel(QProcess::StandardError);
    process->close();

    delete process;
    return exit_code;
}

bool RomCompilerWorker::compile(const QStringList& sourcefiles)
{
    foreach(QString sourcefile, sourcefiles)
    {
        if(QFileInfo(sourcefile).suffix() != "c")
            continue;

        QString objectfile;
        if(!getObjectFile(sourcefile, objectfile)) continue;

        args.variables["%{SOURCE}"] = sourcefile;
        args.variables["%{OBJECT}"] = objectfile;

        QString program;
        QStringList program_args;
        buildProgramCommand(args.compile_step, program, program_args);

        args.variables.remove("%{SOURCE}");
        args.variables.remove("%{OBJECT}");

        if(runtool(program, program_args) != 0)
        {
            return false;
        }

        if(!args.variables.contains("%{OBJECTS}"))
            args.variables["%{OBJECTS}"] = objectfile;
        else
            args.variables["%{OBJECTS}"] += " " + objectfile;

    }
    return true;
}

bool RomCompilerWorker::assemble(const QStringList& sourcefiles)
{
    foreach(QString sourcefile, sourcefiles)
    {
        if(QFileInfo(sourcefile).suffix() != "S")
            continue;

        QString objectfile;
        if(!getObjectFile(sourcefile, objectfile)) continue;

        args.variables["%{SOURCE}"] = sourcefile;
        args.variables["%{OBJECT}"] = objectfile;

        QString program;
        QStringList program_args;
        buildProgramCommand(args.assemble_step, program, program_args);

        args.variables.remove("%{SOURCE}");
        args.variables.remove("%{OBJECT}");

        if(runtool(program, program_args) != 0)
        {
            return false;
        }

        if(!args.variables.contains("%{OBJECTS}"))
            args.variables["%{OBJECTS}"] = objectfile;
        else
            args.variables["%{OBJECTS}"] += " " + objectfile;

    }
    return true;
}

bool RomCompilerWorker::link()
{
    QString program;
    QStringList program_args;

    buildProgramCommand(args.link_step, program, program_args);
    if(runtool(program, program_args) != 0)
    {
        return false;
    }
    return true;
}

bool RomCompilerWorker::objcopy()
{
    QString program;
    QStringList program_args;

    buildProgramCommand(args.objcopy_step, program, program_args);
    return runtool(program, program_args) == 0;
}


bool RomCompilerWorker::fixup()
{
    QString program;
    QStringList program_args;

    buildProgramCommand(args.fix_step, program, program_args);
    return runtool(program, program_args) == 0;
}

bool RomCompilerWorker::customBuild()
{
    QString program;
    QStringList program_args;

    buildProgramCommand(args.custom_step, program, program_args);
    return runtool(program, program_args) == 0;
}

bool RomCompilerWorker::getObjectFile(const QString& source_file, QString& out_object_file) const
{
    QFileInfo info(source_file);
    if(info.suffix() != "c" && info.suffix() != "s")
    {
        out_object_file = "";
        return false;
    }
    out_object_file = expandVariable("%{TEMP}") + info.baseName() + ".o";
    return true;
}

QString RomCompilerWorker::expandVariable(QString variable) const
{
    bool replaced;
    do
    {
        replaced = false;
        foreach(const QString& key, args.variables.keys())
        {
            QString prev_arg = variable;
            variable = variable.replace(key, args.variables[key]);

            if(prev_arg != variable)
            {
                replaced = true;
                break;
            }
        }
    }
    while(replaced);

    return variable;
}

void RomCompilerWorker::buildProgramCommand(const QString& command, QString& out_program, QStringList& out_args)
{
    QStringList temp_args = command.split(" ");
    foreach(QString arg, temp_args)
    {
        arg = expandVariable(arg);
        // if the arg is a file/path do not split!
        if(!QFileInfo::exists(arg))
            out_args << arg.split(" ");
        else
            out_args << arg;
    }

    if(out_args.size())
    {
        out_program = out_args.first();
        out_args.pop_front();
    }

    out_args.removeAll("");
    out_args.removeAll(" ");
}

// -------------------------- Worker ----------------------------------------

RomCompiler::~RomCompiler()
{
    thread.quit();
    thread.wait();
}

void RomCompiler::build(Game* game)
{
    static bool register_type = true;
    if(register_type)
    {
        // Allow to pass RomCompileArgs through QObject::connect
        qRegisterMetaType<RomCompileArgs>("RomCompileArgs");
        register_type = false;
    }

    RomCompilerWorker* worker = new RomCompilerWorker();
    worker->moveToThread(&thread);
    connect(this, SIGNAL(started(RomCompileArgs)), worker, SLOT(run(RomCompileArgs)));
    connect(&thread, SIGNAL(finished()), worker, SLOT(deleteLater()));

    connect(worker, SIGNAL(log(QString,QString)), this, SLOT(on_workerLog(QString,QString)));
    connect(worker, SIGNAL(warning(QString,QString)), this, SLOT(on_workerWarning(QString,QString)));
    connect(worker, SIGNAL(error(QString,QString)), this, SLOT(on_workerError(QString,QString)));
    connect(worker, SIGNAL(finished(bool,QString)), this, SLOT(on_workerFinished(bool,QString)));

    thread.start();

    RomCompileArgs args;
    setup(game, args);

    emit started(args);
}

void RomCompiler::setup(Game* game, RomCompileArgs& args)
{
    args.variables["%{TEMP}"] = QApplication::applicationDirPath() + "/temp/";
    args.variables["%{PROJECT}"] = game->getAbsoluteProjectPath();
    args.variables["%{GAME}"] = game->getName();
    args.variables["%{CODE}"] = game->getAbsoluteCodePath();
    args.variables["%{GENERATED}"] = game->getAbsoluteGeneratedPath();
    args.sourcefiles = game->getSourceFiles();

#ifdef _WIN32
    args.variables["%{OS}"]      = "win32";
#elif __linux__
    args.variables["%{OS}"]      = "linux";
#elif __APPLE__
    args.variables["%{OS}"]      = "macos";
#endif

    args.variables["%{CC}"]         = getConfig(BUILD_CC);
    args.variables["%{AS}"]         = getConfig(BUILD_AS);
    args.variables["%{LD}"]         = getConfig(BUILD_LD);
    args.variables["%{OBJCOPY}"]    = getConfig(BUILD_OBJCOPY);
    args.variables["%{FIX}"]        = getConfig(BUILD_FIX);
    args.variables["%{ARCH}"]       = getConfig(BUILD_ARCH);
    args.variables["%{CFLAGS}"]     = getConfig(BUILD_CFLAGS);
    args.variables["%{LDFLAGS}"]    = getConfig(BUILD_LDFLAGS);
    args.variables["%{LIBS}"]       = getConfig(BUILD_LIBS);
    args.variables["%{INCLUDES}"]   = getConfig(BUILD_INCLUDES);
    args.variables["%{ROM}"]        = getConfig(BUILD_ROM);

    args.compile_step  = getConfig(BUILD_STEP_COMPILE);
    args.assemble_step  = getConfig(BUILD_STEP_ASSEMBLE);
    args.link_step     = getConfig(BUILD_STEP_LINK);
    args.objcopy_step  = getConfig(BUILD_STEP_OBJCOPY);
    args.fix_step      = getConfig(BUILD_STEP_FIX);
    args.custom_step   = getConfig(BUILD_CUSTOM);
}

void RomCompiler::on_workerLog(QString category, QString log)
{
    msgLog(category) << log << "\n";
}

void RomCompiler::on_workerWarning(QString category, QString log)
{
    msgWarn(category) << log << "\n";
}

void RomCompiler::on_workerError(QString category, QString log)
{
    msgError(category) << log << "\n";
}

void RomCompiler::on_workerFinished(bool success, QString rom_file)
{
    if(success)
    {
        msgLog(COMPILE_CATEGORY) << "Created rom file...\n";
        msgLog(COMPILE_CATEGORY) << rom_file << "\n";
    }
    else
    {
        msgError(COMPILE_CATEGORY) << "Failed to create rom file...\n";
        msgError(COMPILE_CATEGORY) << rom_file << "\n";
    }

    msgLog(COMPILE_CATEGORY) << "Compilation finished\n";
    emit finished(success, rom_file);
}
