/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBayCom-Mailbox

  --------------------------
  Morse Daemon, Header Files
  --------------------------



  Copyright (C)       Dietmar Zlabinger
                      Reinberg-Heidenreichstein 31
                      A-3861 Eggern

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/


#define DOTFREQ  820
#define DASHFREQ 800
#define DOTDUR   m.cwms

#define DASHDUR   DOTDUR*3
#define INTRASPACING DOTDUR
#define CHARSPACING  DOTDUR*3
#define WORDSPACING  DOTDUR*7


static char cs[]=
{
'a', 'Ñ', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
'm', 'n', 'o', 'î', 'p', 'q', 'r', 's', 't', 'u', 'Å', 'v',
'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9',
'0', '.', ',', ':', '?', '\'', '-', '/', '(', ')', '"', '_', ';',
'$','#', (char)NULL
};
static char mos[][8]=
{
".-",   //abc..
".-.-",
"-...",
"-.-.",
"-..",
".",
"..-.",
"--.",
"....",
"..",
".---",
"-.-",
".-..",
"--",
"-.",
"---",
"---.",
".--.",
"--.-",
".-.",
"...",
"-",
"..-",
"..--",
"...-",
".--",
"-..-",
"-.--",
"--..",
".----", //123
"..---",
"...--",
"....-",
".....",
"-....",
"--...",
"---..",
"----.",
"-----",
".-.-.-", //specials
"--..--",
"---...",
"..--..",
".----.",
"-....-",
"-..-.",
"-.--.",
"-.--.-",
".-..-.",
"..--.-",
"-.-.-.",
"...-..-",
"----",  // "ch"
""};

