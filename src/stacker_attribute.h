#pragma once

#include <cstdint>
#include <cstring>

#include "stacker_token.h"

namespace stkr {

/* Determines the set of mode values an attribute can take on along with 
 * constraints on the attribute value in each mode. */
enum AttributeSemantic {
	ASEM_INVALID = -1,

	ASEM_FLAG,
	ASEM_REAL,
	ASEM_STRING,
	ASEM_STRING_SET,
	ASEM_DIMENSON,
	ASEM_ABSOLUTE_DIMENSION,
	ASEM_ALIGNMENT,
	ASEM_JUSTIFICATION,
	ASEM_COLOR,
	ASEM_URL,
	ASEM_BACKGROUND,
	ASEM_BACKGROUND_SIZE,
	ASEM_BOUNDING_BOX,
	ASEM_LAYOUT,
	ASEM_EDGES,
	ASEM_WHITE_SPACE,
	ASEM_WRAP_MODE,
	ASEM_CURSOR
};

/* An optional semantic tag attached to a value generated by the parser
 * or passed in as a attribute value. For example, parsing a "url(...)" 
 * construct might generate a string with VSEM_URL attached, meaning, "this 
 * string is supposed to be a URL". We might then prevent that string from
 * being assigned to the "font" attribute because it wouldn't make sense. */
enum ValueSemantic {
	VSEM_INVALID = -1,

	VSEM_NONE,
	VSEM_BOOLEAN,
	VSEM_LIST,
	VSEM_TOKEN,
	VSEM_PERCENTAGE,
	VSEM_URL,
	VSEM_COLOR,
	VSEM_PANE,
	VSEM_EDGES
};

/* How an attribute is combined with other attributes of the same name in 
 * related buffers to form a final value. */
enum AttributeOperator {
	AOP_SET,
	AOP_OVERRIDE,
	AOP_COMPUTED,
	AOP_ADD,
	AOP_SUBTRACT,
	AOP_MULTIPLY,
	AOP_DIVIDE
};

/* How an attribute is represented inside an attribute heap. */
enum AttributeStorage {
	STORAGE_INVALID = -1,

	STORAGE_NONE,
	STORAGE_INT16,
	STORAGE_INT32,
	STORAGE_FLOAT32,
	STORAGE_STRING,

