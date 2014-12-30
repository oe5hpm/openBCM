/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -------------------------
  Charset conversion tables
  -------------------------

 ***************************************************************/

#include "baycom.h"

#define CHARSET_NUM 1

charset **charsets;

/*---------------------------------------------------------------------------*/

void init_charsets () //should be read from file and alloc'd dynamically
//*************************************************************************
//
// Init Charsets
//
//*************************************************************************
{
//IBM-ANSI umlaut
  charset *ibm_ansi = (charset *) t_malloc(sizeof(charset), "*chr");
  static char ibm_ansi_name[] = "ibm-ansi";
  static charset_entry ibm_ansi_chars[] =
  {
    {0x84, 0xe4}, //ae
    {0x94, 0xf6}, //oe
    {0x81, 0xfc}, //ue
    {0x8e, 0xc4}, //Ae
    {0x99, 0xd6}, //Oe
    {0x9a, 0xdc}, //Ue
    {0xe1, 0xdf}, //ss
    {0,0}
  };
  ibm_ansi->id = 1;
  ibm_ansi->name = ibm_ansi_name;
  ibm_ansi->chars = ibm_ansi_chars;
  charsets = (charset **) t_malloc(CHARSET_NUM * sizeof(charset *), "*chs");
  charsets[0] = ibm_ansi;
};

/*---------------------------------------------------------------------------*/
