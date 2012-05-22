metismex
========

Based on the [metismex.c](http://www.cs.ubc.ca/~rbridson/download/metismex.c) 
code by [Robert Bridson](http://www.cs.ubc.ca/~rbridson/) and
using pieces of the [meshpart toolkit](http://www.cerfacs.fr/algor/Softs/MESHPART/) 
by [John Gilbert](http://www.cs.ucsb.edu/~gilbert/) and 
[Shanghua Teng](http://www-bcf.usc.edu/~shanghua/).

This code has been modified to work with the latest version of Metis (5.0.2
as if 2012-05-22).
All functions except for one (NodeBisect) currently work.  


Compiling
---------

The following instructions worked on Ubuntu Linux (10.10) and
OSX 10.7.

1. Download and extract the 5.0.2 version of 
  [metis](http://glaros.dtc.umn.edu/gkhome/fetch/sw/metis/metis-5.0.2.tar.gz).
  
        wget http://glaros.dtc.umn.edu/gkhome/fetch/sw/metis/metis-5.0.2.tar.gz
        tar xzvf metis-5.0pre2.tar.gz
        cd metis-5.0pre2/
    

2. Configure metis for 64-bit integers.  _This step is only required
  for 64-bit versions of Matlab._  We need to edit `include/metis.h`
  and change `#define IDXTYPEWIDTH 32` to `#define IDXTYPEWIDTH 64`.
  On my system, this change was to line 33.
  
3. Make sure you have a working version of `cmake` installed.  On OSX 10.7,
  I used the homebrew port. `brew install cmake`  
  
5. Compile metis.
      
        make config
        make all
    
6. Download and compile metismex. **It is critical that this is in a
  subdirectory of metis** otherwise the relative paths won't work.
        
        # in the metis-5.0.2 directory
        git clone git://github.com/dgleich/metismex.git
        cd metismex
        matlab -nodisplay
        >> make

7. Check that it works

        >> A = spaugment(ones(5)); A(1,10) = 1; A(10,1) = 1; A(5,6) = 1; A(6,5) = 1;
        >> [p1,p2] = metispart(sparse(A))
        
        p1 =
        
             1     2     3     4     5
        
        
        p2 =
        
             6     7     8     9    10

