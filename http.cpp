/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -------------------
  HTTP-Server for BCM   Reference: RFC 1945
  -------------------

  Copyright (C)       Florian Radlherr
                      Taubenbergstr. 32
                      83627 Warngau

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980115 OE3DZW utcoffset -> ad_timezone, increased expire by 800s
//19980329 OE3DZW fixed send
//19980428 OE3DZW fixed conversion of to/subject
//19980505 OE3DZW l baybox -30 statt st l in Leiste, logout als
//                remove cookie, "home" statt back
//19980525 DG9MHZ added support for MS IE ;-((( (fixed parser bug)
//19980916 OE3DZW added signature
//19981013 OE3DZW added debug output to find bug in http-post (send mails)
//1999xxxx Jan    cleaned up HTML syntax, better noframe support, 7+ download
//19991122 Jan    fixed HTTP version determination, fixed send contents parsing
//                large security fixes, clean-up
//19991211 Jan    read is now normal cmd, longer expire
//19991212 Jan    removed "Go back" link, was pointing back to /command
//                which is not what we want to have in bottom frame
//20000108 OE3DZW fixed (twice) protocol-check of http
//20000115 Jan    removed obsolete HTTP/0.9 for good, most WWW servers (Apache)
//                don't care about HTTP version
//20021205 DH8YMB createlogin if m.guestcall & _GUEST hinzu, some other changes
//20021208 DH8YMB Login-Callsign in ROT
//20021212 DH8YMB Guestcall-Fehlermeldung hinzu
//20030101 hpk    in the CB-BCNNET login-concept: if user is
//                HTTP-Authenticated, logintype will be set to 1 (full-login)
//20050401 DH8YMB added frameless CSS Style

#include "baycom.h"
#define BGCOL "\"#FFFFFF\""
#define HTTPSIGSTR "OBCMHTTPD"

//define this if you want files under bcm/http/ to be accessible without pw
//increases possibility of security-hole in something world-open
//#define FILES_WITHOUT_PW

/*---------------------------------------------------------------------------*/

class httpd
{
public:
  httpd (void);
  ~httpd (void);
  void start_http (char *name);

private:
  // data
  char signature[sizeof(HTTPSIGSTR)];
  enum { GET, HEAD, POST } method;
  enum { NONE, BIN, TXT, GIF, JPG, WAV} _mimetype;
  char uri[200];
  char login[CALLEN+1];
  char cookie_login[CALLEN+1];
  char pw[NAMELEN+1];
  char cookie_pw[NAMELEN+1];
  char guestpw[NAMELEN+5];
  int  status;
  int  logout;
  int  userlogin, guestlogin;
  char file[80];
  char userpass[50];
//  char referer[100];
  char host[100];
  long contentlength;
  char *content;
  int nocookie;
  int mimetype;
  int httpsurface;
//  user_t uu;

  // methods
  char *ht_time (time_t tt);
  void generate_css (void);
  void base64bin (char *in, char *out, int maxlen);
  void get_authorization (char *buf, int cookie);
  void get_contentlength (char *buf);
  void get_field (char *buf, char *dest, unsigned maxlen);
//  void form_referer(void);
  void get_request (void);
  void get_postarea (char *tag, char *out, int max, unsigned wrap = 0);
  void put_header (char *title);
  void put_homepage (char *cmd);
  void put_css_footer (void);
  void put_frame (void);
  void put_sendform (char *to, char *lt, char *subj, char *read);
  void put_createlogin (char *createlogin, char *createpw);
//  void postmsg (char *in, char *out, int outlen);
};

/*---------------------------------------------------------------------------*/

httpd::httpd (void)
//*************************************************************************
//
//  Initialization
//
//*************************************************************************
{
  strcpy(signature, HTTPSIGSTR);
}

/*---------------------------------------------------------------------------*/

httpd::~httpd (void)
//*************************************************************************
//
//  Deinitialization
//
//*************************************************************************
{
  if (strcmp(signature, HTTPSIGSTR))
    trace(fatal, "httpd", "sig broken");
}

/*---------------------------------------------------------------------------*/

char *httpd::ht_time (time_t ut)
//*************************************************************************
//
//  Gibt RFC-Datum als String zurueck
//
//*************************************************************************
{
  return datestr(ut - ad_timezone(), 18); // UTC RFC-Date
}

/*---------------------------------------------------------------------------*/

