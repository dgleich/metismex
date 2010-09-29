metismex
========

Based on the [metismex.c](http://www.cs.ubc.ca/~rbridson/download/metismex.c) 
code by [Robert Bridson](http://www.cs.ubc.ca/~rbridson/) and
using pieces of the [meshpart toolkit](http://www.cerfacs.fr/algor/Softs/MESHPART/) 
by [John Gilbert](http://www.cs.ucsb.edu/~gilbert/) and 
[Shanghua Teng](http://www-bcf.usc.edu/~shanghua/).

This code has been modified to work with the latest version of Metis.
All functions except for one (NodeBisect) currently work.  


Compiling
---------

The following instructions worked on Ubuntu linux (10.4), I think they 
also work on 9.04.  I could _not_ get this to work on Ubuntu 6.06.

1. Download and extract the experimental version of 
  [metis](http://glaros.dtc.umn.edu/gkhome/metis/metis/overview).
    wget http://glaros.dtc.umn.edu/gkhome/fetch/sw/metis/metis-5.0pre2.tar.gz
    tar xzvf metis-5.0pre2.tar.gz
    cd metis-5.0pre2/
    
2. Fix a small compile bug with recent gcc libraries.  Edit `GKlib/trunk/gk_arch.h`
  and add the line `#define __USE_POSIX199309` as line 25.  The file should read:
        
        ... line 22
        #if !defined(__USE_XOPEN2K)
        #define __USE_XOPEN2K 
        #define __USE_POSIX199309 
        #endif
        #include <execinfo.h>
        #endif
        ... line 29

3. Configure metis for 64-bit integers.  _This step is only required
  for 64-bit versions of Matlab._  We need to edit `include/metis.h`
  and change `#define IDXTYPEWIDTH 32` to `#define IDXTYPEWIDTH 64`.
  On my system, this change was to line 35.
  (I also changed this variable in config/config.h, but that does
  not appear to be necessary.)
  
4. Configure metis for shared libraries. I believe this step is only 
  required for 64-bit systems as well, but I haven't tested it.
  We need to tell metis to compile with `-fPIC`.  Edit
  `Makefile.in` and add `-fPIC` to `COPTIONS` on line 57.  Likewise, 
  edit  `GKlib/trunk/Makefile` and add `-fPIC` to 
  `COPTIONS` on line 63.
  
5. Compile metis.
      
        make all
    
6. Download and compile metismex
        
        git clone git://github.com/dgleich/metismex.git
        cd metismex
        matlab -nodisplay
        >> mex -O -largeArrayDims -I../include -I../libmetis -I../GKlib/trunk ...
            -L../build/Linux-x86_64/ -lmetis metismex.c -DLINUX -DUNIX

7. Check that it works

        >> A = spaugment(ones(5)); A(1,10) = 1; A(10,1) = 1; A(5,6) = 1; A(6,5) = 1;
        >> [p1,p2] = metispart(sparse(A))
        
        p1 =
        
             1     2     3     4     5
        
        
        p2 =
        
             6     7     8     9    10

