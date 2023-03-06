#include "codeview.h"
#include <QPainter>
#include <QTextBlock>
#include <QTextStream>
#include <QFontMetricsF>

// TODO: make customizable
struct SyntaxConfig
{
    SyntaxConfig(QColor color = QColor(), QString pattern="", bool is_bold = false)
        : color(color)
        , pattern(pattern)
        , is_bold(is_bold)
    {}
    QColor color;
    QString pattern;
    bool is_bold = false;
};

// TODO: build a user keywords pattern for any typedefs in the game code
const int tab_width = 4;
static const char* keywords_pattern =
    "(#define|#include|#ifdef|#ifndef|#if|#else|#elif|#end)|\\b(auto|break|case|char|const|continue|default|do|double|else|enum|extern|float|for|goto|if|inline|int|long|register|return|short|signed|sizeof|static|struct|switch|typedef|union|unsigned|void|volatile|while)\\b"
;

static QMap<QString, SyntaxConfig>& getSyntaxConfig()
{
    // NOTE: prefix denote priority. 0 is lowest priority
    static QMap<QString, SyntaxConfig> syntax_config;
    if(syntax_config.size() == 0)
    {
        syntax_config["6_comment"]       = SyntaxConfig( QColor(0x6A, 0x99, 0x49), "//[^\n]*");
        syntax_config["5_string"]        = SyntaxConfig( QColor(0xD9, 0x82, 0x00), "\".*\"");
        syntax_config["4_keywords"]      = SyntaxConfig( QColor(0x35, 0x8C, 0xD6), keywords_pattern );
        syntax_config["3_include"]       = SyntaxConfig( QColor(0xD9, 0x82, 0x00), "#include ?(\"|<).+(\"|>)");
        syntax_config["3_define"]        = SyntaxConfig( QColor(0x84, 0xC3, 0xAD), "#define ?(.+\\\\n)*(.+\\n)");
        syntax_config["2_number"]        = SyntaxConfig( QColor(0xA7, 0xCE, 0xA8), "(?=.)((0x|0b)?([0-9]*)(\\.([0-9]+)?(f|i)?)?)");
        syntax_config["2_function"]      = SyntaxConfig( QColor(0xDC, 0xDC, 0x9D), "\\b[A-Za-z0-9_]+(?=\\()");
        syntax_config["1_identifier"]    = SyntaxConfig( QColor(0x84, 0xC3, 0xAD), "\\w[\\w\\d_]*");
        syntax_config["0_left_bracket"]  = SyntaxConfig( QColor(0xA4, 0x63, 0xD6), "{"  );
        syntax_config["0_right_bracket"] = SyntaxConfig( QColor(0xA4, 0x63, 0xD6), "}"  );
        syntax_config["0_left_brace"]    = SyntaxConfig( QColor(0xA4, 0x63, 0xD6), "\\[");
        syntax_config["0_right_brace"]   = SyntaxConfig( QColor(0xA4, 0x63, 0xD6), "\\]");
        syntax_config["0_left_paren"]    = SyntaxConfig( QColor(0xF1, 0xD7, 0x10), "\\(");
        syntax_config["0_right_paren"]   = SyntaxConfig( QColor(0xF1, 0xD7, 0x10), "\\)");
    }
    return syntax_config;
}

// TODO: build a list of known struct definitions. Create a global lookup

CodeViewGutter::CodeViewGutter(CodeView *code_view)
    : QWidget(code_view), code_view(code_view)
{}

QSize CodeViewGutter::sizeHint() const
{
    return QSize(code_view->gutterWidth(), 0);
}

void CodeViewGutter::paintEvent(QPaintEvent *event)
{
    code_view->gutterPaintEvent(event);
}

// ---------------------------- Code Highligher --------------------------------//

CodeHighlighter::CodeHighlighter(CodeView* code_view, QTextDocument *parent)
    : QSyntaxHighlighter(parent)
    , code_view(code_view)
{
    reset();
}