void httpd::generate_css (void)
//*************************************************************************
//
// Generiert die default CSS-Style Datei
//
//*************************************************************************
{
  FILE *cssfile = NULL;
  FILE *backfile = NULL;

   if ((cssfile = s_fopen("http/style.css", "srt")) != NULL) {}
   else
   {
    cssfile = s_fopen("http/style.css", "sat"); //schreibend oeffnen
    fputs("/* Schriftstil, Abstaende */\n", cssfile);
    fputs("body\n", cssfile);
    fputs("{\n", cssfile);
    fputs("margin-top:8px;\n", cssfile);
    fputs("margin-left:8px;\n", cssfile);
    fputs("margin-right:8px;\n", cssfile);
    fputs("margin-bottom:8px;\n", cssfile);
    fputs("color:black;\n", cssfile);
    fputs("background-color:#ffffff;\n", cssfile);
    backfile = s_fopen("http/back.jpg", "srt");
    if (backfile)
    {
      s_fclose(backfile);
      fputs("background-image:url(back.jpg);\n", cssfile);
    }
    fputs("font-family:verdana,arial;\n", cssfile);
    fputs("font-size: 10pt;\n", cssfile);
    fputs("}\n", cssfile);
    fputs("table,tr,td\n", cssfile);
    fputs("{\n", cssfile);
    fputs("color:black;\n", cssfile);
    fputs("font-family:verdana,arial;\n", cssfile);
    fputs("font-size: 10pt\n", cssfile);
    fputs("}\n", cssfile);
    fputs("/* Scrollleiste Internet Explorer ab v5.5 */\n", cssfile);
    fputs("body\n", cssfile);
    fputs("{\n", cssfile);
    fputs("scrollbar-arrow-color:#808080;\n", cssfile);
    fputs("scrollbar-base-color:white;\n", cssfile);
    fputs("scrollbar-highlight-color:#808080;\n", cssfile);
    fputs("scrollbar-shadow-color:#000000;\n", cssfile);
    fputs("SCROLLBAR-TRACK-COLOR:#cccccc;\n", cssfile);
    fputs("}\n", cssfile);
    fputs("/* Menue */\n", cssfile);
    fputs("#menu a\n", cssfile);
    fputs("{\n", cssfile);
    fputs("display:block;\n", cssfile);
    fputs("background-color:#ffffff;\n", cssfile);
    fputs("color:black;\n", cssfile);
    fputs("text-decoration:none;\n", cssfile);
    fputs("font-family:verdana,sans-serif;\n", cssfile);
    fputs("font-size:10pt;\n", cssfile);
    fputs("width:140px;\n", cssfile);
    fputs("border-bottom:solid 1px #ffffff;\n", cssfile);
    fputs("border-top:solid 1px #ffffff;\n", cssfile);
    fputs("}\n", cssfile);
    fputs("#menu a:visited\n", cssfile);
    fputs("{\n", cssfile);
    fputs("background-color:#ffffff;\n", cssfile);
    fputs("color:black;\n", cssfile);
    fputs("text-decoration:none;\n", cssfile);
    fputs("font-family:verdana,sans-serif;\n", cssfile);
    fputs("font-size:10pt;\n", cssfile);
    fputs("width:140px;\n", cssfile);
    fputs("border-bottom:solid 1px #ffffff;\n", cssfile);
    fputs("border-top:solid 1px #ffffff;\n", cssfile);
    fputs("}\n", cssfile);
    fputs("#menu a:active\n", cssfile);
    fputs("{\n", cssfile);
    fputs("background-color:#ffffff;\n", cssfile);
    fputs("color:black;\n", cssfile);
    fputs("text-decoration:none;\n", cssfile);
    fputs("font-family:verdana,sans-serif;\n", cssfile);
    fputs("font-size:10pt;\n", cssfile);
    fputs("width:140px;\n", cssfile);
    fputs("border-bottom:solid 1px #ffffff;\n", cssfile);
    fputs("border-top:solid 1px #ffffff;6;\n", cssfile);
    fputs("}\n", cssfile);
    fputs("#menu a:hover\n", cssfile);
    fputs("{\n", cssfile);
    fputs("background-color:#e6e6e6;\n", cssfile);
    fputs("color:black;\n", cssfile);
    fputs("text-decoration:none;\n", cssfile);
    fputs("font-family:verdana,sans-serif;\n", cssfile);
    fputs("font-size:10pt;\n", cssfile);
    fputs("width:140px;\n", cssfile);
    fputs("border-bottom:solid 1px #000000;\n", cssfile);
    fputs("border-top:solid 1px #000000;\n", cssfile);
    fputs("}\n", cssfile);
    fputs("/* Menuetitel */\n", cssfile);
    fputs(".rubrik\n", cssfile);
    fputs("{\n", cssfile);
    fputs("background-color:#9198ab;\n", cssfile);
    fputs("color:white;\n", cssfile);
    fputs("text-decoration:none;\n", cssfile);
    fputs("font-family:verdana,sans-serif;\n", cssfile);
    fputs("font-size:10pt;\n", cssfile);
    fputs("width:140px;\n", cssfile);
    fputs("border-bottom:solid 1px #000000;\n", cssfile);
    fputs("border-top:solid 1px #000000;\n", cssfile);
    fputs("height:19px;\n", cssfile);
    fputs("}\n", cssfile);
    fputs("/* Fuss- und Kopfleiste */\n", cssfile);
    fputs(".leiste\n", cssfile);
    fputs("{\n", cssfile);
    fputs("background-color:#9198ab;\n", cssfile);
    fputs("color:white;\n", cssfile);
    fputs("text-decoration:none;\n", cssfile);
    fputs("font-family:verdana,sans-serif;\n", cssfile);
    fputs("font-size:10pt;\n", cssfile);
    fputs("height:17px;\n", cssfile);
    fputs("}\n", cssfile);
    fputs("/* Schriftform und -farbe von allgemeinen Links */\n", cssfile);
    fputs("a:link\n", cssfile);
    fputs("{\n", cssfile);
    fputs("color:blue;\n", cssfile);
    fputs("text-decoration:underline;\n", cssfile);
    fputs("font-family:courier;\n", cssfile);
    fputs("font-size:10pt;\n", cssfile);
    fputs("}\n", cssfile);
    fputs("a:visited\n", cssfile);
    fputs("{\n", cssfile);
    fputs("color:blue;\n", cssfile);
    fputs("text-decoration:underline;\n", cssfile);
    fputs("font-family:courier;\n", cssfile);
    fputs("font-size:10pt;\n", cssfile);
    fputs("}\n", cssfile);
    fputs("a:active\n", cssfile);
    fputs("{\n", cssfile);
    fputs("color:blue;\n", cssfile);
    fputs("text-decoration:underline;\n", cssfile);
    fputs("font-family:courier;\n", cssfile);
    fputs("font-size:10pt;\n", cssfile);
    fputs("}\n", cssfile);
    fputs("a:hover\n", cssfile);
    fputs("{\n", cssfile);
    fputs("color:#808080;\n", cssfile);
    fputs("background-color:white;\n", cssfile);
    fputs("text-decoration:none;\n", cssfile);
    fputs("font-family:courier;\n", cssfile);
    fputs("font-size:10pt;\n", cssfile);
    fputs("}\n", cssfile);
    fputs("form\n", cssfile);
    fputs("{\n", cssfile);
    fputs("margin-bottom:0px;\n", cssfile);
    fputs("}\n", cssfile);
    trace(report, "generate_css", "default http/style.css created");
   }
   s_fclose(cssfile);
}

/*---------------------------------------------------------------------------*/

int isamprnet (char *ip)
//*************************************************************************
//
// Stellt fest, ob es sich um eine Ampr-Net Adresse handelt.
//
//*************************************************************************
{
 if (m.httpttypw)
   return 0;
 else
 {
   if (! strncmp(ip, "44.", 3)) return 2;
   else if (! strncmp(ip, "127.", 4)) return 1;
   else return 0;
 }
}

/*---------------------------------------------------------------------------*/

void httpd::base64bin (char *in, char *out, int maxlen)
//*************************************************************************
//
// highly simplified base64-decoder, according RFC1521
// constraints: - no line feeds
//              - no inappropriate characters (lead to abort of conversion)
//              - no zero bytes within data
//
//*************************************************************************
{
  int i, a, end = 0;
  long outword = 0;

  while (*in && ! end && maxlen > 3)
  {
    for (i = 0; i < 4; i++)
    {
      if (! in[i]) end = 1;
      outword <<= 6;
      a = in[i];
      if (isalpha(a) && isupper(a)) a -= ('A');
      else if (isalpha(a) && islower(a)) a -= ('a' - 26);
      else if (isdigit(a)) a += 4;
      else if (a == '+') a = 62;
      else if (a == '/') a = 63;
      else a = 0; // text only! Padding with 0-Bytes
      outword |= a;
    }
    out[0] = (outword >> 16) & 255; // slow, but portable (byte order!)
    out[1] = (outword >> 8) & 255;
    out[2] = (outword) & 255;
    in += 4;
    out += 3;
    maxlen -= 3;
  }
  *out = 0;
}

/*---------------------------------------------------------------------------*/

void httpd::get_contentlength (char *buf)
//*************************************************************************
//
//*************************************************************************
{
  char *dop;

  dop = strchr(buf, ':');
  if (dop) contentlength = atol(dop + 1);
}

/*---------------------------------------------------------------------------*/

void httpd::get_field (char *buf, char *dest, unsigned maxlen)
//*************************************************************************
//
//*************************************************************************
{
  char *dop;

  dop = strchr(buf, ':');
  if (dop)
  {
    dop++;
    while (*dop == ' ') dop++;
    if (strlen(dop) >= maxlen) dop[maxlen - 1] = 0;
    strcpy(dest, dop);
  }
  else *dest = 0;
}

/*---------------------------------------------------------------------------*/

//void httpd::form_referer(void)
//*************************************************************************
//
//*************************************************************************
/*
{
  char tmp[80];
  char *hostpos = stristr(referer, host);

  if (referer[0] && host[0] && hostpos)
  {
    if ((strlen(hostpos + strlen(host)) > 79))
    {
      trace(serious, "httpd", "referer str too long, ip[%s]", b->peerip);
      strcpy(referer, "/");
      return;
    }
    strcpy(tmp, hostpos + strlen(host));
    strcpy(referer, tmp);
  }
  else strcpy(referer, "/");
}*/

/*---------------------------------------------------------------------------*/

void httpd::get_authorization (char *buf, int cookie)
//*************************************************************************
//
//*************************************************************************
{
  char *search;
  char *basic;
  char loginpw[50];
  char *locpw;

  if (cookie) search = m.boxname;
  else search = "Basic";
  basic = strstr(buf, search);
  if (basic)
  {
    basic += strlen(search);
    if (*basic) basic++;
    safe_strcpy(userpass, basic);
    base64bin(basic, loginpw, 49);
    locpw = strchr(loginpw, ':');
    if (locpw && locpw[0] && locpw[1])
    {
      if (cookie)
        safe_strcpy(cookie_pw, locpw + 1)
      else
        safe_strcpy(pw, locpw + 1)
      *locpw = ' ';
    }
    locpw = strchr(loginpw, ' ');
    if (locpw) *locpw = 0;
    loginpw[NAMELEN] = 0;
    if (cookie)
      strcpy(cookie_login, loginpw);
    else
      strcpy(login, loginpw);
  }
}

/*---------------------------------------------------------------------------*/

