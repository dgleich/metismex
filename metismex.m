function metismex
% METISMEX Establish an interface between METIS and Matlab
% 
% [map,edgecut] = metismex('PartGraphRecursive',A,nparts,options);
% [map,edgecut] = metismex('PartGraphKway',A,nparts,options);
% [perm,iperm] = metismex('EdgeND',A,options);
% [perm,iperm] = metismex('NodeND',A,options);
%
% options is now a structure with the following fields:
%   'seed' : an integer for the random seed used in metis
%   'ctype' : 'rm' or 'shem' [default]
%   'iptype' : 'grow' or 'random' (only applys for recursive bisection)
%   'objtype' : 'cut' or 'vol' [default] (only applys for part kway)
%   'rtype' : '1sided' [default] or '2sided' 
%   'ufactor' : an integer for balance
%   'pfactor' : an integer for minimum degree of vertex to be ordered last
%   'ccorder' : a flag (value ignored) to order connected components
%               separately
%   'nseps' : an integer for the number of separators tried at each level
%               (default 1)
%   'niter' : an integer for the number of refinement iterations  
%               (default 10)
%   'ncuts' : number of initial partitions to test (default 1)
%   'dbglvl' : the debug level (default 0)
% 
% The output and options commands are optional.
%
% Note that error checking is not done: make sure A is structurally
% symmetric or it will crash.
%
% See also METISDICE METISPART
