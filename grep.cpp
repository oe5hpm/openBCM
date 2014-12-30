/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  ---------------------------------------------------
  Suchroutinen mit Moeglichkeit regulaerer Ausdruecke
  ---------------------------------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved


  Parts adoped from: "grep.c for VMS"  Copyright (C) 1980, DECUS

  * General permission to copy or modify, but not for profit,  is
  * hereby  granted,  provided that the above copyright notice is
  * included and reference made to  the fact  that  reproduction
  * privileges were granted by DECUS.

 ***************************************************************/

//19980414 OE3DZW fixed endless loop when reg-expression contains "**"

#include "baycom.h"

/*---------------------------------------------------------------------------*/

CRegEx::CRegEx()
//****************************************************************************
//
//****************************************************************************
{
  m_options = 256;
}

/*---------------------------------------------------------------------------*/

int CRegEx::regex_compile (char *source)
//*************************************************************************
//
//  Compile the pattern
//
//*************************************************************************
{
  char *s = source;         // Source string pointer
  char *lp = NULL;          // Last pattern pointer
  int c;                    // Current character
  char *spp;                // Save beginning of pattern
  char *pp = m_pattern + 2;
  char nextstore;
  int quote = 0;

  m_pattern[0] = QUICK;
  m_pattern[1] = CASESENSE;
  if (strstr(s, "**") || strstr(s, "*+")) goto error; //quick fix for ugly bug
  if (s[0] == '"')
  {
    s++;
    quote = 1;
  }
  while ((c = *s++) != 0)
  {
    if (c == '*' || c == '+') // STAR, PLUS are special.
    {
      if (! lp) goto error;
      switch (pp[-1])
      {
      case BOL:
      case EOL:
      case STAR:
      case PLUS: goto error;
      }
      pp[0] = pp[1] = ENDPAT;
      pp += 2;
      spp = pp; // Save pattern end
      while (--spp > lp) spp[0] = spp[-1]; // Move pattern down one byte
      spp[0] = (c == '*') ? STAR : PLUS;
      continue;
    }
    // All the rest.
    nextstore = 0;
    lp = pp; // Remember start
    switch (c)
    {
    case '^': nextstore = BOL; break;
    case '$': nextstore = EOL; break;
    case '.': nextstore = ANY; break;
    case '[':
      if (s[0] == '^')
      {
        ++s;
        *pp++ = NCLASS;
      }
      else *pp++ = CLASS;
      spp = pp++;              // spare for byte count
      while ((c = *s++) != ']')
      {
        if (! c) goto error;
        if (c == '\\')    // Store quoted char
        {
          if ((c = *s++) == 0) goto error;
          else *pp++ = c;
        }
        else if (c == '-' && (pp - spp) > 1 && s[0] != ']' && s[0])
        {
          c = pp[-1];       // Range start
          pp[-1] = RANGE;   // Range signal
          *pp++ = c;        // Re-store start
          *pp++ = s[0];
          s++;
        }
        else *pp++ = c;
      }
      spp[0] = (char) (pp - spp);
      break;
    case '"':
      if (quote) while (*s) s++;
      else  goto def;
      break;
    case '\\':
      if (*s) c = *s++;
    default:
 def: *pp++ = CHAR;
      nextstore = c;
    }
    if (nextstore) *pp++ = nextstore;
  }
  *pp++ = ENDPAT;
  *pp++ = 0;                // Terminate string
  pp = m_pattern + 2;
  while (pp[0])
  {
    if (pp[0] > CHAR && pp[0] < ENDPAT) m_pattern[0] = REGEX;
    pp++;
  }
  if (strchr(source, '\\')) m_pattern[0] = REGEX;

  if (m_pattern[0] == QUICK || (m_options & o_q))
  {
    strcpy(m_pattern + 2, source + quote);
    pp = strchr(m_pattern, '"');
    if (pp && quote) pp[0] = 0;
    m_pattern[0] = QUICK;
  }
  if (m_options & o_i)
  {
    m_pattern[1] = TOUPPER;
    strupr(m_pattern);
  }
  if (m_options & o_r)
  {
    putf("RegEx: \"");
    pp = m_pattern;
    while (*pp)
    {
      if (*pp > ' ') putf("%c", pp[0]);
      else putf("(%d)", pp[0]);
      pp++;
    }
    putf("\"\n");
  }
  return 1;

error:
  return 0;
}

/*---------------------------------------------------------------------------*/