void httpd::get_request (void)
//*************************************************************************
//
//*************************************************************************
{
  char locinbuf[301];
  char *vers;

  getline(locinbuf, sizeof(locinbuf) - 1, 1);
  if ((m.tcpiptrace == 1) || (m.tcpiptrace == 8)) httplog("RX", locinbuf);
  char *locmethod = locinbuf;
  char *locuri = skip(locmethod);
  char *locprotocol = skip(locuri);
  char userpw[NAMELEN+5];

  skip(locprotocol);
  contentlength = 0L;
  content = NULL;
  userpass[0] = 0;
  status = 200;
  logout = 0;
  userlogin = 0;
  guestlogin = 0;
  *file = 0;
  *login = 0;
  *pw = 0;
  *cookie_login = 0;
  *cookie_pw = 0;
  nocookie = 0;
//  referer[0]=0;
  mimetype = NONE;
  *uri = 0;
  *host = 0;

  if (m.disable)
  {
    status = 503;
    return;
  }
  if (! locmethod || ! locuri || ! locprotocol || strlen(locmethod) > 10
      || strlen(locuri) > (sizeof(uri) - 2) || strlen(locprotocol) > 10)
  {
  //don't waste time with that ..
    status = 400;
    return;
  }
  //determine protocol version
  vers = stristr(locprotocol, "HTTP/");
  if (vers != locprotocol)
  {
    status = 400;
    return;
  }
  //check URL
  if (*locuri != '/')
  {
    status = 400;
    return;
  }
  safe_strcpy(uri, locuri);
  if      (! stricmp(locmethod, "GET"))  method = GET;
  else if (! stricmp(locmethod, "HEAD")) method = HEAD;
  else if (! stricmp(locmethod, "POST")) method = POST;
  else
  {
    status = 501;
    return;
  }
  // check if we should send a file
  if (strchr(uri, '.') && ! strchr(uri, '?'))
  { // hm, yes it is a dirty hack...
    snprintf(file, sizeof(file), "http%s", locuri);
    if (strstr(file, "..")) //no "../../etc/passwd" constructions
    {
      status = 403;
      return;
    }
    if (file[strlen(file) - 1] == '/') file[strlen(file) - 1] = 0;
#ifndef _WIN32
    if (access(file, R_OK))
    {
      status = 404;
      return;
    }
#endif
  }
  if (! stricmp(uri, "/logout"))
  {
    status = 401;
    logout = 1;
    return;
  }
  if (! stricmp(uri, "/userlogin"))
  {
    if (m.httpguestfirst)
    {
      userlogin = 1;
      safe_strcpy(login, "");
      safe_strcpy(pw, "");
      safe_strcpy(cookie_login, "");
      safe_strcpy(cookie_pw, "");
    }
  }
#ifdef _GUEST
  if (! stricmp(uri, "/guestlogin"))
  {
    guestlogin = 0;
    safe_strcpy(login, "");
    safe_strcpy(pw, "");
    safe_strcpy(cookie_login, "");
    safe_strcpy(cookie_pw, "");
  }
#endif
  if (httpsurface == 0)
  {
  //check for valid url - this is really needed only if not using frames
  //when header is generated before cmd parsing
    if (strcmp(uri, "/") && strncmp(uri, "/cmd?", 5)
        && strncmp(uri, "/bread/", 7) && strncmp(uri, "/send?", 6)
        && strcmp(uri, "/send") && strcmp(uri, "/ask")
        && strcmp(uri, "/sendok") && strcmp(uri, "/askok")
        && strcmp(uri, "/userlogin")
        && strcmp(uri, "/login") && strcmp(uri, "/command")
#ifdef _GUEST
        && strcmp(uri, "/guestlogin")
#endif
        && ! (strchr(uri, '.') && ! strchr(uri, '?'))
     )
    {
      status = 404;
      return;
    }
  }
  else
  {
    if (strcmp(uri, "/") && strncmp(uri, "/cmd?", 5)
        && strncmp(uri, "/bread/", 7) && strncmp(uri, "/send?", 6)
        && strcmp(uri, "/send") && strcmp(uri, "/ask")
        && strcmp(uri, "/sendok") && strcmp(uri, "/askok")
        && strcmp(uri, "/userlogin")
#ifdef _GUEST
        && strcmp(uri, "/guestlogin")
#endif
        && ! (strchr(uri, '.') && ! strchr(uri, '?'))
     )
    {
     status = 404;
     return;
    }
  }
  do //request is ok .. now parse the headers
  {
    getline(locinbuf, sizeof(locinbuf) - 1, 1);
    if ((m.tcpiptrace == 1) || (m.tcpiptrace == 8))
      httplog("RX", locinbuf);
    if (*locinbuf)
    {
      if (stristr(locinbuf, "Authorization") == locinbuf)
      {
        get_authorization(locinbuf, 0);
      }
      if (stristr(locinbuf, "Cookie") == locinbuf)
        get_authorization(locinbuf, 1);
/*      if (stristr(locinbuf, "Referer") == locinbuf)
        get_field(locinbuf, referer, sizeof(referer)); */
      if (stristr(locinbuf, "Host") == locinbuf)
        get_field(locinbuf, host, sizeof(host));
      if (stristr(locinbuf, "Content-length") == locinbuf)
        get_contentlength(locinbuf);
    }
  }
  while (*locinbuf);
#ifdef _GUEST
  if (m.httpguestfirst || guestlogin)
  {
  //dh8ymb: falls author. fail und "not userlogin" und _guest
  //        dann login als guest
  if (! userlogin)
    if (((! *login && ! *cookie_login) || (! *pw && ! *cookie_pw)) )
    {
      safe_strcpy(login, m.guestcall);
      get_ttypw(m.guestcall, guestpw);
      safe_strcpy(pw, guestpw);
    }
  }
#endif
  if (((! *login && ! *cookie_login) || (! *pw && ! *cookie_pw))
#ifdef FILES_WITHOUT_PW
     && ! *file
#endif
     )
  { //shortcut ..
    status = 401;
    return;
  }
#ifdef FILES_WITHOUT_PW
  if (! *file)
  {
#endif
    // oe3dzw ttypw nur beim Drahtzugang, sonst Name des Users
    if (*login)
    {
      if (isamprnet(b->peerip) > 0)
        get_httppw(login, userpw);
      else
        get_ttypw(login, userpw);
      if (! *pw || stricmp(userpw, pw))
      {
        status = 401;
        pwlog(b->peerip, b->uplink, "bad password");
        return;
      }
      else b->pwok = OK;
    }
    else
    {
      if (isamprnet(b->peerip) > 0)
        get_httppw(cookie_login, userpw);
      else
        get_ttypw(cookie_login, userpw);
      if (! *cookie_pw || stricmp(userpw, cookie_pw))
      {
        status = 401;
        pwlog(b->peerip, b->uplink, "bad cookie");
        return;
      }
      else
        b->pwok = OK;
      safe_strcpy(pw, cookie_pw);
      safe_strcpy(login, cookie_login);
      nocookie = 1;
    }
    strupr(login);
    loaduser(login, u, 0);
    if (u->httpsurface == 0)
      httpsurface = m.defhttpsurface;
    else
      httpsurface = u->httpsurface - 1;
    sprintf(t->name, "/%s", login);
#ifdef FILES_WITHOUT_PW
  }
#endif
  b->charset = 1;
  //get_ttycharset(login) ..
  if (contentlength)
  {
    long l;
/* dh8ymb: wozu?
    if (contentlength > MAXMAILLEN)
    {
      trace(serious, "httpd", "content too long, ip [%s]", b->peerip);
      status = 500;
      return;
    }
*/
    content = (char *) t_malloc(contentlength + 1, "hcon");
    *content = getv(); //skip leading CR/LFs
              //(some?) browsers put two newlines before content
    while (*content == LF || *content == CR) *content = getv();
    for (l = 1; l < contentlength; l++) content[l] = getv();
    content[l] = 0;
  }
  else if (method == POST)
  {
    status = 400;
    return;
  }
  locuri = strchr(uri, '?');
  if (locuri)
  {
    if (! content)
    {
      *locuri = 0;
      content = locuri + 1;
    }
  }
//  form_referer();
}

