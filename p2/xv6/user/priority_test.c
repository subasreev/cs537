#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int
main (int argc, char *argv[])
{
  int i;
  int x = 0;
  if (argc != 2)
    exit ();
  setpri (2);

  for (i = 1; i < atoi (argv[1]); i++)
    {
    if(i== 1000000)
      setpri(1);
      x++;
    }
  exit ();
}
