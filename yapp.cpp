/***************************************************************

  BayCom(R)        Packet-Radio fuer IBM PC

  OpenBCM-Mailbox

  -------------
  YAPP-Protocol
  -------------


  Copyright (C)       Johann Hanne, DH3MB, jonny@baycom.org
                      Falkenweg 6
                      D-85122 Hitzhofen

  Alle Rechte vorbehalten / All Rights Reserved

 ***************************************************************/

//19980311 OE3DZW removed path from yapp-filename
//19989313 DG9MHZ optimized by deti
//19990814 DH3MB  Fixed some stupid bugs
//19990815 DH3MB  Some improvements
//20020524 DK2UI  added setfiletime


#include "baycom.h"

#ifdef FEATURE_YAPP

/*---------------------------------------------------------------------------*/

fileio_yapp::fileio_yapp (void) : fileio ()
{
  yappc = 0;
  f->requires = 0;
}

/*---------------------------------------------------------------------------*/

void fileio_yapp::tx_RR (void)
{
  bputv(ACK);
  bputv(0x01);
  putflush();
}

/*---------------------------------------------------------------------------*/

void fileio_yapp::tx_RF (void)
{
  bputv(ACK);
  bputv(0x02);
  putflush();
}

/*---------------------------------------------------------------------------*/

void fileio_yapp::tx_RT (void)
{
  bputv(ACK);
  bputv(ACK);
  putflush();
}

/*---------------------------------------------------------------------------*/

void fileio_yapp::tx_AF (void)
{
  bputv(ACK);
  bputv(0x03);
  putflush();
}

/*---------------------------------------------------------------------------*/

void fileio_yapp::tx_AT (void)
{
  bputv(ACK);
  bputv(0x04);
  putflush();
}

/*---------------------------------------------------------------------------*/

void fileio_yapp::tx_RE (long int length, char yappch)
{
  char buffer[13];
  char len;

  len = sprintf(buffer, "%ld", length) + 5;
  bputv(NAK);
  bputv(len);
  bputv('R');
  bputv(0);
  putf("%s", buffer);
  bputv(0);
  if (yappch)
  {
    bputv('C');
    bputv(0);
  }
  putflush();
}

/*---------------------------------------------------------------------------*/

void fileio_yapp::tx_SI (void)
{
  bputv(ENQ);
  bputv(0x01);
  putflush();
}

/*---------------------------------------------------------------------------*/

void fileio_yapp::tx_CN (char *reason)
{
  int length;

  length = strlen(reason);
  bputv(CTRLX);
  bputv(length);
  putf("%s", reason);
  putflush();
}

/*---------------------------------------------------------------------------*/

void fileio_yapp::tx_CA (void)
{
  bputv(ACK);
  bputv(0x05);
  putflush();
}

/*---------------------------------------------------------------------------*/

void fileio_yapp::tx_HD (void)
{
  char size_buffer[13];
  int len;

  sprintf(size_buffer, "%ld", f->size);
  len = strlen(f->name) + strlen(size_buffer) + 11;
  bputv(SOH);
  bputv(len);
  putf("%s", f->name);
  bputv(0);
  putf("%s", size_buffer);
  bputv(0);
  putf("%08lX", f->dostime);
  bputv(0);
  putflush();
}

/*---------------------------------------------------------------------------*/

void fileio_yapp::tx_ET (void)
{
  bputv(EOT);
  bputv(0x01);
  putflush();
}

/*---------------------------------------------------------------------------*/

void fileio_yapp::tx_DT (int length)
{
  if (length == 256) length = 0;
  bputv(STX);
  bputv((char) length);
}

/*---------------------------------------------------------------------------*/

void fileio_yapp::tx_EF (void)
{
  bputv(ETX);
  bputv(0x01);
  putflush();
}

/*---------------------------------------------------------------------------*/

void fileio_yapp::wait_CA (void)
{
  char c1 = 0, c2 = 0, quit = 0;

  while (! quit)
  {
    c1 = getv();
    if (c2 == ACK && c1 == 0x05) quit = 1;
    c2 = getv();
    if (c1 == ACK && c2 == 0x05) quit = 1;
  }
  putflush();
}

/*---------------------------------------------------------------------------*/

yapp_packet_t fileio_yapp::YAPP_packet (char *cmd)
{
  if (cmd[0] == ACK && cmd[1] == 0x01) return YAPP_PACKET_RR;
  if (cmd[0] == ACK && cmd[1] == 0x02) return YAPP_PACKET_RF;
  if (cmd[0] == ACK && cmd[1] == ACK) return YAPP_PACKET_RT;
  if (cmd[0] == ACK && cmd[1] == 0x03) return YAPP_PACKET_AF;
  if (cmd[0] == ACK && cmd[1] == 0x04) return YAPP_PACKET_AT;
  if (cmd[0] == ENQ && cmd[1] == 0x01) return YAPP_PACKET_SI;
  if (cmd[0] == SOH) return YAPP_PACKET_HD;
  if (cmd[0] == STX) return YAPP_PACKET_DT;
  if (cmd[0] == ETX && cmd[1] == 0x01) return YAPP_PACKET_EF;
  if (cmd[0] == EOT && cmd[1] == 0x01) return YAPP_PACKET_ET;
  if (cmd[0] == NAK) return YAPP_PACKET_NR;
  if (cmd[0] == CTRLX) return YAPP_PACKET_CN;
  if (cmd[0] == ACK && cmd[1] == 0x05) return YAPP_PACKET_CA;
  return YAPP_PACKET_UNKNOWN;
}

