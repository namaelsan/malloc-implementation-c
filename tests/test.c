/*test.c sonradan eklenecek sizin bir sey yapmaniz gerekmiyor*/
#include "../tests/mymalloc.h"
#define studentmain main
#include "../tests/mymalloc.c"
#undef studentmain
int main(){
  return 0;
}
