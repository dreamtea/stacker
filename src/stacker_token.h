#pragma once

#include <cstdint>

namespace stkr {

enum Token {
	TOKEN_INVALID,
	TOKEN_EOF,
	TOKEN_TEXT,
	TOKEN_TEXT_BLANK,
	TOKEN_BREAK,
	TOKEN_OPEN_ANGLE,
	TOKEN_CLOSE_ANGLE,
	TOKEN_OPEN_ANGLE_SLASH,
	TOKEN_SLASH_CLOSE_ANGLE,
	TOKEN_OPEN_PARENTHESIS,
	TOKEN_CLOSE_PARENTHESIS,
	TOKEN_EQUALS,
	TOKEN_PLUS_EQUALS,
	TOKEN_DASH_EQUALS,
	TOKEN_STAR_EQUALS,
	TOKEN_SLASH_EQUALS,
	TOKEN_COLON_EQUALS,
	TOKEN_COMMA,
	TOKEN_BOOLEAN,
	TOKEN_INTEGER,
	TOKEN_FLOAT,
	TOKEN_PERCENTAGE,
	TOKEN_STRING,
	TOKEN_COLOR_LITERAL,
	TOKEN_URL_LITERAL,

	/* Keywords. */
	TOKEN_KEYWORD_FIRST,

	/* Tag names. */
	TOKEN_TAG_FIRST = TOKEN_KEYWORD_FIRST,
	TOKEN_DOCUMENT = TOKEN_TAG_FIRST,
	TOKEN_HBOX,
	TOKEN_VBOX,
	TOKEN_RULE,
	TOKEN_PARAGRAPH,
	TOKEN_CODE,
	TOKEN_H1,
	TOKEN_H2,
	TOKEN_H3,
	TOKEN_A,
	TOKEN_IMG,
	TOKEN_TAG_LAST,
	
	/* Attribute names. */
	TOKEN_ATTRIBUTE_FIRST = TOKEN_TAG_LAST,
	TOKEN_MATCH = TOKEN_ATTRIBUTE_FIRST,
	TOKEN_CLASS,
	TOKEN_GLOBAL,
	TOKEN_WIDTH,
	TOKEN_HEIGHT,
	TOKEN_MIN_WIDTH,
	TOKEN_MIN_HEIGHT,
	TOKEN_MAX_WIDTH,
	TOKEN_MAX_HEIGHT,
	TOKEN_GROW,
	TOKEN_SHRINK,
	TOKEN_PADDING,
	TOKEN_PADDING_LEFT,
	TOKEN_PADDING_RIGHT,
	TOKEN_PADDING_TOP,
	TOKEN_PADDING_BOTTOM,
	TOKEN_MARGIN,
	TOKEN_MARGIN_LEFT,
	TOKEN_MARGIN_RIGHT,
	TOKEN_MARGIN_TOP,
	TOKEN_MARGIN_BOTTOM,
	TOKEN_ARRANGE,
	TOKEN_ALIGN,
	TOKEN_JUSTIFY,
	TOKEN_LEADING,
	TOKEN_INDENT,
	TOKEN_COLOR,
	TOKEN_SELECTION_COLOR,
	TOKEN_SELECTION_FILL_COLOR,
	TOKEN_URL,
	TOKEN_LAYOUT,
	TOKEN_FONT,
	TOKEN_FONT_SIZE,
	TOKEN_BOLD,
	TOKEN_ITALIC,
	TOKEN_UNDERLINE,
	TOKEN_WHITE_SPACE,
	TOKEN_WRAP,
	TOKEN_BACKGROUND,
	TOKEN_BACKGROUND_COLOR,
	TOKEN_BACKGROUND_WIDTH,
	TOKEN_BACKGROUND_HEIGHT,
	TOKEN_BACKGROUND_SIZE,
	TOKEN_BACKGROUND_OFFSET_X,
	TOKEN_BACKGROUND_OFFSET_Y,
	TOKEN_BACKGROUND_HORIZONTAL_ALIGNMENT,
	TOKEN_BACKGROUND_VERTICAL_ALIGNMENT,
	TOKEN_BACKGROUND_BOX,
	TOKEN_BORDER_COLOR,
	TOKEN_BORDER_WIDTH,
	TOKEN_TINT,
	TOKEN_CLIP,
	TOKEN_CLIP_LEFT,
	TOKEN_CLIP_RIGHT,
	TOKEN_CLIP_TOP,
	TOKEN_CLIP_BOTTOM,
	TOKEN_CLIP_BOX,
	TOKEN_CURSOR,
	TOKEN_ENABLED,
	TOKEN_ATTRIBUTE_LAST,