/*---------------------------------------------------------------------------*/

void httpd::get_postarea (char *tag, char *result, int max, unsigned wrap)
//*************************************************************************
//
//*************************************************************************
{
  unsigned int i;
  *result = 0;
  char *firstresult = result;
  char *found;
  char hex[3];
  unsigned val;

  if (! content) return;
  found = stristr(content, tag);
  if (found)
  {
    found += strlen(tag);
    while (*found && *found != '&' && max)
    {
      if (*found == '+') *result = ' ';
      else if (*found == '%' && found[1] && found[2])
      {
        hex[0] = found[1];
        hex[1] = found[2];
        hex[2] = 0;
        sscanf(hex, "%2X", &val);
        found += 2;
        if (val == 0x0D)
        {
          found++;
          continue;
        }
        *result = val;
      }
      else *result = *found;
      result++;
      found++;
      max--;
      *result = 0;
    }
  }
  // do word wrapping since the browser seems not to be able to do it
  if (wrap)
  {
    result = firstresult;
    while (*result)
    {
      if (*result == LF)
      {
        result++;
        continue;
      }
      for (i = 0; i < wrap && result[i] && result[i] != LF; i++);
      if (i == wrap)
      {
        while (i && result[i] != ' ') i--;
        if (! i) i = wrap;
        else result[i] = LF;
      }
      result += i;
    }
  }
}

/*---------------------------------------------------------------------------*/

void httpd::put_header (char *title)
//*************************************************************************
//
// creates HTTP header, puts <HTML>...<BODY> if title!=NULL
//
//*************************************************************************
{
  struct
  { int status;
    char *phrase;
  }
  st_tab[] =
  {
    { 200, "OK" },
    { 201, "Created" },
    { 202, "Accepted" },
    { 204, "No Content" },
    { 301, "Moved Permanently" },
    { 302, "Moved Temporarily" },
    { 304, "Not Modified" },
    { 400, "Bad Request" },
    { 401, "Unauthorized" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
    { 500, "Internal Server Error" },
    { 501, "Not Implemented" },
    { 502, "Bad Gateway" },
    { 503, "Service Unavailable" },
    { 0, "Unknown" }
  };
  int st;
  int expsec;
  char *mimestr;
  int ht = b->http;
  FILE *wavfile = NULL;
  FILE *backfile = NULL;
  b->http = 1;

  for (st = 0; st_tab[st].status; st++)
  {
    if (st_tab[st].status == status) break;
  }
  html_putf("HTTP/1.0 %d %s\n", status, st_tab[st].phrase);
  html_putf("Date: %s\n", ht_time(ad_time()));
  html_putf("Server: " STD_BOXHEADER "/" VNUMMER "\n");
  if (status == 401)
  {
    if (strcmp(uri, "/logout"))
      html_putf("WWW-Authenticate: Basic realm=\"%s\"\n", m.boxname);
    html_putf("Set-Cookie: %s=; path=/; expires=%s;\n",
               m.boxname, ht_time(ad_time()) );
//   html_putf("Set-Cookie: %s=; path=/; expires=Tue, 01 Jan 1980 1:00 GMT;",
//              m.boxname);
//   html_putf("Cache-Control: max-age=0 Cache-Control: must-revalidate\n");
  }
  else if (status == 200 && ! strcmp(uri, "/") && *pw && ! nocookie)
       {
         html_putf("Set-Cookie: %s=%s; path=/; expires=%s;\n",
                   m.boxname, userpass, ht_time(ad_time() + MAXAGE));
//         html_putf("Set-Cookie: %s=%s; path=/; expires=%s;",
//                     m.boxname, userpass, ht_time(ad_time() + MAXAGE));
//         html_putf("Cache-Control: max-age=0\nCache-Control: must-revalidate\n");
       }
  if (mimetype == NONE && status == 200)
  {
    if (stristr(uri, ".jpg")) mimetype = JPG;
    if (stristr(uri, ".gif")) mimetype = GIF;
    if (stristr(uri, ".wav")) mimetype = WAV;
  }
  switch (mimetype)
  {
    case JPG: mimestr = MIME_JPG; break;
    case GIF: mimestr = MIME_GIF; break;
    case BIN: mimestr = MIME_BIN; break;
    case WAV: mimestr = MIME_WAV; break;
    default:  mimestr = MIME_TXT;
  }
  html_putf("Content-type: %s\n", mimestr);
    html_putf("Cache-Control: max-age=0\nCache-Control: must-revalidate\n");
  if (status == 200)
  {
    if (*file)
    {
      html_putf("Content-length: %ld\n", filesize(file));
      html_putf("Last-modified: %s\n", ht_time(file_isreg(file)));
    }
    else
    {
      expsec = 1800;
      if (stristr(uri, "/send") == uri) expsec = 8000; //OE3DZW was 7200
      html_putf("Expires: %s\n", ht_time(ad_time() + expsec));
    }
  }
  html_putf("\n");
  if (status != 200) //something is wrong ..
  {
    if (status != 401 && status != 404 && status != 503)
      trace(replog, "httpd", "strange request from [%s]", b->peerip);
    if (! strcmp(uri, "/logout"))
    {
      html_putf("<html><head><title>%s</title></head>\n", m.boxname);
      html_putf("<body><h1>%s</h1>\n", m.boxname);
    }
    else
    {
      html_putf("<html><head><title>%s</title></head>\n", st_tab[st].phrase);
      html_putf("<body><h1>%s (%d)</h1>\n", st_tab[st].phrase, status);
    }
    if (status == 503) // m.disable=1
    {
      html_putf(ms(m_maintenance), m.boxname);
      return;
    }
    if (! logout)
    {
      html_putf("<p>An error ocurred while processing your query.</p>\n");
#ifdef _GUEST
     if (strcmp(m.guestcall, "OFF"))
     {
       get_ttypw(m.guestcall, guestpw);
       if (strlen(guestpw) > 1)
       {
         html_putf("<p><b>You can log in as user '%s' with password '%s' for read-only access!</b><br>\n",
                    m.guestcall, guestpw);
         html_putf("<b>If you need full access contact sysop %s!</b></p>\n",
                    m.sysopcall);
       }
       else
       if (m.httpguestfirst)
         html_putf("HTTPD guest error: no ttypw set for guestcall - inform sysop!</p>\n");
     }
#endif
    }
    else
    {
      // leider behalten die Browser die Authentization-Info bis zum Browserneustart,
      // so dass der Browser ein neues Cookie anfordert wenn er nicht neugestartet wird!
      html_putf("<p> Your cookie has been removed... now close all browser windows to take effect!</p>\n");
    }
    html_putf("<b>" STD_BOXHEADER "</b> - Version " VNUMMER " - <i>httpd</i>\n");
    html_putf("</body></html>\n");
  }
  if (title)
  {
    html_putf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">");
    html_putf("<html><head>\n");
    if (m.httprobots)
    {
      html_putf("<meta name=\"ROBOTS\" content=\"INDEX,FOLLOW\">\n");
      html_putf("<meta name=\"DESCRIPTION\" content=\"PACKET RADIO MAILBOX %s\">\n",
                m.boxname);
    }
    else
      html_putf("<meta name=\"ROBOTS\" content=\"NOINDEX,NOFOLLOW\">\n");
    html_putf("<meta http-equiv=\"expires\" content=\"0\">\n");
    html_putf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n");
    html_putf("<title>%s - %s</title>", m.boxname, title);
    if (httpsurface == 1)
    {
      html_putf("<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\">");
      html_putf("<script type=\"text/javascript\">\n"
                "<!--\n"
                "function BlurLinks()\n"
                "{\n"
                " lnks=document.getElementsByTagName('a');\n"
                " for(i=0;i<lnks.length;i++)\n"
                " {\n"
                "   lnks[i].onfocus=new Function(\"if(this.blur)this.blur()\");\n"
                " }\n"
                "}\n"
                "onload=BlurLinks;\n"
                "-->\n"
                "</script>\n");
    }
    html_putf("</head>\n");
    if (httpsurface != 1)
    {
      backfile = s_fopen("http/back.jpg", "srt");
      if (backfile)
      {
        s_fclose(backfile);
        html_putf("<body BGCOLOR=" BGCOL " background=\"/back.jpg\">\n");
      }
      else
        html_putf("<body BGCOLOR=" BGCOL ">\n");
    }
    else
      html_putf("<body>\n");
    wavfile = s_fopen("http/qsl.wav", "srt");
    if (wavfile)
    {
      s_fclose(wavfile);
      html_putf("<EMBED src=\"qsl.wav\" autostart=true loop=false height=0 width=0 volume=100>\n");
    }
  }
  b->http = ht;
  return;
}

