#ifndef CODEVIEW_H
#define CODEVIEW_H

#include <QThread>
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class CodeView;

class CodeViewGutter : public QWidget
{
public:
    CodeViewGutter(CodeView *code_view);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    CodeView *code_view;
};

class CodeHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    CodeHighlighter(CodeView* code_view, QTextDocument *parent = nullptr);
    void reset();

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QList<HighlightingRule> highlight_rules;
    QRegularExpression comment_start_expression;
    QRegularExpression comment_end_expression;
    QTextCharFormat multiline_comment_format;
    CodeView* code_view;
};

class CodeView: public QPlainTextEdit
{
    Q_OBJECT

private:
    QString file_path;
    CodeViewGutter * gutter;
    const int gutter_margin = 10;
    CodeHighlighter* highlighter;

public:
    CodeView(QWidget *parent = nullptr);
    ~CodeView();

    void setupFont();
    void gutterPaintEvent(QPaintEvent *event);
    int gutterWidth();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void setText(const QString& text);
    void updateGutterWidth(int newBlockCount);
    void updateGutter(const QRect &rect, int dy);
};

#endif // CODEVIEW_H
