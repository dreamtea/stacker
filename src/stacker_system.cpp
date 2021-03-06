#include "stacker_system.h"

#include "stacker_shared.h"
#include "stacker_attribute_buffer.h"
#include "stacker_util.h"
#include "stacker_platform.h"
#include "stacker_document.h"
#include "stacker_layer.h"

namespace stkr {

static void make_font_descriptor(LogicalFont *descriptor, const char *face,
	unsigned size, unsigned flags)
{
	if (face != NULL) {
		strncpy(descriptor->face, face, sizeof(descriptor->face));
		descriptor->face[sizeof(descriptor->face) - 1] = '\0';
	} else {
		descriptor->face[0] = '\0';
	}
	descriptor->font_size = size;
	descriptor->flags = (uint16_t)flags;
}
  
static void initialize_font_cache(System *system)
{
	system->default_font_id = INVALID_FONT_ID;
	system->font_cache_entries = 0;
	make_font_descriptor(&system->default_font_descriptor, 
		DEFAULT_FONT_FACE, DEFAULT_FONT_SIZE, DEFAULT_FONT_FLAGS);
	system->default_font_id = get_font_id(system, 
		&system->default_font_descriptor);
	ensure(system->default_font_id != INVALID_FONT_ID);
	system->debug_label_font_id = INVALID_FONT_ID;
}

/* Returns a key uniquely identifying a font specification. */
static uint32_t make_font_key(const LogicalFont *logfont)
{
	uint32_t seed = logfont->font_size | (logfont->flags << 16);
	return murmur3_32(logfont->face, strlen(logfont->face), seed);
}

/* Precalculates numbers needed for typesetting from the system font metrics. */
static void calculate_derived_font_metrics(FontMetrics *metrics)
{
	/* w = (1/3)em, y = (1/6)em, z = (1/9)em */
	static const int ONE_THIRD = (1 << TEXT_METRIC_PRECISION) / 3;
	static const int ONE_SIXTH = (1 << TEXT_METRIC_PRECISION) / 6;
	static const int ONE_NINTH = (1 << TEXT_METRIC_PRECISION) / 9;
	metrics->space_width   = fixed_multiply(metrics->em_width, ONE_THIRD, TEXT_METRIC_PRECISION);
	metrics->space_stretch = fixed_multiply(metrics->em_width, ONE_SIXTH, TEXT_METRIC_PRECISION);
	metrics->space_shrink  = fixed_multiply(metrics->em_width, ONE_NINTH, TEXT_METRIC_PRECISION);
	metrics->paragraph_indent_width = metrics->em_width;
}

/* Returns the ID of a font from the font cache, creating it if necessary. */
int16_t get_font_id(System *system, const LogicalFont *logfont)
{
	uint32_t key = make_font_key(logfont);
	for (unsigned i = 0; i < system->font_cache_entries; ++i)
		if (system->font_cache[i].key == key)
			return (int16_t)i;
 	if (system->font_cache_entries == MAX_CACHED_FONTS)
		return 0;
	void *handle = platform_match_font(system->back_end, logfont);
	if (handle == NULL)
		return system->default_font_id;
	CachedFont *cf = system->font_cache + system->font_cache_entries;
	cf->key = key;
	cf->handle = handle;
	cf->descriptor = *logfont;
	platform_font_metrics(system->back_end, handle, &cf->metrics);
	calculate_derived_font_metrics(&cf->metrics);
	return int16_t(system->font_cache_entries++);
}

/* Returns the system handle for a cached font. */
void *get_font_handle(System *system, int16_t font_id)
{
	assertb(unsigned(font_id) < system->font_cache_entries);
	return system->font_cache[font_id].handle;
}

/* Returns the logical font used to create a font ID. */
const LogicalFont *get_font_descriptor(System *system, int16_t font_id)
{
	if (font_id != INVALID_FONT_ID) {
		assertb(unsigned(font_id) < system->font_cache_entries);
		return &system->font_cache[font_id].descriptor;
	}
	return &system->default_font_descriptor;
}

const FontMetrics *get_font_metrics(System *system, int16_t font_id)
{
	assertb(unsigned(font_id) < system->font_cache_entries);
	return &system->font_cache[font_id].metrics;
}

unsigned measure_text(System *system, int16_t font_id, const void *text, 
	unsigned length, unsigned *advances)
{
	void *font_handle = get_font_handle(system, font_id);
	return platform_measure_text(system->back_end, font_handle, 
		text, length, advances);
}

/* A convenience function to determine the size of a string's bounding 
 * rectangle. Optionally returns the temporary heap-allocated advances array
 * used, for which the caller takes responsibility. */
unsigned measure_text_rectangle(System *system, int16_t font_id, 
	const void *text, unsigned length, unsigned *out_width, 
	unsigned *out_height, unsigned **out_advances)
{
	unsigned *advances = new unsigned[length];
	unsigned num_characters = measure_text(system, font_id, text, 
		length, advances);
	if (out_width != NULL) {
		*out_width = 0;
		for (unsigned i = 0; i < num_characters; ++i)
			*out_width += advances[i];
		*out_width = round_fixed_to_int(*out_width, TEXT_METRIC_PRECISION);
	}
	if (out_height != NULL) {
		const FontMetrics *metrics = get_font_metrics(system, font_id);
		*out_height = round_fixed_to_int(metrics->height, TEXT_METRIC_PRECISION);
	}
	if (out_advances != NULL) {
		*out_advances = advances;	
	} else {
		delete [] out_advances;
	}
	return num_characters;
}

/* Precomputes hashed rule names for tag tokens and pseudo classes. */
static void make_built_in_rule_names(System *system)
{
	system->rule_name_all         = murmur3_64_cstr("*");
	system->rule_name_active      = murmur3_64_cstr(":active");
	system->rule_name_highlighted = murmur3_64_cstr(":highlighted");
	for (unsigned i = 0; i < NUM_KEYWORDS; ++i) {
		system->token_rule_names[i] = murmur3_64_cstr(
			TOKEN_STRINGS[TOKEN_KEYWORD_FIRST + i]);
	}
}

static unsigned add_font_assignments(AttributeAssignment *attributes, 
	unsigned count, const char *face, unsigned size, unsigned flags)
{
	attributes[count++] = make_assignment(TOKEN_FONT, face);
	attributes[count++] = make_assignment(TOKEN_FONT_SIZE, size);
	attributes[count++] = make_assignment(TOKEN_BOLD, (flags & STYLE_BOLD) != 0, VSEM_BOOLEAN);
	attributes[count++] = make_assignment(TOKEN_ITALIC, (flags & TOKEN_ITALIC) != 0, VSEM_BOOLEAN);
	attributes[count++] = make_assignment(TOKEN_UNDERLINE, (flags & STYLE_UNDERLINE) != 0, VSEM_BOOLEAN);
	return count;
}

static void add_default_rules(System *system)
{
	static const unsigned MAX_ROOT_ATTRIBUTES = 32;

	AttributeAssignment attributes[MAX_ROOT_ATTRIBUTES];
	unsigned count = 0;
	
	attributes[count++] = make_assignment(TOKEN_COLOR, DEFAULT_TEXT_COLOR, VSEM_COLOR);
	attributes[count++] = make_assignment(TOKEN_JUSTIFY, TOKEN_LEFT, VSEM_TOKEN);
	attributes[count++] = make_assignment(TOKEN_WRAP, TOKEN_WORD_WRAP, VSEM_TOKEN);
	attributes[count++] = make_assignment(TOKEN_LEADING, TOKEN_AUTO, VSEM_TOKEN);
	attributes[count++] = make_assignment(TOKEN_WHITE_SPACE, TOKEN_NORMAL, VSEM_TOKEN);
	count = add_font_assignments(attributes, count, DEFAULT_FONT_FACE, DEFAULT_FONT_SIZE, DEFAULT_FONT_FLAGS);
	add_rule(NULL, system, NULL, "document", -1, attributes, count, RFLAG_ENABLED | RFLAG_GLOBAL, RULE_PRIORITY_LOWEST);

	count = 0;
	attributes[count++] = make_assignment(TOKEN_WIDTH, TOKEN_GROW, VSEM_TOKEN);
	attributes[count++] = make_assignment(TOKEN_JUSTIFY, TOKEN_FLUSH, VSEM_TOKEN);
	attributes[count++] = make_assignment(TOKEN_INDENT, TOKEN_AUTO, VSEM_TOKEN);
	add_rule(NULL, system, NULL, "p", -1, attributes, count, RFLAG_ENABLED | RFLAG_GLOBAL, RULE_PRIORITY_LOWEST);

	count = 0;
	attributes[count++] = make_assignment(TOKEN_CURSOR, TOKEN_CURSOR_HAND, VSEM_TOKEN);
	attributes[count++] = make_assignment(TOKEN_UNDERLINE, true, VSEM_BOOLEAN);
	attributes[count++] = make_assignment(TOKEN_COLOR, DEFAULT_LINK_COLOR, VSEM_COLOR);
	add_rule(NULL, system, NULL, "a", -1, attributes, count, RFLAG_ENABLED | RFLAG_GLOBAL, RULE_PRIORITY_LOWEST);

	count = 0;
	attributes[count++] = make_assignment(TOKEN_COLOR, DEFAULT_HIGHLIGHTED_LINK_COLOR, VSEM_COLOR);
	add_rule(NULL, system, NULL, "a:highlighted", -1, attributes, count, RFLAG_ENABLED | RFLAG_GLOBAL, RULE_PRIORITY_LOWEST);

	count = 0;
	attributes[count++] = make_assignment(TOKEN_COLOR, DEFAULT_ACTIVE_LINK_COLOR, VSEM_COLOR);
	add_rule(NULL, system, NULL, "a:active", -1, attributes, count, RFLAG_ENABLED | RFLAG_GLOBAL, RULE_PRIORITY_LOWEST);

	count = 0;
	attributes[count++] = make_assignment(TOKEN_WIDTH, TOKEN_GROW, VSEM_TOKEN);
	attributes[count++] = make_assignment(TOKEN_FONT_SIZE, 2.5f, VSEM_NONE, AOP_MULTIPLY);
	attributes[count++] = make_assignment(TOKEN_BOLD, true, VSEM_BOOLEAN);
	add_rule(NULL, system, NULL, "h1", -1, attributes, count, RFLAG_ENABLED | RFLAG_GLOBAL, RULE_PRIORITY_LOWEST);

	count = 0;
	attributes[count++] = make_assignment(TOKEN_WIDTH, TOKEN_GROW, VSEM_TOKEN);
	attributes[count++] = make_assignment(TOKEN_FONT_SIZE, 2.0f, VSEM_NONE, AOP_MULTIPLY);
	attributes[count++] = make_assignment(TOKEN_BOLD, true, VSEM_BOOLEAN);
	add_rule(NULL, system, NULL, "h2", -1, attributes, count, RFLAG_ENABLED | RFLAG_GLOBAL, RULE_PRIORITY_LOWEST);

	count = 0;
	attributes[count++] = make_assignment(TOKEN_WIDTH, TOKEN_GROW, VSEM_TOKEN);
	attributes[count++] = make_assignment(TOKEN_FONT_SIZE, 1.5f, VSEM_NONE, AOP_MULTIPLY);
	attributes[count++] = make_assignment(TOKEN_BOLD, true, VSEM_BOOLEAN);
	add_rule(NULL, system, NULL, "h3", -1, attributes, count, RFLAG_ENABLED | RFLAG_GLOBAL, RULE_PRIORITY_LOWEST);

	count = 0;
	attributes[count++] = make_assignment(TOKEN_WHITE_SPACE, TOKEN_PRESERVE, VSEM_TOKEN);
	count = add_font_assignments(attributes, count, DEFAULT_FIXED_FONT_FACE, DEFAULT_FIXED_FONT_SIZE, DEFAULT_FIXED_FONT_FLAGS);
	add_rule(NULL, system, NULL, "code", -1, attributes, count, RFLAG_ENABLED | RFLAG_GLOBAL, RULE_PRIORITY_LOWEST);
}

static void initialize_url_notifications(System *system, UrlCache *url_cache)
{
	if (url_cache != NULL) {
		system->image_layer_notify_id = url_cache->add_notify_sink(
			(urlcache::NotifyCallback)&image_layer_notify_callback, system);
		system->document_notify_id = url_cache->add_notify_sink(
			(urlcache::NotifyCallback)&document_fetch_notify_callback, system);
	} else {
		system->image_layer_notify_id = urlcache::INVALID_NOTIFY_SINK_ID;
	}
}

static void deinitialize_url_notifications(System *system, UrlCache *url_cache)
{
	if (url_cache != NULL) {
		url_cache->remove_notify_sink(system->image_layer_notify_id);
		url_cache->remove_notify_sink(system->document_notify_id);
	}
}

int16_t get_debug_label_font_id(System *system)
{
	if (system->debug_label_font_id == INVALID_FONT_ID) {
		LogicalFont descriptor;
		make_font_descriptor(&descriptor, DEBUG_LABEL_FONT_FACE, 
			DEBUG_LABEL_FONT_SIZE, DEBUG_LABEL_FONT_FLAGS);
		system->debug_label_font_id = get_font_id(system, &descriptor);
	}
	return system->debug_label_font_id;
}

System *create_system(unsigned flags, BackEnd *back_end, UrlCache *url_cache, 
	TextEncoding encoding, TextEncoding  message_encoding)
{
	System *system = new System();
	system->flags = flags;
	system->encoding = encoding;
	system->message_encoding = message_encoding;
	system->back_end = back_end;
	system->url_cache = url_cache;
	system->rule_table_revision = 0;
	system->rule_revision_counter = 0;
	system->total_boxes = 0;
	system->total_nodes = 0;
	initialize_font_cache(system);
	make_built_in_rule_names(system);
	initialize_url_notifications(system, url_cache);
	add_default_rules(system);
	return system;
}

void destroy_system(System *system)
{
	assertb(system->total_nodes == 0);
	assertb(system->total_boxes == 0);
	clear_rule_table(&system->global_rules);
	for (unsigned i = 0; i < system->font_cache_entries; ++i)
		platform_release_font(system->back_end, system->font_cache[i].handle);
	deinitialize_url_notifications(system, system->url_cache);
	delete system;
}

BackEnd *get_back_end(System *system)
{
	return system->back_end;
}

unsigned get_total_nodes(const System *system)
{
	return system->total_nodes;
}

unsigned get_total_boxes(const System *system)
{
	return system->total_boxes;
}

} // namespace stkr
