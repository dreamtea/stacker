#pragma once

#include <cstdint>

#include "stacker_token.h"
#include "stacker_attribute.h"

namespace stkr {

struct System;
struct Document;
struct Node;

const unsigned MAX_ATTRIBUTES       = 32;
const unsigned MAX_MESSAGE_SIZE     = 511;
const unsigned ERROR_CONTEXT_CHARS  = 16;

enum ParserFlag {
	PARSEFLAG_SINGLE_NODE = 1 << 0 /* Stop after parsing the first node in the input. */
};

struct Parser {
	System *system;
	Document *document;
	Node *root;
	Node *first_parsed;
	Node *last_parsed;
	int token;
	Variant token_value;
	const char *input;
	unsigned pos;
	unsigned token_start;
	unsigned token_escape_count;
	unsigned input_size;
	bool in_tag;
	bool emit_break;
	int line;
	Node *scope;
	unsigned flags;
	int code;
	char message[MAX_MESSAGE_SIZE + 1];
};

void init_parser(Parser *parser);
int parse(Parser *parser, Document *document, Node *root, 
	const char *input, unsigned length);

} // namespace stkr