/*---------------------------------------------------------------------------*/

void httpd::put_sendform (char *to, char *lt, char *subj, char *read_)
//*************************************************************************
//
//*************************************************************************
{
  if (! to) to = "";
  if (! lt) lt = "";
  if (! subj) subj = "";
  html_putf("<h2><i>%s</i></h2>\n", ms(m_http_sendmail));
  html_putf("<pre><form method=post action=\"/sendok\">\n");
  if (! strcmp(to, m.sysopcall))
  {
    if (! m.httpshowsysopcall)
      html_putf("%20s: <input type=\"text\" name=\"to\" "
                "value=\"\" size=30><BR>", ms(m_http_to));
    else
      html_putf("%20s: %s <input type=\"hidden\" <name=\"to\" "
                "value=\"%s\" size=8><BR>", ms(m_http_to), to, to);
  }
  else
    html_putf("%20s: <input type=\"text\" name=\"to\" "
              "value=\"%s\" size=30><BR>", ms(m_http_to), to);
  html_putf("%20s: <input type=\"text\" name=\"life\" "
            "value=\"%s\" size=3><BR>", ms(m_http_lifetime), lt);
  html_putf("%20s: <input type=\"text\" name=\"subj\" "
            "value=\"%s\" size=70>\n", ms(m_http_subject), subj);
  html_putf("<br>");
  html_putf("<tt><textarea name=\"text\" rows=15"
            " cols=79 hscroll=\"no\" wrap=\"virtual\">\n");
  if (read_ && *read_)
    mbread(read_, 3);
  html_putf("</textarea></tt>\n");
  html_putf("<BR><input type=\"submit\" value=\"%s\">", ms(m_http_sendmail));
  html_putf("&nbsp;<input type=\"reset\" value=\"%s\">", ms(m_http_clearform));
//  html_putf("&nbsp;<input type=\"file\" name=\"attach\" value=\"\">");
  html_putf("\n</form></pre>\n");
}

/*---------------------------------------------------------------------------*/

void httpd::put_createlogin (char *createlogin, char *createpw) //dh8ymb
//*************************************************************************
//
//*************************************************************************
{
  if (! createlogin) createlogin = "";
  if (! createpw) createpw = "";
  html_putf("<h2><i>Create a new account</i></h2>");
  html_putf("<pre><form method=post action=\"/askok\">\n");
  html_putf("To create your own account, type in your logincallsign\n");
  html_putf("and choose a password (max. 8 characters). Then click CREATE.\n");
  html_putf("Note: This works only if a TTYPW password is not set for your callsign yet!\n");
  html_putf("If you want to change your TTYPW or you forgot the password,\n");
  html_putf("write a mail to sysop (%s) by clicking \"send sysop\"<BR><P>\n",
             m.sysopcall);
  html_putf("   Login: <input type=\"text\" name=\"createlogin\" "
              "value=\"%s\" size=8><BR>", createlogin);
  html_putf("Password: <input type=\"text\" name=\"createpw\" "
              "value=\"%s\" size=8>\n", createpw);
  html_putf("<BR><BR><BR><BR><input type=\"submit\" value=\"Create\">");
  html_putf("&nbsp;<input type=\"reset\" value=\"Reset\">\n");
  html_putf("</form></pre>\n");
}

/*---------------------------------------------------------------------------*/

