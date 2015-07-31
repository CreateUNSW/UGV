#include "shared.hpp"


unsigned char hexToNum( char in )
{
  switch (in)
  {
    case '0':  return 0; break;
    case '1':  return 1; break;
    case '2':  return 2; break;
    case '3':  return 3; break;
    case '4':  return 4; break;
    case '5':  return 5; break;
    case '6':  return 6; break;
    case '7':  return 7; break;
    case '8':  return 8; break;
    case '9':  return 9; break;
    case 'A': return 10; break;
    case 'B': return 11; break;
    case 'C': return 12; break;
    case 'D': return 13; break;
    case 'E': return 14; break;
    case 'F': return 15; break;
    default: return 0; break; // Error
  }
}

char decToHex( unsigned char in )
{
  switch (in)
  {
    case 0:  return '0'; break;
    case 1:  return '1'; break;
    case 2:  return '2'; break;
    case 3:  return '3'; break;
    case 4:  return '4'; break;
    case 5:  return '5'; break;
    case 6:  return '6'; break;
    case 7:  return '7'; break;
    case 8:  return '8'; break;
    case 9:  return '9'; break;
    case 10: return 'A'; break;
    case 11: return 'B'; break;
    case 12: return 'C'; break;
    case 13: return 'D'; break;
    case 14: return 'E'; break;
    case 15: return 'F'; break;
    default: return 'z'; break; // Error
  }
}

unsigned char calculateChecksum( char * message, int len)
{
  unsigned char primes[] = {  2,   3,   5,   7,  11,  13,  17,  19,  23,  29,
                             31,  37,  41,  43,  47,  53,  59,  61,  67,  71,
                             73,  79,  83,  89,  97, 101, 103, 107, 109, 113,
                            127, 131, 137, 139, 149, 151, 157, 163, 167, 173 };
  int checksum = 0;
  for (unsigned char i=0; i < len; i++ )
  {
    if ( message[i] == ':' ) break; // Seperates message body from checksum
    checksum += message[i] * primes[i];
  }
  return checksum % 256;
}
