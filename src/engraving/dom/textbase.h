/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __TEXTBASE_H__
#define __TEXTBASE_H__

#include <variant>

#include "modularity/ioc.h"

#include "draw/fontmetrics.h"
#include "draw/types/color.h"
#include "iengravingfontsprovider.h"

#include "engravingitem.h"
#include "property.h"
#include "types.h"

namespace mu::engraving {
class TextBase;
class TextBlock;

//---------------------------------------------------------
//   FrameType
//---------------------------------------------------------

enum class FrameType : char {
    NO_FRAME, SQUARE, CIRCLE
};

//---------------------------------------------------------
//   VerticalAlignment
//---------------------------------------------------------

enum class VerticalAlignment : signed char {
    AlignUndefined = -1, AlignNormal, AlignSuperScript, AlignSubScript
};

//---------------------------------------------------------
//   FormatId
//---------------------------------------------------------

enum class FormatId : char {
    Bold, Italic, Underline, Strike, Valign, FontSize, FontFamily
};

using FormatValue = std::variant<std::monostate, bool, int, double, String>;

//---------------------------------------------------------
//   MultiClick
//---------------------------------------------------------

enum class SelectTextType : char {
    Word, All
};

//---------------------------------------------------------
//   CharFormat
//---------------------------------------------------------

class CharFormat
{
    FontStyle _style          { FontStyle::Normal };
    VerticalAlignment _valign { VerticalAlignment::AlignNormal };
    double _fontSize           { 12.0 };
    String _fontFamily;

public:
    CharFormat() {}
    CharFormat(const CharFormat& cf) { *this = cf; }
    bool operator==(const CharFormat& cf) const;
    CharFormat& operator=(const CharFormat& cf);

    FontStyle style() const { return _style; }
    void setStyle(FontStyle s) { _style = s; }
    bool bold() const { return _style & FontStyle::Bold; }
    bool italic() const { return _style & FontStyle::Italic; }
    bool underline() const { return _style & FontStyle::Underline; }
    bool strike() const { return _style & FontStyle::Strike; }
    void setBold(bool val) { _style = val ? _style + FontStyle::Bold : _style - FontStyle::Bold; }
    void setItalic(bool val) { _style = val ? _style + FontStyle::Italic : _style - FontStyle::Italic; }
    void setUnderline(bool val) { _style = val ? _style + FontStyle::Underline : _style - FontStyle::Underline; }
    void setStrike(bool val) { _style = val ? _style + FontStyle::Strike : _style - FontStyle::Strike; }

    VerticalAlignment valign() const { return _valign; }
    double fontSize() const { return _fontSize; }
    String fontFamily() const { return _fontFamily; }
    void setValign(VerticalAlignment val) { _valign = val; }
    void setFontSize(double val) { _fontSize = val; }
    void setFontFamily(const String& val) { _fontFamily = val; }

    FormatValue formatValue(FormatId) const;
    void setFormatValue(FormatId, const FormatValue& val);
};

//---------------------------------------------------------
//   TextCursor
//    Contains current position and start of selection
//    during editing.
//---------------------------------------------------------

class TextCursor
{
    TextBase* _text;
    CharFormat _format;
    size_t _row           { 0 };
    size_t _column        { 0 };
    size_t _selectLine    { 0 };           // start of selection
    size_t _selectColumn  { 0 };
    bool _editing { false };

public:
    enum class MoveOperation {
        Start,
        Up,
        StartOfLine,
        Left,
        WordLeft,
        End,
        Down,
        EndOfLine,
        NextWord,
        Right
    };

    enum class MoveMode {
        MoveAnchor,
        KeepAnchor
    };

    TextCursor(TextBase* t)
        : _text(t) {}

    TextBase* text() const { return _text; }
    bool hasSelection() const { return (_selectLine != _row) || (_selectColumn != _column); }
    void clearSelection();
    void endEdit();
    void startEdit();
    bool editing() const { return _editing; }

    CharFormat* format() { return &_format; }
    const CharFormat* format() const { return &_format; }
    void setFormat(const CharFormat& f) { _format = f; }

    size_t row() const { return _row; }
    size_t column() const { return _column; }
    size_t selectLine() const { return _selectLine; }
    size_t selectColumn() const { return _selectColumn; }
    void setRow(size_t val) { _row = val; }
    void setColumn(size_t val) { _column = val; }
    void setSelectLine(size_t val) { _selectLine = val; }
    void setSelectColumn(size_t val) { _selectColumn = val; }
    size_t columns() const;
    void init();