/*---------------------------------------------------------------------------*/

void fileio_yapp::YAPP_unk (char *ystate, char *cmd)
{
  char buf[100];

  sprintf(buf, "\nUnexpected YAPP-sequence 0x%02x 0x%02x in ystate %s.\n",
              cmd[0], cmd[1], ystate);
  tx_CN(buf);
}

/*---------------------------------------------------------------------------*/

void fileio_yapp::_rx (void)
{
  unsigned short int length, i;
  char quit = 0;
  char cmd[2];
  char databuf[256];
  char expectedsize[13];
  char datetime[9];
  unsigned long int bps;
  time_t startt = ad_time(), rxtime;
  unsigned long dostime;
  char checksum, rx_checksum;

  if (f->exists) putf(ms(m_yapp_fileexists));
  putf(ms(m_yapp_waiting), f->fullname);
  putflush();
  state = YAPP_STATE_R;
  while (! quit)
  {
    cmd[0] = getv();
    cmd[1] = getv();
    packettype = YAPP_packet(cmd);
    switch (state)
    {
      case YAPP_STATE_R:        // Reception (Initialization)
        switch (packettype)
        {
          case YAPP_PACKET_SI:
            tx_RR();
            state = YAPP_STATE_RH;
            break;
          case YAPP_PACKET_NR:
            quit = 1;
            putf("\n");
            putf(ms(m_yapp_abortrx));
            putf(ms(m_yapp_nr));
            putf("\n");
            break;
          default:
            YAPP_unk("R", cmd);
            quit = 1;
            break;
        }
        break;
      case YAPP_STATE_RH:       // Receiving header
        switch (packettype)
        {
          case YAPP_PACKET_HD:
            // Throw away the file name
            while (getv());
            // Get the file size
            i = 0;
            while (((expectedsize[i] = getv()) != 0) && (i < 12)) i++;
            expectedsize[i + 1] = 0;
            // If we receive a date/time field -> YAPP C
            if (getvalid())
            { // Read the date/time field
              i = 0;
              while (((datetime[i] = getv()) != 0) && (i < 8)) i++;
              datetime[i + 1] = 0;
              yappc = 1;
              sscanf(datetime, "%lX", &dostime);
            }
            if (f->exists) // File exists -> Resume mode
            {
              f->ptr = s_fopen(f->fullname, "lab");
// TODO
              tx_RE(f->size, yappc);
            }
            else
            {
              f->ptr = s_fopen(f->fullname, "lwb");
// TODO
              if (yappc)
                tx_RT();
              else
                tx_RF();
            }
            state = YAPP_STATE_RD;
            break;
          case YAPP_PACKET_SI:
            // Einfach gar nichts tun... Nicht sinnvoll,
            // steht aber in der YAPP-Spezifikation
            break;
          case YAPP_PACKET_ET: // Transfer complete
            tx_AT();
//            putf("\n*** YAPP reception complete\n");
            rxtime = ad_time() - startt;
            if (! rxtime) rxtime = 1L;
            bps = filesize(f->fullname) / rxtime;
            putf("\n");
            putf(ms(m_rxok), "YAPP", bps << 3);
            quit = 1;
            break;
          default:
            YAPP_unk("RH", cmd);
            quit = 1;
            break;
        }
        break;
      case YAPP_STATE_RD:       // Receiving data
        switch (packettype)
        {
          case YAPP_PACKET_DT:
            length = (unsigned int) cmd[1];
            if (! length) length = 256;
            // Receive a data block and calculate the checksum
            checksum = 0;
            for (i = 0; i < length; i++)
            {
              databuf[i] = getv();
              checksum += (char) databuf[i];
            }
            if (yappc) rx_checksum = getv();
            // Check the the checksum
            if (yappc && (rx_checksum != checksum))
            {
              tx_CN("Checksum error");
              wait_CA();
            }
            else fwrite(databuf, length, 1, f->ptr); // Write the data block
            waitfor(e_ticsfull);
            break;
          case YAPP_PACKET_EF: // End of file
            s_fclose(f->ptr);
            tx_AF();
            state = YAPP_STATE_RH;
            setfiletime(f->fullname, dostime);
            break;
          case YAPP_PACKET_CN: // Cancel
            s_fclose(f->ptr);
            tx_CA();
            putflush();
            putf("\n");
            putf(ms(m_yapp_abortrx));
            while (cmd[1]--) bputv(getv());
            putv(LF);
            quit = 1;
            break;
          default:
            s_fclose(f->ptr);
            YAPP_unk("RD", cmd);
            quit = 1;
            break;
        }
        break;
          case YAPP_STATE_S:
          case YAPP_STATE_SH:
          case YAPP_STATE_ST:
          case YAPP_STATE_SD:
          case YAPP_STATE_SE:
          break;
    }
  }
  putflush();
}

