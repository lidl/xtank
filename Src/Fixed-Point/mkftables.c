/*
** mkftable.c
**
** To make the ftables.h file:
**
**    cc -o mkftables mkftables.c -lm
**    mkftables > ftables.h
*/

#include "fixed.h"
#include "math.h"

#define PI 3.14159265358979323

main() {
  int i;

  puts("/*\n** ftable.h\n**\n** Tables for fixed point arithmetic.\n*/\n");

  /* Make 256 entry sine table for angles from 0 to PI/2 */
  puts("Fixed fixed_sin[256] = {");
  for(i = 0 ; i < 256 ; i++) {
    printf("%d,",float2f(sin((double) i * (PI/2) / 256)));
    if((i&7) == 7) putchar('\n');
  }
  puts("};\n");

  /* Make 257 entry atan table for fixed point numbers from 0 to 1 */
  puts("Fixed fixed_atan[257] = {");
  for(i = 0 ; i < 257 ; i++) {
    printf("%d,",angle2f(atan((double) i / 256)));
    if((i%6) == 5) putchar('\n');
  }
  puts("\n};");
}