    struct Range {
        int startPosition = 0;
        int endPosition = 0;
        String text;
    };

    std::pair<size_t, size_t> positionToLocalCoord(int position) const;

    int currentPosition() const;
    Range selectionRange() const;

    const TextBlock& curLine() const;
    TextBlock& curLine();

    mu::RectF cursorRect() const;
    bool movePosition(TextCursor::MoveOperation op, TextCursor::MoveMode mode = TextCursor::MoveMode::MoveAnchor, int count = 1);
    void selectWord();
    void moveCursorToEnd() { movePosition(TextCursor::MoveOperation::End); }
    void moveCursorToStart() { movePosition(TextCursor::MoveOperation::Start); }
    Char currentCharacter() const;
    bool set(const mu::PointF& p, TextCursor::MoveMode mode = TextCursor::MoveMode::MoveAnchor);
    String selectedText(bool withFormat = false) const;
    String extractText(int r1, int c1, int r2, int c2, bool withFormat = false) const;
    void updateCursorFormat();
    void setFormat(FormatId, FormatValue val);
    void changeSelectionFormat(FormatId id, const FormatValue& val);
    const CharFormat selectedFragmentsFormat() const;

private:
    Range range(int start, int end) const;
    int position(int row, int column) const;
};

//---------------------------------------------------------
//   TextFragment
//    contains a styled text
//---------------------------------------------------------

class TextFragment
{
    INJECT_STATIC(IEngravingFontsProvider, engravingFonts)
public:
    mutable CharFormat format;
    mu::PointF pos;                    // y is relative to TextBlock->y()
    mutable String text;

    bool operator ==(const TextFragment& f) const;

    TextFragment();
    TextFragment(const String& s);
    TextFragment(TextCursor*, const String&);
    TextFragment split(int column);
    void draw(mu::draw::Painter*, const TextBase*) const;
    mu::draw::Font font(const TextBase*) const;
    int columns() const;
    void changeFormat(FormatId id, const FormatValue& data);
};

//---------------------------------------------------------
//   TextBlock
//    represents a block of formatted text
//---------------------------------------------------------

class TextBlock
{
    std::list<TextFragment> _fragments;
    double _y = 0;
    double _lineSpacing = 0.0;
    mu::RectF _bbox;
    bool _eol = false;

    void simplify();

public:
    TextBlock() {}
    bool operator ==(const TextBlock& x) const { return _fragments == x._fragments; }
    bool operator !=(const TextBlock& x) const { return _fragments != x._fragments; }
    void draw(mu::draw::Painter*, const TextBase*) const;
    void layout(const TextBase*);
    const std::list<TextFragment>& fragments() const { return _fragments; }
    std::list<TextFragment>& fragments() { return _fragments; }
    std::list<TextFragment> fragmentsWithoutEmpty();
    const mu::RectF& boundingRect() const { return _bbox; }
    mu::RectF boundingRect(int col1, int col2, const TextBase*) const;
    size_t columns() const;
    void insert(TextCursor*, const String&);
    void insertEmptyFragmentIfNeeded(TextCursor*);
    void removeEmptyFragment();
    String remove(int column, TextCursor*);
    String remove(int start, int n, TextCursor*);
    int column(double x, TextBase*) const;
    TextBlock split(int column, TextCursor* cursor);
    double xpos(size_t col, const TextBase*) const;
    const CharFormat* formatAt(int) const;
    const TextFragment* fragment(int col) const;
    std::list<TextFragment>::iterator fragment(int column, int* rcol, int* ridx);
    double y() const { return _y; }
    void setY(double val) { _y = val; }
    double lineSpacing() const { return _lineSpacing; }
    String text(int, int, bool = false) const;
    bool eol() const { return _eol; }
    void setEol(bool val) { _eol = val; }
    void changeFormat(FormatId, const FormatValue& val, int start, int n);
};

//---------------------------------------------------------
//   TextBase
//---------------------------------------------------------

class TextBase : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, TextBase)

    INJECT(IEngravingFontsProvider, engravingFonts)