void httpd::put_homepage (char *cmd)
//*************************************************************************
//
//*************************************************************************
{
  FILE *logofile = NULL;
  FILE *menuefile = NULL;
  char menue_line[LINELEN+1];
  char filename[20];

  if (! cmd) cmd = "";
  static char tar[] = " target=\"txt\"";

  strupr(login);
//  loaduser(login, u, 0);
  if (httpsurface == 0)
  {
    logofile = s_fopen("http/logo.gif", "srt");
    if (logofile)
    {
      s_fclose(logofile);
      html_putf("<IMG SRC=\"logo.gif\" BORDER=\"0\" ALT=\"Logo %s\" HEIGHT=\"25\"> ", m.boxname);
    }
    html_putf("<B><FONT SIZE=+2>%s - </FONT></B>", m.boxname);
    html_putf("<B><FONT SIZE=+2>" LOGINSIGN "</FONT></B>"
             "<FONT SIZE=0> [%s]</FONT>\n", m.boxheader);
    html_putf("<br><tt><b>Login: <FONT COLOR=\"#ff0000\">%s @ %s</FONT><FONT COLOR=\"#000000\"></FONT></b></tt>\n",
             u->call, u->mybbs);
  }
  if (httpsurface != 1)
  {
    html_putf("<form method=get action=\"/cmd\"%s>\n", tar);
    html_putf("Command: <input name=\"cmd\" value=\"%s\" size=40>", cmd);
    html_putf("  <input type=\"submit\" value=\"Execute\">\n");
    html_putf("</form>\n");
    if (httpsurface == 0)
      html_putf("<a href=\"/\" target=\"_parent\">home</a> |\n");
    else
      html_putf("<a href=\"/\">home</a> |\n");
    html_putf("<a href=\"/cmd?cmd=c+300\"%s>newest check</a> |\n", tar);
#ifdef _GUEST
    if (strcmp(m.guestcall, login)) //dh8ymb: Guests haben keine eigenen Mails
#endif
    {
      html_putf("<a href=\"/cmd?cmd=d+n\"%s>d n</a> |\n", tar);
      html_putf("<a href=\"/cmd?cmd=d+-20\"%s>newest own</a> |\n", tar);
    }
    html_putf("<a href=\"/cmd?cmd=d+b\"%s>boards</a> |\n", tar);
    html_putf("<a href=\"/cmd?cmd=h+index\"%s>help index</a> |\n",tar);
    html_putf("<a href=\"/cmd?cmd=log+-z\"%s>log</a> |\n", tar);
    html_putf("<a href=\"/cmd?cmd=ps+-ru\"%s>ps</a> |\n", tar);
#ifdef _GUEST //dh8ymb
    if (! strcmp(m.guestcall, login))
    {
      if (m.httpguestfirst)
      {
        html_putf("<a href=\"/userlogin\" target=_top>userlogin</a> |\n");
      }
      html_putf("<a href=\"/send\"%s>send sysop</a> |\n", tar);
      if (m.httpaccount && (isamprnet(b->peerip) == 0))
                         // 44.x und 127.x IP's nutzen den Namen als PW
        html_putf("<a href=\"/ask\"%s>create account</a> |\n", tar);
    }
    else
#endif
      html_putf("<a href=\"/send\"%s>send</a> |\n", tar);
    html_putf("<a href=\"/cmd?cmd=sl\"%s>slog</a> |\n", tar);
    html_putf("<a href=\"/cmd?cmd=st+f\"%s>status forward</a> |\n", tar);
    html_putf("<a href=\"/cmd?cmd=l+baybox+-30\"%s>bcm news</a> |\n", tar);
    html_putf("<a href=\"/cmd?cmd=u\"%s>users</a> |\n", tar);
    html_putf("<a href=\"/cmd?cmd=v\"%s>version</a> |\n", tar);
#ifdef _GUEST //dh8ymb
    if (! strcmp(m.guestcall, login))
    {
      html_putf("<a href=\"/logout\" target=_parent>remove cookie</a><br>\n");
    }
    else
#endif
    {
      html_putf("<a href=\"/logout\" target=_parent>remove cookie</a> |\n");
      html_putf("<a href=\"/cmd?cmd=ct\"%s>checktime</a><br>\n", tar);
    }
/*
#ifdef _WIN32
#include "winsock.h"
  char hostname[65];
  hostent *h = NULL;
  if (myhostname(hostname, 64) != -1)
  {
    hostname[65] = 0;
    h = gethostbyname(hostname);
    if (h != NULL)
    {
      html_putf("<BR><a href=ftp://%s@%s:%d>FTP (Filesurf)</a> |\n",
                 login, inet_ntoa(*(reinterpret_cast<in_addr*>(h->h_addr))), m.ftp_port);
      html_putf("<a href=nntp://%s:%d>NNTP-Access</a>\n",
                 inet_ntoa(*(reinterpret_cast<in_addr*>(h->h_addr))), m.nntp_port);
    }
    else
      trace(report, "put_homepage", "Error resolving own hostname");
  }
#endif
*/
  }
  if (httpsurface == 2)
    html_putf("<hr>\n");
  if (httpsurface == 1)
  {
    html_putf("<A NAME=\"start\"></A>");
    html_putf("<table bgcolor=\"#000000\" cellspacing=\"0\" cellpadding=\"0\" border=\"0\" width=\"100%%\">");
    html_putf("<tr><td width=\"100%%\"><table cellspacing=\"1\" cellpadding=\"0\" border=\"0\" width=\"100%%\">");
    html_putf("<tr><td align=\"right\" width=\"100%%\" colspan=\"3\" bgcolor=\"#646B84\" class=\"leiste\">" LOGINSIGN "</td></tr>");
    html_putf("<tr><td bgcolor=\"#ffffff\" valign=\"top\" width=\"140\" rowspan=\"3\">");
    html_putf("<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" width=\"140\">");
    logofile = s_fopen("http/logo.gif", "srt");
    if (logofile)
    {
      s_fclose(logofile);
      html_putf("<br><CENTER><IMG SRC=\"logo.gif\" alt=\"\" BORDER=\"0\" HEIGHT=\"25\"></CENTER>");
    }
    else
      html_putf("<br>");
    html_putf("<tr><td><CENTER><FONT SIZE=0>Packet Radio Mailbox</FONT></CENTER></td></tr>");
    html_putf("<tr><td><CENTER><H1>%s</H1></CENTER></td></tr>", m.boxname);
    html_putf("<tr><td><CENTER><FONT SIZE=0>[%s]</FONT></CENTER><br></td></tr>", m.boxheader);
    html_putf("<tr><td><CENTER><b>&nbsp;Login: <FONT COLOR=\"#ff0000\">%s</FONT><FONT COLOR=\"#000000\"></FONT></b></CENTER>", u->call);
    html_putf("<br><br></td></tr><tr>");
    html_putf("<td valign=\"top\" align=\"center\" width=\"100%%\">");
    html_putf("<table width=\"140\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" id=\"menu\" align=\"center\">");
    html_putf("<tr><td class=\"rubrik\"><b>&nbsp;%s</b></td></tr>", ms(m_http_commands));
    html_putf("<tr><td><form method=get action=\"/cmd\"><input name=\"cmd\" value=\"\" size=20></form></td></tr>");
    html_putf("<tr><td><a href=\"/\">&nbsp;%s</a></td></tr>", ms(m_http_home));
    html_putf("</table></td></tr></table>");
    html_putf("<table width=\"140\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" id=\"menu\" align=\"center\">");
    html_putf("<tr><td class=\"rubrik\"><b>&nbsp;%s</b></td></tr>", ms(m_http_user));
#ifdef _GUEST //dh8ymb
    if (! strcmp(m.guestcall, login))
    {
      if (m.httpguestfirst)
      {
        html_putf("<tr><td><a href=\"userlogin\">&nbsp;%s</a></td></tr>", ms(m_http_login));
      }
      if (m.httpaccount && (isamprnet(b->peerip) == 0))
                           // 44.x und 127.x IP's nutzen den Namen als PW
        html_putf("<tr><td><a href=\"ask\">&nbsp;%s</a></td></tr>", ms(m_http_createaccount));
    }
#endif
    html_putf("<tr><td><a href=\"logout\">&nbsp;%s</a></td></tr>", ms(m_http_removecookie));
    html_putf("</table>");
    html_putf("<table width=\"140\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" id=\"menu\" align=\"center\">");
    html_putf("<tr><td class=\"rubrik\"><b>&nbsp;%s</b></td></tr>", ms(m_http_mail));
#ifdef _GUEST
    if (strcmp(m.guestcall, login)) // GUEST hat keine eigenen Mails
#endif
    {
      html_putf("<tr><td><a href=\"cmd?cmd=d+-20\">&nbsp;%s</a></td></tr>", ms(m_http_listusermail));
    }
#ifdef _GUEST
    if (! strcmp(m.guestcall, login)) // GUEST darf nur dem Sysop mailen
    {
      html_putf("<tr><td><a href=\"send\">&nbsp;%s</a></td></tr>", ms(m_http_sendsysop));
    }
    else
#endif
    {
      html_putf("<tr><td><a href=\"send\">&nbsp;%s</a></td></tr>", ms(m_http_send));
    }
#ifdef _GUEST
    if (strcmp(m.guestcall, login)) // Checktime/DIR NEWS gilt nicht fuer GUEST
#endif
    {
      html_putf("<tr><td><a href=\"cmd?cmd=d+n\">&nbsp;%s</a></td></tr>", ms(m_http_check));
      html_putf("<tr><td><a href=\"cmd?cmd=ct\">&nbsp;%s</a></td></tr>", ms(m_http_setchecktime));
    }
    html_putf("<tr><td><a href=\"cmd?cmd=c+300\">&nbsp;%s</a></td></tr>", ms(m_http_newestmails));
    html_putf("<tr><td><a href=\"cmd?cmd=d+b\">&nbsp;%s</a></td></tr>", ms(m_http_boards));
    html_putf("<tr><td><a href=\"cmd?cmd=d+baybox+-30\">&nbsp;%s</a></td></tr>", ms(m_http_baybox));
    html_putf("</table>");
    html_putf("<table width=\"140\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" id=\"menu\" align=\"center\">");
    html_putf("<tr><td class=\"rubrik\"><b>&nbsp;%s</b></td></tr>", ms(m_http_system));
    html_putf("<tr><td><a href=\"cmd?cmd=log\">&nbsp;%s</a></td></tr>", ms(m_http_loginstats));
    html_putf("<tr><td><a href=\"cmd?cmd=slog\">&nbsp;%s</a></td></tr>", ms(m_http_logging));
    html_putf("<tr><td><a href=\"cmd?cmd=st+f\">&nbsp;%s</a></td></tr>", ms(m_http_sfstats));
    html_putf("<tr><td><a href=\"cmd?cmd=d+-n+u+m\">&nbsp;%s</a></td></tr>", ms(m_http_stuck));
    html_putf("<tr><td><a href=\"cmd?cmd=p+-sfn\">&nbsp;%s</a></td></tr>", ms(m_http_unknownbbs));
    html_putf("<tr><td><a href=\"cmd?cmd=ps\">&nbsp;%s</a></td></tr>", ms(m_http_ps));
    html_putf("<tr><td><a href=\"cmd?cmd=u\">&nbsp;%s</a></td></tr>", ms(m_http_u));
    html_putf("<tr><td><a href=\"cmd?cmd=v\">&nbsp;%s</a></td></tr>", ms(m_http_ver));
    html_putf("</table>");
    html_putf("<table width=\"140\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" id=\"menu\" align=\"center\">");
    html_putf("<tr><td class=\"rubrik\"><b>&nbsp;%s</b></td></tr>", ms(m_http_help));
    html_putf("<tr><td><a href=\"cmd?cmd=h+index\">&nbsp;%s</a></td></tr>", ms(m_http_helpindex));
    html_putf("<tr><td><a href=\"cmd?cmd=h+httpnewbie\">&nbsp;%s</a></td></tr>", ms(m_http_helpnewbies));
    strcpy(filename, "http/webmenu.");
    strcat(filename, ms(m_helpext));
    strlwr(filename);
    menuefile = s_fopen(filename, "lrt");
    if (! menuefile) menuefile = s_fopen("http/webmenu.gb", "lrt");
    if (! menuefile) menuefile = s_fopen("http/webmenu.dl", "lrt");
    if (menuefile)
    {
      html_putf("</table>");
      html_putf("<table width=\"140\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" id=\"menu\" align=\"center\">");
      while (fgets(menue_line, LINELEN, menuefile))
        html_putf("%s", menue_line);
      s_fclose(menuefile);
    }
    html_putf("</table></td>");
    html_putf("<td valign=\"middle\" bgcolor=\"#ffffff\" width=\"100%%\" height=\"50%%\"><br>");
    logofile = s_fopen("http/banner.gif", "srt");
    if (logofile)
    {
      s_fclose(logofile);
      html_putf("<center><img src=\"banner.gif\" height=\"60\" alt=\"\"></center><br>");
    }
    else
      html_putf("<br>");
    html_putf("</td></tr><tr></td></tr><tr>");
    html_putf("<td valign=\"top\" bgcolor=\"#e6e6e6\" width=\"100%%\" height=\"450\"><br>");
    html_putf("<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" width=\"100%%\"><tr>");
    html_putf("<td width=\"20\">&nbsp;&nbsp;</td>");
    html_putf("<td valign=\"top\" width=\"100%%\">");
  }
}

