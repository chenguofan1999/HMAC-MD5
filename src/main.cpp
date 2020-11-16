#include <iostream>
#include "md5.h"
using namespace std;
int main()
{
  MD5 md5 ;
  char *msg = "ABCDE";
  puts( md5.digestString( msg ) ) ;

  // print the digest for a binary file on disk.
  //puts( md5.digestFile( "jdfgsdhfsdfsd 156445dsfsd7fg/*/+bfjsdgf%$^" ) ) ;

  return 0;
}