public:

    ~TextBase();

    virtual bool mousePress(EditData&) override;

    Text& operator=(const Text&) = delete;

    virtual void drawEditMode(mu::draw::Painter* p, EditData& ed, double currentViewScaling) override;
    static void drawTextWorkaround(mu::draw::Painter* p, mu::draw::Font& f, const mu::PointF& pos, const String& text);

    Align align() const { return m_align; }
    void setAlign(Align a) { m_align = a; }

    static String plainToXmlText(const String& s) { return s.toXmlEscaped(); }
    void setPlainText(const String& t) { setXmlText(plainToXmlText(t)); }
    virtual void setXmlText(const String&);
    void setXmlText(const char* str) { setXmlText(String::fromUtf8(str)); }
    void checkCustomFormatting(const String&);
    String xmlText() const;
    String plainText() const;
    void resetFormatting();

    void insertText(EditData&, const String&);

    double lineSpacing() const;
    double lineHeight() const;
    virtual double baseLine() const override;

    bool empty() const { return xmlText().isEmpty(); }
    void clear() { setXmlText(String()); }

    FontStyle fontStyle() const;
    String family() const;
    double size() const;

    void setFontStyle(const FontStyle& val);
    void setFamily(const String& val);
    void setSize(const double& val);

    virtual void startEdit(EditData&) override;
    virtual bool isEditAllowed(EditData&) const override;
    virtual bool edit(EditData&) override;
    virtual void editCut(EditData&) override;
    virtual void editCopy(EditData&) override;
    virtual void endEdit(EditData&) override;
    void movePosition(EditData&, TextCursor::MoveOperation);

    bool deleteSelectedText(EditData&);

    void selectAll(TextCursor*);
    void select(EditData&, SelectTextType);
    bool isPrimed() const { return m_primed; }
    void setPrimed(bool primed) { m_primed = primed; }

    virtual void paste(EditData& ed, const String& txt);

    mu::RectF pageRectangle() const;

    void dragTo(EditData&);

    std::vector<mu::LineF> dragAnchorLines() const override;

    virtual bool acceptDrop(EditData&) const override;
    virtual EngravingItem* drop(EditData&) override;

    friend class TextBlock;
    friend class TextFragment;

    static String unEscape(String s);
    static String escape(String s);

    String accessibleInfo() const override;
    String screenReaderInfo() const override;

    int subtype() const override;
    TranslatableString subtypeUserName() const override;

    std::list<TextFragment> fragmentList() const;   // for MusicXML formatted export

    static bool validateText(String& s);
    bool inHexState() const { return m_hexState >= 0; }
    void endHexState(EditData&);

    mu::draw::Font font() const;
    mu::draw::FontMetrics fontMetrics() const;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;
    PropertyValue propertyDefault(Pid id) const override;
    void undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps) override;
    Sid getPropertyStyle(Pid) const override;
    void styleChanged() override;
    void editInsertText(TextCursor*, const String&);

    TextCursor* cursorFromEditData(const EditData&);
    TextCursor* cursor() const { return m_cursor; }

    void setTextInvalid() { m_textInvalid = true; }
    bool isTextInvalid() const { return m_textInvalid; }

    // helper functions
    bool hasFrame() const { return m_frameType != FrameType::NO_FRAME; }
    bool circle() const { return m_frameType == FrameType::CIRCLE; }
    bool square() const { return m_frameType == FrameType::SQUARE; }

    TextStyleType textStyleType() const { return m_textStyleType; }
    void setTextStyleType(TextStyleType id) { m_textStyleType = id; }
    void initTextStyleType(TextStyleType id);
    void initTextStyleType(TextStyleType id, bool preserveDifferent);
    virtual void initElementStyle(const ElementStyle*) override;

    static const String UNDEFINED_FONT_FAMILY;
    static const double UNDEFINED_FONT_SIZE;

    bool bold() const { return fontStyle() & FontStyle::Bold; }
    bool italic() const { return fontStyle() & FontStyle::Italic; }
    bool underline() const { return fontStyle() & FontStyle::Underline; }
    bool strike() const { return fontStyle() & FontStyle::Strike; }
    void setBold(bool val) { setFontStyle(val ? fontStyle() + FontStyle::Bold : fontStyle() - FontStyle::Bold); }
    void setItalic(bool val) { setFontStyle(val ? fontStyle() + FontStyle::Italic : fontStyle() - FontStyle::Italic); }
    void setUnderline(bool val)
    {
        setFontStyle(val ? fontStyle() + FontStyle::Underline : fontStyle() - FontStyle::Underline);
    }

    void setStrike(bool val)
    {
        setFontStyle(val ? fontStyle() + FontStyle::Strike : fontStyle() - FontStyle::Strike);
    }

    bool hasCustomFormatting() const;

    friend class TextCursor;
    using EngravingObject::undoChangeProperty;

    mu::draw::Color textColor() const;
    FrameType frameType() const { return m_frameType; }
    void setFrameType(FrameType val) { m_frameType = val; }
    double textLineSpacing() const { return m_textLineSpacing; }
    void setTextLineSpacing(double val) { m_textLineSpacing = val; }
    mu::draw::Color bgColor() const { return m_bgColor; }
    void setBgColor(const mu::draw::Color& val) { m_bgColor = val; }
    mu::draw::Color frameColor() const { return m_frameColor; }
    void setFrameColor(const mu::draw::Color& val) { m_frameColor = val; }
    Spatium frameWidth() const { return m_frameWidth; }
    void setFrameWidth(Spatium val) { m_frameWidth = val; }
    Spatium paddingWidth() const { return m_paddingWidth; }
    void setPaddingWidth(Spatium val) { m_paddingWidth = val; }
    int frameRound() const { return m_frameRound; }
    void setFrameRound(int val) { m_frameRound = val; }

    struct LayoutData : public EngravingItem::LayoutData {
        std::vector<TextBlock> blocks;
        bool layoutInvalid = true;

        mu::RectF frame;

        size_t rows() const { return blocks.size(); }
        const TextBlock& textBlock(size_t i) const { return blocks.at(i); }
        TextBlock& textBlock(size_t i) { return blocks[i]; }
    };
    DECLARE_LAYOUTDATA_METHODS(TextBase);

    void createBlocks();
    void createBlocks(LayoutData* ldata) const;
    void layoutFrame();
    void layoutFrame(LayoutData* ldata) const;

    //! NOTE It can only be set for some types of text, see who has the setter.
    //! At the moment it's: Text, Jump, Marker
    bool layoutToParentWidth() const { return m_layoutToParentWidth; }