	/* 
	 * Attribute value keywords.
	 */
	TOKEN_ATTRIBUTE_VALUE_FIRST = TOKEN_ATTRIBUTE_LAST,

	/* Shared attribute keywords. */
	TOKEN_UNDEFINED = TOKEN_ATTRIBUTE_VALUE_FIRST,
	TOKEN_NONE,
	TOKEN_ALL,
	TOKEN_AUTO,
	TOKEN_DEFAULT,
	TOKEN_FALSE,
	TOKEN_TRUE,
	TOKEN_LEFT,
	TOKEN_RIGHT,
	TOKEN_TOP,
	TOKEN_BOTTOM,
	TOKEN_HORIZONTAL,
	TOKEN_VERTICAL,
	TOKEN_RGB,
	TOKEN_RGBA,
	TOKEN_ALPHA,

	/* Layout modes. */
	TOKEN_BLOCK,
	TOKEN_INLINE,
	TOKEN_INLINE_CONTAINER,

	/* Alignment and arrangement. */
	TOKEN_START,
	TOKEN_MIDDLE,
	TOKEN_END,

	/* Justification. */
	TOKEN_CENTER,
	TOKEN_FLUSH,

	/* Special background sizes. */
	TOKEN_FIT,
	TOKEN_FILL,

	/* Bounding boxes. */
	TOKEN_CONTENT_BOX,
	TOKEN_PADDING_BOX,
	TOKEN_MARGIN_BOX,

	/* Pane types. */
	TOKEN_FLAT,
	TOKEN_SUNKEN,
	TOKEN_RAISED,

	/* White space. */
	TOKEN_NORMAL,
	TOKEN_PRESERVE,

	/* Wrap modes. */
	TOKEN_WORD_WRAP,
	TOKEN_CHARACTER_WRAP,

	/* Cursor types. */
	TOKEN_CURSOR_HAND,
	TOKEN_CURSOR_CARET,
	TOKEN_CURSOR_CROSSHAIR,
	TOKEN_CURSOR_MOVE,
	TOKEN_CURSOR_SIZE_NS,
	TOKEN_CURSOR_SIZE_EW,
	TOKEN_CURSOR_WAIT,

	TOKEN_ATTRIBUTE_VALUE_LAST,
	TOKEN_KEYWORD_LAST = TOKEN_ATTRIBUTE_VALUE_LAST,

	/* Counts for each sort of token. */
	NUM_KEYWORDS               = TOKEN_KEYWORD_LAST - TOKEN_KEYWORD_FIRST,
	NUM_TAG_TOKENS             = TOKEN_TAG_LAST - TOKEN_TAG_FIRST,
	NUM_ATTRIBUTE_TOKENS       = TOKEN_ATTRIBUTE_LAST - TOKEN_ATTRIBUTE_FIRST,
	NUM_ATTRIBUTE_VALUE_TOKENS = TOKEN_ATTRIBUTE_VALUE_LAST - TOKEN_ATTRIBUTE_VALUE_FIRST,
	NUM_TOKENS                 = TOKEN_KEYWORD_LAST
};

extern const char * const TOKEN_STRINGS[NUM_TOKENS];

bool is_keyword(int token);
int find_keyword(const char *s, unsigned length);

bool is_enum_token(int token);
bool is_background_attribute(int token);
bool is_cascaded_style_attribute(int token);
bool is_layout_attribute(int token);
bool is_assignment_operator(int token);

} // namespace stkr