void CodeHighlighter::reset()
{
    highlight_rules.clear();
    const QList<QString>& syntax_keys = getSyntaxConfig().keys();
    foreach(const QString& id, syntax_keys)
    {
        SyntaxConfig config = getSyntaxConfig()[id];
        HighlightingRule rule;
        QTextCharFormat format;
        format.setFontWeight(config.is_bold ? QFont::Bold : QFont::Normal);
        format.setForeground(config.color);
        rule.pattern = QRegularExpression(config.pattern);
        rule.format = format;
        highlight_rules.append(rule);
    }
    multiline_comment_format.setForeground(getSyntaxConfig()["6_comment"].color);
    comment_start_expression = QRegularExpression(QStringLiteral("/\\*"));
    comment_end_expression = QRegularExpression(QStringLiteral("\\*/"));
}

void CodeHighlighter::highlightBlock(const QString &text)
{
    //if(code_view->visibleRegion().isEmpty()) return;

    foreach(const HighlightingRule &rule, highlight_rules)
    {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext())
        {
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // multi-line comment
    setCurrentBlockState(0);

    int start_index = 0;
    if (previousBlockState() != 1)
        start_index = text.indexOf(comment_start_expression);

    while (start_index >= 0)
    {
        QRegularExpressionMatch match = comment_end_expression.match(text, start_index);
        int end_index = match.capturedStart();
        int comment_len = 0;
        if (end_index == -1)
        {
            setCurrentBlockState(1);
            comment_len = text.length() - start_index;
        }
        else
        {
            comment_len = end_index - start_index
                            + match.capturedLength();
        }
        setFormat(start_index, comment_len, multiline_comment_format);
        start_index = text.indexOf(comment_start_expression, start_index + comment_len);
    }
}

// ---------------------------- Code View --------------------------------//

CodeView::CodeView(QWidget *parent)
    : QPlainTextEdit(parent)
    , gutter(new CodeViewGutter(this))
    , highlighter(new CodeHighlighter(this, document()))
{
    connect(this, &CodeView::blockCountChanged, this, &CodeView::updateGutterWidth);
    connect(this, &CodeView::updateRequest, this, &CodeView::updateGutter);
    connect(this, &CodeView::cursorPositionChanged, this, &CodeView::highlightCurrentLine);

    setupFont();
    updateGutterWidth(0);
    highlightCurrentLine();
}

CodeView::~CodeView()
{
    delete highlighter;
}

void CodeView::setText(const QString& text)
{
    textCursor().setPosition(0);
    updateGutterWidth(0);
    highlightCurrentLine();
    setPlainText(text);
}

int CodeView::gutterWidth()
{
    // estimate count number of digits
    int digits = 1;
    int lines = blockCount();

    while (lines != 0)
    {
        lines = lines / 10;
        ++digits;
    }

    int char_width = fontMetrics().charWidth("9", 0);
//    int char_width = fontMetrics().horizontalAdvance(QLatin1Char('9'));
    return qMax(5, digits) * char_width;
}

void CodeView::updateGutterWidth(int /* newBlockCount */)
{
    setViewportMargins(gutterWidth() + gutter_margin, 0, 0, 0);
}

void CodeView::updateGutter(const QRect &rect, int dy)
{
    if (dy)
    {
        gutter->scroll(0, dy);
    }
    else
    {
        gutter->update(0, rect.y(), gutter->width(), rect.height());
    }
    if (rect.contains(viewport()->rect()))
    {
        updateGutterWidth(0);
    }
}

void CodeView::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    gutter->setGeometry(QRect(cr.left(), cr.top(), gutterWidth(), cr.height()));
}

void CodeView::highlightCurrentLine()
{
    return;
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly() && !document()->isEmpty()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::blue).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void CodeView::setupFont()
{
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    setFont(font);
    setTabStopWidth(QFontMetricsF(font).averageCharWidth() * tab_width);
}

void CodeView::gutterPaintEvent(QPaintEvent *event)
{
    QPainter painter(gutter);
    painter.fillRect(event->rect(), Qt::transparent);
    painter.setFont(font());
    QTextBlock block = firstVisibleBlock();
    int block_number = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    int width = gutter->width();
    int height = fontMetrics().height();

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            QString number = QString::number(block_number + 1);
            painter.setPen(palette().text().color());
            painter.drawText(0, top, width, height, Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++block_number;
    }
}
