/*
 * simple copy of metismex.c to try and see if it's the matlab runtimes
 * 
 * 
 * Matlab compile files
 * 
 * gcc-4.2 -c  -I../include -I../libmetis -I../GKlib/trunk -I/opt/matlab/R2010a/extern/include -DMATLAB_MEX_FILE -ansi -D_GNU_SOURCE  -fexceptions -fPIC -fno-omit-frame-pointer -pthread  -DLINUX -DUNIX -g  "metismex.c"
 * gcc-4.2 -g -pthread -shared -Wl,--version-script,/opt/matlab/R2010a/extern/lib/glnxa64/mexFunction.map -Wl,--no-undefined -o  "metismex.mexa64"  metismex.o  -L../build/Linux-x86_64/ -lmetis -Wl,-rpath-link,/opt/matlab/R2010a/bin/glnxa64 -L/opt/matlab/R2010a/bin/glnxa64 -lmx -lmex -lmat -lm -lstdc++
 * 
 * Adapted compile runs
 * gcc-4.2 -o simple_test  -I../include -I../libmetis -I../GKlib/trunk  -D_GNU_SOURCE  -fexceptions -fPIC -fno-omit-frame-pointer -pthread  -DLINUX -DUNIX -g  "simple_test.c" -lstdc++  -L../build/Linux-x86_64/  -lmetis 
 * 
 * Okay, that test ALSO shows an error, let's prune it back until it doesn't show an error.
 * gcc-4.2 -o simple_test  -I../include -I../libmetis -I../GKlib/trunk -g  "simple_test.c" -L../build/Linux-x86_64/ -lm -lmetis; ./simple_test
 * 
 * gcc-4.2 -DUNIX -pedantic -std=c99 -fPIC  -DLINUX -D_FILE_OFFSET_BITS=64  -I../include -I../GKlib/trunk -g  "simple_test.c" -L../build/Linux-x86_64/ -lmetis -lm -o simple_test ; ./simple_test 
 * 
 * gcc-4.2 -DUNIX -pedantic -std=c99 -fPIC  -DLINUX -D_FILE_OFFSET_BITS=64  -g  -g -I./ -I../include -I../GKlib/trunk  -I../libmetis  simple_test.c -L../build/Linux-x86_64/ -lmetis -lm -o simple_test; ./simple_test
 * 
 * AHHHHH It was the stupid IDXTYPEWIDTH variable... we need to change it to 64
 */



#include <strings.h>

//#define IDXTYPEWIDTH 64

//#include <GKlib.h>
#include <metis.h>
#include <metislib.h>
//#include <defs.h>
//#include <struct.h>
//#include <macros.h>
//#include <rename.h>
#include <proto.h>


typedef size_t mwIndex;
typedef size_t mwSize;

int main() {
    idxtype i, n, nparts, wgtflag, options[10] = {0}, numflag = 0, edgecut, sepsize;
    idxtype *part, *perm, *iperm, *sep;
    double *optarray, *partpr, *permpr, *ipermpr, *seppr;
   
    //InitRandom(-1);

    /* Copy the matrix over, getting rid of diagonal, and converting to
     * integer weights */
    // convertMatrix (A_IN, &xadj, &adjncy, &vwgt, &adjwgt);
    // setup the matrix
    
    

    n = 4;
    idxtype xadj[]={0,2,4,6,8};
    idxtype adjncy[]={1,2,0,3,0,3,1,2};
    idxtype vwgt[] = {1,1,1,1};
    idxtype adjwgt[] = {1,1,1,1,1,1,1,1};

    nparts = 2;

    /* Allocate memory for result of call */
    part = (idxtype*) calloc (n, sizeof(idxtype));
    
    wgtflag = 0;
    numflag = 0;
    nparts = 2;

    /* Do the call */
    METIS_PartGraphRecursive (&n, xadj, adjncy, NULL, NULL, &wgtflag,
                              &numflag, &nparts, options, &edgecut, part);
                              
    printf("done %i\n",IDXTYPEWIDTH);
    return 0;
}
