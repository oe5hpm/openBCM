/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  --------------------------
  Convers-Modus fuer Mailbox
  --------------------------


  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19981001 OE3DZW removed obsolete parts of convers-code
//                added putflush
//19991118 DF3VI  no talk to user that is not logged in
//19991118 DF3VI  change talk adress permanent or temporary
//19991118 DF3VI  give little help
//19991119 DF3VI  execute bbs-comand with !cmd
//                mailbef() needs change: don't convers and logouts if convers
//19991127 DF3VI  finished convers
//20000111 OE3DZW remove "!" command under DOS, needs too much stack
//                dynamic allocation of buffer

#include "baycom.h"

/*---------------------------------------------------------------------------*/

int conversout (int prompt)
//*************************************************************************
//
//*************************************************************************
{
  char s[LINELEN+1];
  unsigned num;
  int ok = 0;
  conv_t *cp;

  while (b->conv)
  {
    cp = b->conv;
    b->conv = cp->next;
    if (prompt) putv(LF);
    if (cp->line[1] == 1)
    {
      if (sscanf(cp->line + 5, "%s %d", s, &num) == 2)
      {
        putf("*SYSTEM*: ");
        putf(ms(m_msgarrived), s, num);
        strcpy(b->prompt, b->logincall);
        putv(LF);
        sprintf(s, "%d", num);
        b->optgroup = o_id;
        mbdir(s, 0);
      }
    }
    else
      putf("%s\n", cp->line);
    t_free(cp);
    ok = 1;
    prompt = 0;
  }
  return ok;
}

/*---------------------------------------------------------------------------*/

void convers_msg (int pending, char *befname)
//*************************************************************************
//
//*************************************************************************
{
  strupr(befname);
  if (! pending)
    putf(ms(m_conv_notloggedon), befname);
  else if (pending == 10 || pending == 15)
    putf(ms(m_conv_linespending), befname, pending);
  else if (pending > 19)
    putf(ms(m_conv_toomanypending), befname);
  return;
}

/*---------------------------------------------------------------------------*/

void convers (char *befbuf, char *befname)
//*************************************************************************
//
//*************************************************************************
{
  char *s = (char *) t_malloc(BUFLEN, "conv"); //save stack
  int conversaktiv = 1;
  int pending;
  char *msg;

  msg = strchr(befbuf, ' ');
  if (msg) *msg++ = 0;
  strupr(befbuf);
  b->job = j_convers;
  if (eingelogt(befbuf, 0, 0) || ! strcmp(befbuf, "ALL"))
  {
    if (! msg)
    {
      putf(ms(m_conversto), befname, befbuf);
      while (conversaktiv)
      {
        conversout(0);
        if (getvalid())
        {
          getline(s, BUFLEN - 1, 1);
          if (*s == '!')
          {
            mailbef(s + 1, 0);
            putf(ms(m_conversto), befname, befbuf);
            strcpy(s, befname);
            strcat(s, " ");
            strcat(s, befbuf);
            lastcmd(s);
            b->job = j_convers;
          }
          else
          if (*s == '/')
          {
            switch (toupper(s[1]))
            {
              case 'Q':
              case 'B':
                putf(ms(m_conversend), befname);
                conversaktiv = 0;
                break;
              case 'U':
              case 'W': listusers(1); break;
              case 'T':  //Talk-Befehl
              case 'M':
              case 'C':
              case 'S':
                if (strchr(s, ' ')) // mit Argument
                {
                  msg = strchr(s, ' ');
                  while (*msg == ' ') msg++;
                  if (! strchr(msg, ' ')) // EIN Arg: Neues Ziel
                  {
                    strupr(msg);
                    if ((eingelogt(msg, 0, 0) || (! stricmp(msg, "ALL"))))
                    {
                      strcpy(befbuf, msg); // Wenn eingeloggt, umschalten
                      strupr(befbuf);
                      putf(ms(m_conversto), befname, befbuf);
                      strcpy(s, befname);
                      strcat(s, " ");
                      strcat(s, befbuf);
                      lastcmd(s);
                    }
                    else
                      convers_msg(0, msg); // Sonst Fehlermeldung
                  }
                  else // Zwei o. mehr Argumente
                  {
                    char newcall[CALLEN+1];
                    msg = nextword(msg, newcall, CALLEN);
                    newcall[CALLEN] = 0;
                    strupr(newcall);
//                  subst1(newcall, ' ', 0);
                    if (eingelogt(newcall, 0, 0) || ! stricmp(newcall, "ALL"))
                    {
//                    while (*msg!=' ') msg++; // Call ueberlesen
                      pending = mbtalk(b->logincall, newcall, msg); // Text raus
                      convers_msg(pending, befname);
                    }
                    else
                    {
//                    subst1(msg, ' ', 0);
                      convers_msg(0, newcall); // Sonst Fehlermeldung
                    }
                  }
                }
                else // Kein Arg: Zielcall anzeigen
                  putf(ms(m_conversto), befname, befbuf);
                break;
              default: // Fehlermeldung
                putf("/Quit /Bye /User /Who /Talk /Msg /Chat /Send !<cmd>\n");
            }
          }
          else if (*s)
          {
            if (conversaktiv)
            {
              pending = mbtalk(b->logincall, befbuf, s);
              convers_msg(pending, befname);
            }
          }
        }
        putflush();
        timeout(m.usrtimeout);
        wdelay(152);
      }
    }
    else
    {
      pending = mbtalk(b->logincall, befbuf, msg);
      convers_msg(pending, befname);
    }
  }
  else
    if (mbcallok(befbuf)) convers_msg(0, befbuf);
    else putf(ms(m_syntaxconvers), befname);
  t_free(s);
}

/*---------------------------------------------------------------------------*/
