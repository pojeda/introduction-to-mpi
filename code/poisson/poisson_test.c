#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "poisson_step.c"

static void test_poisson_step(void **state) {
   float u[MAX+2][MAX+2], unew[MAX+2][MAX+2], rho[MAX+2][MAX+2];
   float h, hsq;
   double unorm, residual;

   /* Set variables */
   h = 0.1;
   hsq = h*h;

   // Initialise the u and rho field to 0 
   for( int j=0; j <= MAX+1; j++ ){
      for( int i=0; i <= MAX+1; i++ ) {
         u[i][j] = 0.0;
         rho[i][j] = 0.0;
      }
   }

   // Test a configuration with u=10 at x=1 and y=1
   u[1][1] = 10;

   unorm = poisson_step( u, unew, rho, hsq );

   assert_true( unorm == 112.5 );
}

/* In the main function create the list of the tests */
int main(void) {
   const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_poisson_step),
   };

   // Call a library function that will run the tests
   return cmocka_run_group_tests(tests, NULL, NULL);
}