protected:
    TextBase(const ElementType& type, EngravingItem* parent = 0, TextStyleType tid = TextStyleType::DEFAULT,
             ElementFlags = ElementFlag::NOTHING);
    TextBase(const ElementType& type, EngravingItem* parent, ElementFlags);
    TextBase(const TextBase&);

    void insertSym(EditData& ed, SymId id);
    void prepareFormat(const String& token, TextCursor& cursor);
    bool prepareFormat(const String& token, CharFormat& format);

    virtual void commitText();

    bool m_layoutToParentWidth = false;

private:

    void drawSelection(mu::draw::Painter*, const mu::RectF&) const;
    void insert(TextCursor*, char32_t code, LayoutData* ldata) const;
    String genText(const LayoutData* ldata) const;
    void genText();
    virtual int getPropertyFlagsIdx(Pid id) const override;
    String stripText(bool, bool, bool) const;
    Sid offsetSid() const;

    static String getHtmlStartTag(double, double&, const String&, String&, FontStyle, VerticalAlignment);
    static String getHtmlEndTag(FontStyle, VerticalAlignment);

#ifndef ENGRAVING_NO_ACCESSIBILITY
    AccessibleItemPtr createAccessible() override;
#endif

    void notifyAboutTextCursorChanged();
    void notifyAboutTextInserted(int startPosition, int endPosition, const String& text);
    void notifyAboutTextRemoved(int startPosition, int endPosition, const String& text);

    Align m_align;

    FrameType m_frameType = FrameType::NO_FRAME;
    double m_textLineSpacing = 1.0;
    mu::draw::Color m_bgColor;
    mu::draw::Color m_frameColor;
    Spatium m_frameWidth;
    Spatium m_paddingWidth;
    int m_frameRound = 0;

    // there are two representations of text; only one
    // might be valid and the other can be constructed from it

    String m_text;                          // cached
    bool m_textInvalid = true;

    TextStyleType m_textStyleType = TextStyleType::DEFAULT;           // text style id

    int m_hexState = -1;
    bool m_primed = 0;

    TextCursor* m_cursor = nullptr;
};

inline bool isTextNavigationKey(int key, KeyboardModifiers modifiers)
{
    // space + TextEditingControlModifier = insert nonbreaking space, so that's *not* a navigation key
    return (key == Key_Space && modifiers != TextEditingControlModifier) || key == Key_Tab;
}
} // namespace mu::engraving

#endif
