function test_main
A = spaugment(ones(5)); A(1,10) = 1; A(10,1) = 1; A(5,6) = 1; A(6,5) = 1;
map = metismex('PartGraphRecursive',sparse(A),2);
map2 = metismex('PartGraphRecursive',sparse(A),2,struct('seed',2));
assert(any(map ~= map2));