char *CRegEx::pmatch (char *begin, char *line, char *pattern)
//*************************************************************************
//
//
//
//*************************************************************************
{
  char *l = line;           // Current line pointer
  char *p = pattern;        // Current pattern pointer
  char c;                   // Current character
  char *e;                  // End for STAR and PLUS match
  int op;                   // Pattern operation
  int n;                    // Class counter
  char *are;                // Start of STAR match

  while ((op = *p++) != ENDPAT)
  {
    c = l[0];
    switch (op)
    {
    case CHAR:
      if (c != *p++) goto notmatch;
      l++;
      break;
    case BOL:
      if (begin != l) goto notmatch;
      break;
    case EOL:
      if (c != LF) goto notmatch;
      break;
    case ANY:
      l++;
      if (c == LF) goto notmatch;
      break;
    case CLASS:
      l++;
      n = *p++;
      do
      {
        if (p[0] == RANGE)
        {
          p += 3;
          n -= 2;
          if (c >= p[-2] && c <= p[-1]) break;
        }
        else if (c == *p++) break;
      }
      while (--n > 1);
      if (n <= 1) goto notmatch;
      p += n - 2;
      break;
    case NCLASS:
      l++;
      n = *p++;
      do
      {
        if (p[0] == RANGE)
        {
          n -= 2;
          if (c >= p[1] && c <= p[2]) goto notmatch;
          p += 3;
        }
        else if (c == *p++) goto notmatch;
      }
      while (--n > 1);
      break;
    case PLUS:            // One or more ...
      l = pmatch(begin, l, p);
      if (! l) goto notmatch;    // Gotta have a match
    case STAR:            // Zero or more ...
      are = l;            // Remember line start
      while (l[0] && (e = pmatch(begin, l, p)) != NULL)
        l = e;            // Get longest match
      while (*p++ != ENDPAT);   // Skip over pattern
      while (l >= are)    // Try to match rest
      {
        e = pmatch(begin, l, p);
        if (e) return e;
        --l;              // Nope, try earlier
      }
      goto notmatch;      // Nothing else worked
    }
  }
  return l;

notmatch:
  return NULL;
}

/*---------------------------------------------------------------------------*/

int CRegEx::regex_match (char *pattern, char *line)
//****************************************************************************
//
//****************************************************************************
{
  regex_compile(pattern);
  return regex_match(line);
}

/*---------------------------------------------------------------------------*/

int CRegEx::regex_match (char *line)
//*************************************************************************
//
//  Match the current line, return 1 if it does.
//
//*************************************************************************
{
  static char lbuf[LMAX];
  char *lbufp = lbuf;

  lbuf[0] = 1;
  strcpy(lbuf + 1, line);
  if (m_pattern[1] == TOUPPER) strupr(lbuf + 1);
  if (m_pattern[0] == QUICK) return strstr(lbuf + 1, m_pattern + 2) != NULL;
  else
  {
    while (lbufp[0])
    {
      if (pmatch(lbuf + 1, lbufp, m_pattern + 2)) return 1;
      lbufp++;
    }
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

int CRegEx::grepfile (FILE * fp)
//*************************************************************************
//
//  Scan the file for the pattern in pbuf[]
//
//*************************************************************************
{
  unsigned int lno = 0, count = 0, n;
  char obuf[LMAX];

  while (fgets(obuf, LMAX - 2, fp))
  {
    ++lno;
    n = regex_match(obuf);
    if ((n && ! (m_options & o_v)) || (! n && (m_options & o_v)))
    {
      ++count;
      if (! (m_options & o_c))
      {
        if (m_options & o_n)
          putf("%d:", lno);
        putf("%s", obuf);
      }
    }
    if ((lno % 50) == 0)
    {
      waitfor(e_ticsfull);
      if (testabbruch()) break;
    }
  }
  if ((m_options & o_c) && ! (m_options & o_x))
    putf(ms(m_matchinglines), count);
  return count;
}

/*---------------------------------------------------------------------------*/

int grep (char *name, char *pattern, bitfeld options)
//*************************************************************************
//
//
//
//*************************************************************************
{
  FILE *f;
  CRegEx cr;
  int matches = 0;
  cr.m_options = options;

  if (name[0])
  {
    if (cr.regex_compile(pattern))
    {
      f = s_fopen(name, "srt");
      if (f)
      {
        setvbuf(f, NULL, _IOFBF, 4096);
        matches = cr.grepfile(f);
        s_fclose(f);
      }
      else
        putf(ms(m_filenotopen), name);
    }
    else
      putf(ms(m_badregex));
  }
  return matches;
}