	NUM_ATTRIBUTE_TYPES
};

/* How to represent a string set when it is read as a string. */
enum StringSetRepresentation {
	SSR_INTERNAL,        /* Each entry is a null terminated string. The buffer 
	                        is terminated with an extra null. The reported 
	                        length excludes this final null, but includes the 
	                        terminators of the entries. */
	SSR_COMMA_DELIMITED, /* Entries are separated by exactly one comma. */
	SSR_SPACE_DELIMITED  /* Entries are separated by exactly one space. There is 
	                        no white space and the beginning or end of the 
	                        string. */
};

/* Every attribute can be undefined. Undefined means "inherit" and is distinct 
 * from values like "auto". Attribute-specific modes start at ADEF_DEFINED. */
const int ADEF_UNDEFINED = 0;
const int ADEF_DEFINED   = 1;

/* Modes for boolean attributes, which can be undefined, false or true. */
enum FlagMode {
	FLAGMODE_FALSE = ADEF_DEFINED,
	FLAGMODE_TRUE,
	FLAGMODE_LAST
};

/* How a dimension attribute's value should be interpreted. */
enum DimensionMode {
	DMODE_AUTO = ADEF_DEFINED,
	DMODE_ABSOLUTE,
	DMODE_FRACTIONAL,
	DMODE_LAST
};

/* An alignment says how a child is positioned on the minor axis of its parent. */
enum Alignment {
	ALIGN_START = ADEF_DEFINED,
	ALIGN_MIDDLE,
	ALIGN_END,
	ALIGN_SENTINEL
};

/* How to align text horizontally. */
enum Justification {
	JUSTIFY_LEFT = ADEF_DEFINED,
	JUSTIFY_CENTER,
	JUSTIFY_RIGHT,
	JUSTIFY_FLUSH,
	JUSTIFY_SENTINEL
};

/* Built-in background types. */
enum PaneType {
	PANE_FLAT = ADEF_DEFINED,
	PANE_SUNKEN,
	PANE_RAISED,
	PANE_LAST,
	NUM_PANE_TYPES = PANE_LAST - PANE_FLAT
};

/* Modes for background attributes: none, a colour, a URL, or one of the 
 * built-in pane types, which we encode in the mode. */
enum BackgroundMode {
	BGMODE_URL = ADEF_DEFINED,
	BGMODE_COLOR,
	BGMODE_PANE_FIRST,
	BGMODE_PANE_LAST = BGMODE_PANE_FIRST + NUM_PANE_TYPES - 1,
	BGMODE_LAST
};

/* How to collapse spaces and new lines in an inline context.  */
enum WhiteSpaceMode {
	WSM_NORMAL = ADEF_DEFINED,
	WSM_PRESERVE
};

/* Word wrapping modes. */
enum WrapMode {
	WRAPMODE_WORD = ADEF_DEFINED,
	WRAPMODE_CHARACTER,
	WRAP_SENTINEL
};

/* How to position and scale an image with respect to its container. */
enum LayerPositioningMode {
	VLPM_STANDARD = ADEF_DEFINED,
	VLPM_FIT,
	VLPM_FILL,
	VLPM_SENTINEL
};

/* Designates the content, padding or margin box for positioning or clipping. */
enum BoundingBox {
	BBOX_CONTENT = ADEF_DEFINED,
	BBOX_PADDING,
	BBOX_OUTER,
	BBOX_SENTINEL
};

/* The algorithm used to position a node's children. */
enum LayoutContext {
	LCTX_NO_LAYOUT = ADEF_DEFINED,
	LCTX_BLOCK,
	LCTX_INLINE,
	LCTX_INLINE_CONTAINER,
	LCTX_SENTINEL
};

/* Bits used to define sets of edges. */
enum EdgeFlag {
	EDGE_FLAG_NONE       = ADEF_DEFINED,
	EDGE_FLAG_LEFT       = 1 << 1,
	EDGE_FLAG_RIGHT      = 1 << 2,
	EDGE_FLAG_TOP        = 1 << 3,
	EDGE_FLAG_BOTTOM     = 1 << 4,
	EDGE_FLAG_HORIZONTAL = EDGE_FLAG_LEFT | EDGE_FLAG_RIGHT,
	EDGE_FLAG_VERTICAL   = EDGE_FLAG_TOP | EDGE_FLAG_BOTTOM,
	EDGE_FLAG_ALL        = EDGE_FLAG_HORIZONTAL | EDGE_FLAG_VERTICAL
};

/* Mouse cursor types. */
enum CursorType {
	CT_DEFAULT = ADEF_DEFINED,
	CT_HAND,
	CT_CARET,
	CT_CROSSHAIR,
	CT_MOVE,
	CT_SIZE_NS,
	CT_SIZE_EW,
	CT_WAIT,
	CT_SENTINEL
};

enum VariantType { VTYPE_INTEGER, VTYPE_FLOAT, VTYPE_STRING };

/* Tagged union representing a number or string, and also specifying a value
 * semantic which says something like, "this integer is a word wrap mode" or
 * "this string is a URL". */
struct Variant {
	VariantType type;
	ValueSemantic semantic;
	union {
		struct {
			const char *data;
			unsigned length;
		} string;
		int integer;
		float real;
	};
};

/* An intemediate structure storing a value that will become an attribute. */
struct AttributeAssignment { 
	Token name; 
	AttributeOperator op; 
	Variant value; 
};

inline void variant_set_integer(Variant *v, int value, 
	ValueSemantic vs = VSEM_NONE) 
{
	v->type = VTYPE_INTEGER;
	v->semantic = vs;
	v->integer = value;
}

inline void variant_set_float(Variant *v, float value, 
	ValueSemantic vs = VSEM_NONE)
{
	v->type = VTYPE_FLOAT;
	v->semantic = vs;
	v->real = value;
}

inline void variant_set_string(Variant *v, const char *s, unsigned length, 
	ValueSemantic vs = VSEM_NONE)
{
	v->type = VTYPE_STRING;
	v->semantic = vs;
	v->string.data = s;
	v->string.length = length;
}

inline void variant_set_string(Variant *v, const char *s, 
	ValueSemantic vs = VSEM_NONE)
{
	variant_set_string(v, s, strlen(s), vs);
}

} // namespace stkr