/*---------------------------------------------------------------------------*/

void httpd::put_css_footer (void)
//*************************************************************************
//
//*************************************************************************
{
  html_putf("<br></td><td width=\"20\"></td>");
  html_putf("</tr></table></td></tr>");
  html_putf("<tr><td bgcolor=\"#646B84\" class=\"leiste\">&nbsp;<span style=\"font-size:8pt\">%s</span></td>", datestr(ad_time(), 12));
  html_putf("<td align=\"center\" bgcolor=\"#646B84\" class=\"leiste\" style=\"font-size:8pt\">");
  html_putf("<a href=\"javascript:history.go(-1);\" onMouseOver=\"window.status='&nbsp;Zur&uuml;ck'; return true\" onMouseOut=\"window.status=''\">%s</a>&nbsp;", ms(m_http_goback));
  html_putf("<a href=\"#start\" onMouseOver=\"window.status='&nbsp;Nach oben'; return true\" onMouseOut=\"window.status=''\">%s</a>", ms(m_http_goup));
  html_putf("</td></tr></table></td></tr></table>");
}

/*---------------------------------------------------------------------------*/

void httpd::put_frame (void)
//*************************************************************************
//
//*************************************************************************
{
  FILE *backfile = NULL;

  html_putf("<html><head>\n<meta http-equiv=\"expires\" content=\"0\"><title>%s - " LOGINSIGN "\n",
              m.boxname);
  html_putf("</title><frameset rows=\"150,*\">\n");
  html_putf("<frame scrolling=no src=\"/command\" name=\"cmd\">\n");
  html_putf("<frame scrolling=auto src=\"/login\" name=\"txt\">\n");
  html_putf("</frameset></head>\n");
  backfile = s_fopen("http/back.jpg", "srt");
  if (backfile)
  {
    s_fclose(backfile);
    html_putf("<noframe><body BGCOLOR=" BGCOL " background=\"/back.jpg\">\n");
  }
  else
    html_putf("<noframe><body BGCOLOR=" BGCOL ">\n");
  html_putf("<h1>Please use a browser with frame capability</h1><p>\n");
  put_homepage("");
  html_putf("</body></noframe>\n");
  html_putf("</html>\n");
}

/*---------------------------------------------------------------------------*/


//void httpd::postmsg (char *in, char *out, int outlen)
//*************************************************************************
//
//*************************************************************************
/*{
  int n = 0;
  char *pin = in;
  out[0] = 0;
  while (in[0])
  {
    out[1] = 0;
    if (n+2 > outlen)
    {
      trace(serious, "postmsg", "too short %s (%d)", pin, outlen);
      out[n]=0;
      return;
    }
    switch(in[0])
    {
      case '#':
      case '<':
      case '>':
      case '+':
      case '&':
      case '%':
      case '?':
      case '"':
      case 10:
      case 13:
         sprintf(out, "%%%02X", (unsigned) in[0]);
         out+=2;
         n+=2;
         break;
      default:
        if (in[0] == ' ')
          out[0] = '+';
        else
          out[0] = in[0];
    }
    out++;
    in++;
    n++;
  }
}*/

/*---------------------------------------------------------------------------*/

