// SPDX-License-Identifier: GPL-2.0
/*
 * Lexer for the C programming language
 *
 * Copyright (C) 2019 Fredrik Noring
 */

#include <string.h>

#include "iopmod/types.h"
#include "iopmod/tool/lexc.h"

enum character_class {
	ccIdent  = 0x01,
	ccDigit  = 0x02,
	ccHex    = 0x04,
	ccExp    = 0x08,
	ccDot    = 0x10,
	ccHorzWs = 0x20,
	ccVertWs = 0x40,
};

static const unsigned char char_class[256] =
{
/*  0 NUL            1 SOH                  2 STX            3 ETX */
/*  4 EOT            5 ENQ                  6 ACK            7 BEL */
    0,               0,                     0,               0,
    0,               0,                     0,               0,
/*  8 BS             9 HT                  10 NL            11 VT */
/* 12 NP            13 CR                  14 SO            15 SI */
    0,               ccHorzWs,              ccVertWs,        ccHorzWs,
    ccHorzWs,        ccHorzWs,              0,               0,
/* 16 DLE           17 DC1                 18 DC2           19 DC3 */
/* 20 DC4           21 NAK                 22 SYN           23 ETB */
    0,               0,                     0,               0,
    0,               0,                     0,               0,
/* 24 CAN           25 EM                  26 SUB           27 ESC */
/* 28 FS            29 GS                  30 RS            31 US */
    0,               0,                     0,               0,
    0,               0,                     0,               0,
/* 32 SP            33  !                  34  "            35  # */
/* 36  $            37  %                  38  &            39  ' */
    ccHorzWs,        0,                     0,               0,
    0,               0,                     0,               0,
/* 40  (            41  )                  42  *            43  + */
/* 44,              45  -                  46  .            47  / */
    0,               0,                     0,               0,
    0,               0,                     ccDot,           0,
/* 48  0            49  1                  50  2            51  3 */
/* 52  4            53  5                  54  6            55  7 */
    ccDigit|ccHex,   ccDigit|ccHex,         ccDigit|ccHex,   ccDigit|ccHex,
    ccDigit|ccHex,   ccDigit|ccHex,         ccDigit|ccHex,   ccDigit|ccHex,
/* 56  8            57  9                  58  :            59  ; */
/* 60  <            61  =                  62  >            63  ? */
    ccDigit|ccHex,   ccDigit|ccHex,         0,               0,
    0,               0,                     0,               0,
/* 64  @            65  A                  66  B            67  C */
/* 68  D            69  E                  70  F            71  G */
    0,               ccIdent|ccHex,         ccIdent|ccHex,   ccIdent|ccHex,
    ccIdent|ccHex,   ccIdent|ccHex|ccExp,   ccIdent|ccHex,   ccIdent,
/* 72  H            73  I                  74  J            75  K */
/* 76  L            77  M                  78  N            79  O */
    ccIdent,         ccIdent,               ccIdent,         ccIdent,
    ccIdent,         ccIdent,               ccIdent,         ccIdent,
/* 80  P            81  Q                  82  R            83  S */
/* 84  T            85  U                  86  V            87  W */
    ccIdent|ccExp,   ccIdent,               ccIdent,         ccIdent,
    ccIdent,         ccIdent,               ccIdent,         ccIdent,
/* 88  X            89  Y                  90  Z            91  [ */
/* 92  \            93  ]                  94  ^            95  _ */
    ccIdent|ccHex,   ccIdent,               ccIdent,         0,
    0,               0,                     0,               ccIdent,
/* 96  `            97  a                  98  b            99  c */
/* 100  d          101  e                 102  f           103  g */
    0,               ccIdent|ccHex,         ccIdent|ccHex,   ccIdent|ccHex,
    ccIdent|ccHex,   ccIdent|ccHex|ccExp,   ccIdent|ccHex,   ccIdent,
/* 104  h          105  i                 106  j           107  k */
/* 108  l          109  m                 110  n           111  o */
    ccIdent,         ccIdent,               ccIdent,         ccIdent,
    ccIdent,         ccIdent,               ccIdent,         ccIdent,
/* 112  p          113  q                 114  r           115  s */
/* 116  t          117  u                 118  v           119  w */
    ccIdent|ccExp,   ccIdent,               ccIdent,         ccIdent,
    ccIdent,         ccIdent,               ccIdent,         ccIdent,
/* 120  x          121  y                 122  z           123  { */
/* 124  |          125  }                 126  ~           127 DEL */
    ccIdent|ccHex,   ccIdent,               ccIdent,         0,
    0,               0,                     0,               0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

#define CC(class) (char_class[(unsigned char)(c)] & (class))

#define TT(t) ({ token.type = (t); })

#define ADVANCE_CHAR ({ ++pos; ++token.length; })

#define S1(s, t)							\
	if (c != s) return token;					\
	TT(t);								\
	continue

#define S2(s1, t1, s2, t2)						\
	     if (c == s1) TT(t1);					\
	else if (c == s2) TT(t2);					\
	else return token;						\
	continue

#define S3(s1, t1, s2, t2, s3, t3)					\
	     if (c == s1) TT(t1);					\
	else if (c == s2) TT(t2);					\
	else if (c == s3) TT(t3);					\
	else return token;						\
	continue

static int column_for_char(char c, int column)
{
	if (c == '\t')
		return 1 + (((column - 1) >> 3) << 3) + 8;

	return 1 + column;
}

static const char *advance_token(struct lexc_token *token)
{
	while (token->length) {
		char c = *token->string++;

		token->length--;

		if (CC(ccVertWs)) {
			token->line++;
			token->column = 1;
		} else
			token->column = column_for_char(c, token->column);
	}

	token->type = TOKEN_EMPTY;

	return token->string;
}

struct lexc_token lexc_next_any_token(struct lexc_token token)
{
	const char *pos = advance_token(&token);

	for (int escape = 0; pos != token.eos; ADVANCE_CHAR) {
		const char c = *pos;

		switch (token.type) {
		case TOKEN_EMPTY:
			break;

		case TOKEN_WHITESPACE:
			if (CC(ccHorzWs|ccVertWs))
				continue;
			return token;

		case TOKEN_IDENTIFIER:
			if (CC(ccIdent|ccDigit))
				continue;
			return token;

		case TOKEN_INTEGER:
			if (CC(ccDot))  {
				TT(TOKEN_FLOAT);
				continue;
			} else if (CC(ccDigit|ccHex))
				continue;
			return token;

		case TOKEN_FLOAT:
			if (CC(ccDigit|ccExp))
				continue;
			if ((c == '+' || c == '-') &&
				(char_class[(unsigned char)pos[-1]] & ccExp))
				continue;
			return token;

		case TOKEN_CHARACTER:
			if (escape)    { escape = 0; continue; }
			if (c == '\\') { escape = 1; continue; }
			if (c != '\'') continue;
			ADVANCE_CHAR;
			return token;

		case TOKEN_STRING:
			if (escape)    { escape = 0; continue; }
			if (c == '\\') { escape = 1; continue; }
			if (c != '"') continue;
			ADVANCE_CHAR;
			return token;

		case TOKEN_DOT:
			if (CC(ccDigit))
				TT(TOKEN_FLOAT);
			else if (CC(ccDot))
				TT(TOKEN_DOT_DOT);
			else
				return token;
			continue;

		case TOKEN_COMMENT:
			if (token.string[1] == '/' && CC(ccVertWs)) {
				ADVANCE_CHAR;
				return token;
			} else if (token.string[1] == '*' &&
				pos[-1] == '*' && c == '/') {
				ADVANCE_CHAR;
				return token;
			}
			continue;

		case TOKEN_NOT:       S1('=', TOKEN_NE);
		case TOKEN_MOD:       S1('=', TOKEN_ASSIGN_MOD);
		case TOKEN_ASTERISK:  S1('=', TOKEN_ASSIGN_MUL);
		case TOKEN_SHL:       S1('=', TOKEN_ASSIGN_SHL);
		case TOKEN_SAR:       S1('=', TOKEN_ASSIGN_SAR);
		case TOKEN_NUMSIGN:   S1('#', TOKEN_NUMSIGN_NUMSIGN);
		case TOKEN_COLON:     S1(':', TOKEN_COLON_COLON);
		case TOKEN_ASSIGN:    S1('=', TOKEN_EQ);
		case TOKEN_BIT_XOR:   S1('=', TOKEN_ASSIGN_BIT_XOR);
		case TOKEN_DOT_DOT:   S1('.', TOKEN_ELLIPSIS);
		case TOKEN_LT:        S2('<', TOKEN_SHL,
				         '=', TOKEN_LE);
		case TOKEN_GT:        S2('>', TOKEN_SAR,
				         '=', TOKEN_GE);
		case TOKEN_AMPERSAND: S2('&', TOKEN_AND,
				         '=', TOKEN_ASSIGN_BIT_AND);
		case TOKEN_BIT_OR:    S2('|', TOKEN_OR,
				         '=', TOKEN_ASSIGN_BIT_OR);
		case TOKEN_PLUS:      S2('+', TOKEN_INC,
				         '=', TOKEN_ASSIGN_ADD);
		case TOKEN_MINUS:     S3('-', TOKEN_DEC,
					 '>', TOKEN_DEREFERENCE,
					 '=', TOKEN_ASSIGN_SUB);

		case TOKEN_DIV:
		      if (c == '/' || c == '*')
			      TT(TOKEN_COMMENT);
		      else if (c == '=')
			      TT(TOKEN_ASSIGN_DIV);
		      else
			      return token;
		      continue;

		default:
		      return token;
		}

		if (CC(ccDigit))
			TT(TOKEN_INTEGER);
		else if (CC(ccHorzWs|ccVertWs))
			TT(TOKEN_WHITESPACE);
		else
			switch (c) {
			case '<':  TT(TOKEN_LT);          break;
			case '>':  TT(TOKEN_GT);          break;
			case '=':  TT(TOKEN_ASSIGN);      break;
			case '!':  TT(TOKEN_NOT);         break;
			case '+':  TT(TOKEN_PLUS);        break;
			case '-':  TT(TOKEN_MINUS);       break;
			case '*':  TT(TOKEN_ASTERISK);    break;
			case '%':  TT(TOKEN_MOD);         break;
			case '/':  TT(TOKEN_DIV);         break;
			case '&':  TT(TOKEN_AMPERSAND);   break;
			case '|':  TT(TOKEN_BIT_OR);      break;
			case '^':  TT(TOKEN_BIT_XOR);     break;
			case '~':  TT(TOKEN_BIT_NOT);     break;
			case '.':  TT(TOKEN_DOT);         break;
			case ':':  TT(TOKEN_COLON);       break;
			case '\\': TT(TOKEN_BACKSLASH);   break;
			case ';':  TT(TOKEN_SEMICOLON);   break;
			case ',':  TT(TOKEN_COMMA);       break;
			case '(':  TT(TOKEN_LPAREN);      break;
			case ')':  TT(TOKEN_RPAREN);      break;
			case '[':  TT(TOKEN_LBRACK);      break;
			case ']':  TT(TOKEN_RBRACK);      break;
			case '{':  TT(TOKEN_LBRACE);      break;
			case '}':  TT(TOKEN_RBRACE);      break;
			case '?':  TT(TOKEN_CONDITIONAL); break;
			case '"':  TT(TOKEN_STRING);      break;
			case '\'': TT(TOKEN_CHARACTER);   break;
			case '#':  TT(TOKEN_NUMSIGN);     break;

			default:
				if (!CC(ccIdent)) {
					TT(TOKEN_ERROR_INVALID_CHARACTER);
					return token;
				}

				TT(TOKEN_IDENTIFIER);
			}
	}

	if (token.type == TOKEN_STRING)
		TT(TOKEN_ERROR_END_OF_STRING);
	else if (token.type == TOKEN_CHARACTER)
		TT(TOKEN_ERROR_END_OF_CHARACTER);
	else if (token.type == TOKEN_COMMENT && token.string[1] == '*')
		TT(TOKEN_ERROR_END_OF_COMMENT);

	return token;
}

struct lexc_token lexc_next_token(struct lexc_token token)
{
	do {
		token = lexc_next_any_token(token);
	} while (token.type != TOKEN_EMPTY &&
		(token.type == TOKEN_WHITESPACE ||
		 token.type == TOKEN_COMMENT));

	return token;
}

static struct lexc_token first_token(const char *s, const char *eos)
{
	struct lexc_token token = {
		.type = TOKEN_EMPTY,
		.string = s,
		.length = 0,
		.line = 1,
		.column = 1,
		.eos = eos ? eos : &s[strlen(s)]
	};

	return token;
}

struct lexc_token lexc_first_any_token(const char *s, const char *eos)
{
	return lexc_next_any_token(first_token(s, eos));
}

struct lexc_token lexc_first_token(const char *s, const char *eos)
{
	return lexc_next_token(first_token(s, eos));
}

bool lexc_equal_token(struct lexc_token a, struct lexc_token b)
{
	return a.type == b.type &&
		a.length == b.length &&
		strncmp(a.string, b.string, a.length) == 0;
}

const char *lexc_token_type_str(enum lexc_token_type type)
{
	switch (type) {
#define LEXC_TOKEN_TYPE_STR(identifier, description)			\
	case TOKEN_##identifier: return description;
	LEXC_TOKENS(LEXC_TOKEN_TYPE_STR)
	}

	return "<invalid>";
}
