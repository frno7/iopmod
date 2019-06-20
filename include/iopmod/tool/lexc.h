// SPDX-License-Identifier: GPL-2.0
/*
 * Lexer for the C programming language
 *
 * Copyright (C) 2019 Fredrik Noring
 */

#ifndef IOPMOD_TOOL_LEXC_H
#define IOPMOD_TOOL_LEXC_H

#include <stddef.h>

#define LEXC_TOKENS(T)							\
	T(EMPTY, "<empty>")						\
									\
	T(INTEGER, "<integer>")						\
	T(FLOAT, "<float>")						\
	T(STRING, "<string>")						\
	T(CHARACTER, "<character>")					\
	T(IDENTIFIER, "<identifier>")					\
	T(COMMENT, "<comment>")						\
	T(WHITESPACE, "<whitespace>")					\
									\
	T(LPAREN, "(")							\
	T(RPAREN, ")")							\
	T(LBRACK, "[")							\
	T(RBRACK, "]")							\
	T(LBRACE, "{")							\
	T(RBRACE, "}")							\
									\
	T(INC, "++")							\
	T(DEC, "--")							\
	T(DOT, ".")							\
	T(DOT_DOT, "..")						\
	T(ELLIPSIS, "...")						\
	T(DEREFERENCE, "->")						\
	T(CONDITIONAL, "?")						\
	T(COLON, ":")							\
	T(COLON_COLON, "::")						\
	T(SEMICOLON, ";")						\
	T(BACKSLASH, "\\")						\
									\
	T(ASSIGN, "=")							\
	T(ASSIGN_BIT_OR, "|=")						\
	T(ASSIGN_BIT_XOR, "^=")						\
	T(ASSIGN_BIT_AND, "&=")						\
	T(ASSIGN_SHL, "<<=")						\
	T(ASSIGN_SAR, ">>=")						\
	T(ASSIGN_ADD, "+=")						\
	T(ASSIGN_SUB, "-=")						\
	T(ASSIGN_MUL, "*=")						\
	T(ASSIGN_DIV, "/=")						\
	T(ASSIGN_MOD, "%=")						\
									\
	T(COMMA, ",")							\
	T(OR, "||")							\
	T(AND, "&&")							\
	T(NOT, "!")							\
	T(BIT_OR, "|")							\
	T(BIT_XOR, "^")							\
	T(BIT_NOT, "~")							\
	T(AMPERSAND, "&")						\
	T(SHL, "<<")							\
	T(SAR, ">>")							\
	T(PLUS, "+")							\
	T(MINUS, "-")							\
	T(ASTERISK, "*")						\
	T(DIV, "/")							\
	T(MOD, "%")							\
									\
	T(EQ, "==")							\
	T(NE, "!=")							\
	T(GT, ">")							\
	T(GE, ">=")							\
	T(LT, "<")							\
	T(LE, "<=")							\
									\
	T(NUMSIGN, "#")							\
	T(NUMSIGN_NUMSIGN, "##")					\
									\
	/* Errors. */							\
	T(ERROR_END_OF_COMMENT, "unterminated comment")			\
	T(ERROR_END_OF_STRING, "missing terminating \" character")	\
	T(ERROR_END_OF_CHARACTER, "missing terminating \' character")	\
	T(ERROR_END_OF_ESCAPE, "missing escaped character")		\
	T(ERROR_INVALID_CHARACTER, "invalid character")

/**
 * enum lexc_token_type - token types for the C lexer
 * @TOKEN_EMPTY: the end of the input string has been reached
 * @TOKEN_INTEGER: integer token
 * @TOKEN_FLOAT: floating point token
 * @TOKEN_STRING: string token
 * @TOKEN_IDENTIFIER: identifier token
 * @TOKEN_COMMENT: comment token
 * @TOKEN_WHITESPACE: whitespace token
 * ...
 */
enum lexc_token_type {
#define LEXC_ENUM_TOKEN(identifier, description)			\
	TOKEN_##identifier,
	LEXC_TOKENS(LEXC_ENUM_TOKEN)
};

/**
 * struct lexc_token - token for the C lexer
 * @type: type of token
 * @string: pointer to first character representing the token, not NUL
 * 	terminated
 * @length: length in characters of the token
 * @line: vertical position of the token, starting from line 1
 * @column: horizontal position of the token, staring from column 1
 * @eos: pointer to character after end of input string
 */
struct lexc_token {
	enum lexc_token_type type;
	const char *string;
	size_t length;
	int line;
	int column;
	const char *eos;
};

/**
 * lexc_for_every_token - iterate over tokens including comments and whitespace
 * @token: &struct lexc_token loop cursor
 * @s: NUL-terminated string to turn into tokens
 */
#define lexc_for_every_token(token, s)					\
	for ((token) = lexc_first_any_token((s), NULL);			\
	     (token).type != TOKEN_EMPTY;				\
	     (token) = lexc_next_any_token(token))

/**
 * lexc_for_each_token - iterate over tokens except comments and whitespace
 * @token: &struct lexc_token loop cursor
 * @s: NUL-terminated string to turn into tokens
 */
#define lexc_for_each_token(token, s)					\
	for ((token) = lexc_first_token((s), NULL);			\
	     (token).type != TOKEN_EMPTY;				\
	     (token) = lexc_next_token(token))

/**
 * lexc_for_every_token_after - iterate over remaining tokens including
 * 	comments and whitespace
 * @token: &struct lexc_token to start after and loop cursor
 */
#define lexc_for_every_token_after(token)				\
	for ((token) = lexc_next_any_token((token));			\
	     (token).type != TOKEN_EMPTY;				\
	     (token) = lexc_next_any_token(token))

/**
 * lexc_for_each_token_after - iterate over remaining tokens except
 * 	comments and whitespace
 * @token: &struct lexc_token to start after and loop cursor
 */
#define lexc_for_each_token_after(token)				\
	for ((token) = lexc_next_token((token));			\
	     (token).type != TOKEN_EMPTY;				\
	     (token) = lexc_next_token(token))

/**
 * lexc_first_token - first token for string except comments and whitespace
 * @s: string to turn into tokens
 * @eos: pointer to character after end of input string, or %NULL to assume
 * 	@s is NUL-terminated
 *
 * Return: first token that is not a comment or whitespace
 */
struct lexc_token lexc_first_token(const char *s, const char *eos);

/**
 * lexc_next_any_token - first token for string including comments
 * 	and whitespace
 * @s: string to turn into tokens
 * @eos: pointer to character after end of string, or %NULL to assume @s is
 * 	NUL-terminated
 *
 * Return: first token including comments or whitespace
 */
struct lexc_token lexc_first_any_token(const char *s, const char *eos);

/**
 * lexc_next_token - token following given token except comments and whitespace
 * @token: token to advance from
 *
 * Return: next token that is not a comment or whitespace
 */
struct lexc_token lexc_next_token(struct lexc_token token);

/**
 * lexc_next_any_token - token following given token including comments
 * 	and whitespace
 * @token: token to advance from
 *
 * Return: next token including comments or whitespace
 */
struct lexc_token lexc_next_any_token(struct lexc_token token);

/**
 * lexc_equal_token - are the tokens equivalent?
 * @a: first token to compare
 * @b: second token to compare
 *
 * Return: %true if @a and @b are equivalent, otherwise %false
 */
bool lexc_equal_token(struct lexc_token a, struct lexc_token b);

/**
 * lexc_token_type_str - description of given type of token
 * @type: type of token to describe
 *
 * Return: string describing the type of token
 */
const char *lexc_token_type_str(enum lexc_token_type type);

#endif /* IOPMOD_TOOL_LEXC_H */