void httpd::start_http (char *name)
//*************************************************************************
//
//*************************************************************************
{
  char sign1[80];
  FILE *f;
  char cmd[200];
  char *head;
  char to[LINELEN+1];
  char life[LINELEN+1];
  char subj[LINELEN+1];
  char createlogin[LINELEN+1];
  char createpw[LINELEN+1];
  char *oldpw;
  char read_cmd[30];
  char read_[256];
  char tmpname[30];
  int a;
  unsigned int i;

  strcpy(sign1, "STARTHTTP");
  b->http = 1;
  httpsurface = 0;
  *cookie_login = *cookie_pw = 0; //oe3dzw, Initialisierung
  generate_css();
  get_request();
  if (status == 200 && method != HEAD)
  {
    if (*file)
    {
      f = s_fopen(file, "lrb");
      put_header(NULL);
      if (f)
      {
        while ((a = fgetc(f)) != EOF) putv_sock(a);
        s_fclose(f);
      }
    }
    else
    {
      get_postarea("cmd=", cmd, 59);
      if (*cmd) head = cmd;
      else head = STD_BOXHEADER" V"VNUMMER;
      b->continous = 1;
      b->http = 2;
      if (httpsurface == 0)
      {
        if (! strcmp(uri, "/"))
        {
          put_header(NULL);
          put_frame();
          return;
        }
        else if (! stricmp(uri, "/command"))
        {
          put_header(head);
          put_homepage("");
          html_putf("</body></html>");
          return;
        }
      }
#ifdef DEBUG_HTTP
      trace(report, "httpd", "rx %d tx %d", b->rxbytes, b->txbytes);
#endif
      if (httpsurface == 0)
      {
        if (! stricmp(uri, "/login"))
        {
          put_header(head);
          html_putf("<pre>");
          mblogin(login, login_standard, "HTTP");
          html_putf("</pre>");
        }
      }
      else
      {
        if (stristr(uri, "/bread/") != uri)
        {
          put_header(head);
          if (httpsurface == 2)
          {
            html_putf("<h1>" LOGINSIGN "</h1>\n"
                      "<p><tt><b>Login: <FONT COLOR=\"#ff0000\">%s @ %s</FONT><FONT COLOR=\"#000000\"> [%s]</FONT></b></tt></p>\n",
                      login, m.boxadress, m.boxheader);
          }
          put_homepage("");
        }
        if (! strcmp(uri, "/"))
        {
          html_putf("<pre>");
          mblogin(login, login_standard, "HTTP");
          html_putf("</pre>");
        }
      }
      if ((! strcmp(uri, "/") && httpsurface != 0)
          || (! stricmp(uri, "/login") && httpsurface == 0))
      {}
      else if (! stricmp(uri, "/cmd"))
      {
        if (httpsurface == 0)
          put_header(head);
        html_putf("<pre>");
        mblogin(login, login_silent, "HTTP");
        mailbef(cmd, 0);
        html_putf("</pre>");
      }
      else if (stristr(uri, "/bread/") == uri)
      {
        get_postarea("read=", read_, sizeof(read_));
        for (i = 0; i != strlen(read_); i++)
          read_[i] = (read_[i] == ':') ? '/' : read_[i];
        if      (stristr(uri + 7, ".jpg")) mimetype = JPG;
        else if (stristr(uri + 7, ".gif")) mimetype = GIF;
        else if (stristr(uri + 7, ".wav")) mimetype = WAV;
        else mimetype = BIN;
        //  mimetype = TXT;
        if (httpsurface == 0)
          put_header(NULL);
        mblogin(login, login_silent, "HTTP");
        b->http = 1;
        mbread(read_, 4);
        return;
      }
      else if (! stricmp(uri, "/send"))
      {
        if (httpsurface == 0)
          put_header(head);
        mblogin(login, login_silent, "HTTP");
        get_postarea("to=", to, LINELEN);
        remove_emptyadd(to);
        get_postarea("life=", life, LINELEN);
        get_postarea("subj=", subj, LINELEN);
        get_postarea("read=", read_cmd, 29);
#ifdef _GUEST
        if (! strcmp(m.guestcall, login))
        {
          put_sendform(m.sysopcall, life, subj, read_cmd);
        }
        else
          put_sendform(to, life, subj, read_cmd);
#else
        put_sendform(to, life, subj, read_cmd);
#endif
        if (! *read_cmd)
        {
          get_postarea("era=", read_cmd, 29);
          if (*read_cmd) mbchange(read_cmd, w_erase, 0);
        }
      }
      else if (! stricmp(uri, "/ask"))
      {
        if (m.httpaccount)
        {
          if (httpsurface == 0)
            put_header(head);
          mblogin(login, login_silent, "HTTP");
          get_postarea("createlogin=", createlogin, LINELEN);
          get_postarea("createpw=", createpw, LINELEN);
          put_createlogin(createlogin, createpw);
        }
      }
      else if (! stricmp(uri, "/userlogin"))
      {
       if (m.httpguestfirst)
        {
         if (httpsurface == 0)
         {
           put_header(NULL);
           put_frame();
         }
         userlogin = 1;
         html_putf("<pre>");
         mblogin(login, login_standard, "HTTP");
         if (httpsurface == 0)
           html_putf("<meta http-equiv=\"refresh\" content=\"0\">");
         html_putf("</pre>");
        }
       }
#ifdef _GUEST
       else if (! stricmp(uri, "/guestlogin"))
       {
         if (httpsurface == 0)
         {
           put_header(NULL);
           put_frame();
         }
         guestlogin = 1;
         html_putf("<pre>");
         mblogin(login, login_standard, "HTTP");
         html_putf("</pre>");
/*  if (httpsurface == 0)
      put_header(head);
    html_putf("<pre>");
    html_putf("Not implemented yet\n");
    html_putf("</pre>");
*/
      }
#endif
      else if (! stricmp(uri, "/sendok"))
      {
        char *sendtext = (char *) t_malloc(contentlength + 1, "smsg");
 //     char *sendattach = (char *) t_malloc(contentlength + 1, "smsg");
        if (httpsurface == 0)
          put_header(head);
        html_putf("<pre>");
        mblogin(login, login_silent, "HTTP");
        get_postarea("text=", sendtext, contentlength, LINELEN-1);
#ifdef DEBUG_HTTP
        trace(report, "httpd", "debug: contentlength %ld len %ld",
                      contentlength, strlen(sendtext));
#endif
 //       get_postarea("attach=", sendattach, contentlength, LINELEN);
#ifdef _GUEST
        if (! strcmp(m.guestcall, login))
        {
          if (! m.httpshowsysopcall)
            get_postarea("to=", to, LINELEN);
          else
            strcpy(to, m.sysopcall);
        }
        else
          get_postarea("to=", to, LINELEN);
#else
        get_postarea("to=", to, LINELEN);
#endif
        get_postarea("life=", life, LINELEN);
        cut_blanktab(life);
        get_postarea("subj=", subj, LINELEN);
        if (m.ltquery > 0 && ! *life)
          html_putf("No, lifetime is missing!<BR>");
        else
        {
          sprintf(tmpname, "%s/%s.imp", TEMPPATH, time2filename(0));
          strlwr(tmpname);
          if ((f = s_fopen(tmpname, "swt")) != NULL)
          //oe3dzw: nnnn at start of line
          {
            s_fsetopt(f, 1);
            fprintf(f, "%s \nnnnn\nend\n", sendtext);
            s_fclose(f);
            t_free(sendtext);
#ifdef DEBUG_HTTP
            trace(report, "httpd", "debug: sysimport %s", to);
#endif
            sysimport(tmpname);
            if (*life)
              sprintf(cmd, "s %s #%s %s", to, life, subj);
            else
              sprintf(cmd, "s %s %s", to, subj);
            mailbef(cmd, 0);
#ifdef DEBUG_HTTP
            trace(report, "httpd", "debug: mail sent %s", to);
#endif
          }
          else putf(ms(m_filenotopen), strerror(errno));
          unlink(tmpname);
        }
        html_putf("</pre>");
      }
      else if (! stricmp(uri, "/askok"))
      {
        html_putf("<pre>");
        if (m.httpaccount)
        {
          if (httpsurface == 0)
            put_header(head);
          get_postarea("createlogin=", createlogin, LINELEN);
          get_postarea("createpw=", createpw, LINELEN);
          if (mbcallok(createlogin))
          {
            if (strlen(createpw) > 1)
            {
              mblogin(createlogin, login_silent, "HTTP");
              oldpw = get_ttypw(createlogin, "");
              if (! *oldpw)
              {
                if (strlen(createpw) < 9)
                {
                  if (put_ttypw(createlogin, createpw))
                  {
                    html_putf("<BR><P><B>Your account is created successful!</B><BR><P>\n");
                    html_putf("Don't forget the following data:<BR><P>\n");
                    html_putf("<B>Login:</B> %s</b><BR>\n", createlogin);
                    html_putf("<B>Password:</B> %s</b><BR>\n", createpw);
                  }
                  else
                    html_putf("<BR><P><B>Error occured!</B><BR><P>\n");
                }
                else
                  html_putf("<BR><B>Password too long!</B><P>Try again...\n");
              }
              else
              {
                html_putf("<BR><B>Account for %s has been already created!</B><BR>\n",
                     createlogin);
                html_putf("If you only want to change your password, write a mail to sysop (%s) by clicking \"send sysop\" <BR>\n",
                     m.sysopcall);
              }
            }
            else
              html_putf("<BR><B>Enter a password!</B><P>Try again...<BR>\n");
          }
          else
            html_putf("<BR><B>Invalid callsign:</B> %s <P>Try again...<BR>\n",
                      createlogin);
        }
        html_putf("</pre>");
      }
      else status = 404;
      if (status == 200)
      {
        if (httpsurface == 1)
          put_css_footer();
        html_putf("</body></html>");
      }
      else put_header(NULL);
    }
  }
  else put_header(NULL);
  if (strcmp(sign1, "STARTHTTP"))
    trace(fatal, "httpd", "sig starthttp broken");
}

/*---------------------------------------------------------------------------*/

void html_putf (char *format, ...)
//*************************************************************************
//
// Gibt Zeichen als HTML-Code aus
//
//*************************************************************************
{
  va_list argpoint;
  char cbuf[260];
  char *s = cbuf;
  int ht = b->http;

  if (! ht) return;
  va_start(argpoint, format);
#ifdef __LINUX__
  int ret = vsnprintf(cbuf, sizeof(cbuf), format, argpoint);
#else
  int ret = _vsnprintf(cbuf, sizeof(cbuf) - 1, format, argpoint);
#endif
  va_end(argpoint);
  if ((m.tcpiptrace == 1) || (m.tcpiptrace == 8)) httplog("TX", s);
  if (ret == -1)
  {
    s[50] = 0;
    trace(fatal, "html_putf", "long %s", s);
  }
  b->http = 1;
  while (*s) putv(*(s++));
  b->http = ht;
}

/*---------------------------------------------------------------------------*/

void mblogin_http (char *name)
//*************************************************************************
//
//*************************************************************************
{
  httpd ht;
//oe3dzw
  strcpy(b->uplink, "HTTP");
  strcpy(b->peerip, name);
  *b->logincall = 0;
//  b->httpd = (void*) &ht;
  ht.start_http(name);
  putflush_sock();
  disconnect_sock();
}

/*---------------------------------------------------------------------------*/