/*---------------------------------------------------------------------------*/

void fileio_yapp::_tx(void)
{
  char quit = 0;
  char ack[2];
  char buf[256];
  unsigned long int bps;
  time_t startt, txtime;
  int length = 0;
  int c = 0;
  int i;
  char checksum = 0;
  off_t offset = 0; // Offset when resuming

  state = YAPP_STATE_S;
  startt = ad_time();
  while (! quit)
  {
    switch (state)
    {
      case YAPP_STATE_S:        // Transmission (Initialization)
        tx_SI();
        ack[0] = getv();
        ack[1] = getv();
        packettype = YAPP_packet(ack);
        switch (packettype)
        {
          case YAPP_PACKET_RR: state = YAPP_STATE_SH; break;
          case YAPP_PACKET_RF: state = YAPP_STATE_SD; break;
          case YAPP_PACKET_NR:
            putf("\n");
            putf(ms(m_yapp_aborttx));
            putf(ms(m_yapp_nr));
            putf("\n");
            quit = 1;
            break;
          default: YAPP_unk("S", ack); quit = 1; break;
        }
        break;
      case YAPP_STATE_SH:       // Sending header
        tx_HD();
        ack[0] = getv();
        ack[1] = getv();
        packettype = YAPP_packet(ack);
        switch (packettype)
        {
          case YAPP_PACKET_RT: yappc = 1; state = YAPP_STATE_SD; break;
          case YAPP_PACKET_RF: state = YAPP_STATE_SD; break;
          case YAPP_PACKET_NR:
            // NAK [len] ist ambigous :-(
            // this might be "Resume" or "Not Ready"
            for (i = 0; i < ack[1]; i++) buf[i] = getv();
            buf[i] = 0;
            if (buf[0] == 'R' && ! buf[1] && i > 2)
            {
              offset = atol (&buf[2]);
              if (! buf[i-1] && buf[i-2] == 'C' && ! buf[i - 3]) yappc = 1;
              state = YAPP_STATE_SD;
            }
            else
            {
              putf("\n");
              putf(ms(m_yapp_aborttx));
              putf(ms(m_yapp_nr));
              putf("\n");
              quit = 1;
            }
            break;
          default: YAPP_unk("SH", ack); quit = 1; break;
        }
        break;
      case YAPP_STATE_SD:       // Sending data
        f->ptr = s_fopen(f->fullname, "lrb");
        if (f->ptr && f->eraseflag) s_fsetopt(f->ptr, 2); // delete file after transmission
        fseek(f->ptr, offset + f->offset, SEEK_SET);
        do
        {
          c = fgetc(f->ptr);
          if (c != EOF)
          {
            buf[length++] = (char) c;
            checksum += (char) c;
          }
          if (length == 256 || (c == EOF && length))
          {
            tx_DT(length);
            for (i = 0; i < length; i++) bputv(buf[i]); // Send the data block
            if (yappc) bputv(checksum);
            length = 0;
            checksum = 0;
            waitfor(e_ticsfull);
          }
          if (getvalid()) // Huh? Received something while sending file!?
          {
            ack[0] = getv();
            ack[1] = getv();
            packettype = YAPP_packet(ack);
            if (packettype == YAPP_PACKET_CN)
            {
              tx_CA();
              putflush();
              putf("\n");
              putf(ms(m_yapp_aborttx));
              while (ack[1]--) bputv(getv());
              putv(LF);
            }
            else
            {
              putf("\n");
              putf(ms(m_yapp_aborttx));
              putf(ms(m_yapp_cn));
              putf("\n");
            }
            s_fclose(f->ptr);
            quit = 1;
            break;
          }
        }
        while (c != EOF);
        state = YAPP_STATE_SE;
        break;
      case YAPP_STATE_SE:       // Sending EOF (End of File)
        s_fclose(f->ptr);
        tx_EF();
        ack[0] = getv();
        ack[1] = getv();
        packettype = YAPP_packet(ack);
        switch (packettype)
        {
          case YAPP_PACKET_AF: state = YAPP_STATE_ST; break;
          default: YAPP_unk("SE", ack); quit = 1;
        }
        break;
      case YAPP_STATE_ST:       // Sending EOT (End of Transfer)
        tx_ET();
        ack[0] = getv();
        ack[1] = getv();
        packettype = YAPP_packet(ack);
        if (packettype != YAPP_PACKET_AT)
        {
          putf("\n");
          putf(ms(m_yapp_noat));
          putf("\n");
        }
        txtime = ad_time() - startt;
        if (! txtime) txtime = 1L;
        bps = filesize(f->fullname) / txtime;
        putf("\n");
        putf(ms(m_txok), "YAPP", f->crc, f->crc, bps << 3);
        quit = 1;
        break;
        case YAPP_STATE_R:
        case YAPP_STATE_RH:
        case YAPP_STATE_RD:
        break;
    }
  }
  putflush();
}

/*---------------------------------------------------------------------------*/
#endif
