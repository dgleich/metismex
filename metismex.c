/****************************************************************************
* metismex.c
* Public domain MATLAB CMEX-file to let you use METIS-5.0 from MATLAB.
*
* Usage:
* [part,edgecut] = metismex('PartGraphRecursive',A,nparts,options)
* Output arguments, along with options, are optional
*
* Note that error checking is not done: make sure A is structurally
* symmetric or it will crash.
*
* To compile, you need to have Metis 5, and do something like (OSX)
* OSX
*   mex -O -largeArrayDims -I../include -I../libmetis -I../GKlib -L../build/Darwin-x86_64/libmetis -lmetis metismex.c -D__thread= -DLINUX
* LINUX
*   mex -O -largeArrayDims -I../include -I../libmetis -I../GKlib -L../build/Darwin-x86_64/libmetis -lmetis metismex.c -DLINUX
*
* Robert Bridson (and David F. Gleich)
*****************************************************************************/

/**
 * Modifications by David Gleich, started 2008-10-20
 * Fixed 64-bit Matlab sparse issues 
 */

#include "mex.h"

#if MX_API_VER < 0x07030000
typedef int mwIndex;
typedef int mwSize;
#endif /* MX_API_VER */

#include <strings.h>

#if IDXTYPEWIDTH == 32
#  ifdef MX_COMPAT_32
#  else 
#  error(32-bit metis is not compatible with 64-bit matlab, edit metis.h and recompile)
#  endif
#else
#  ifdef MX_COMPAT_32
#  error(64-bit metis is not compatible with 32-bit matlab, edit metis.h and recompile)
#  endif
#endif

#include <metislib.h>


void convertMatrix (const mxArray *A, idx_t **xadj, idx_t **adjncy,
                    idx_t **vwgt, idx_t **adjwgt)
{
    mwIndex i, j, jbar, n, nnz, *jc, *ir;
    double *pr;

    /* Find MATLAB's matrix structure */
    n = mxGetN(A);
    jc = mxGetJc(A);
    nnz = jc[n];
    ir = mxGetIr(A);
    pr = mxGetPr(A);

    /* Allocate room for METIS's structure */
    *xadj = (idx_t*) mxCalloc (n+1, sizeof(idx_t));
    *adjncy = (idx_t*) mxCalloc (nnz, sizeof(idx_t));
    *vwgt = (idx_t*) mxCalloc (n, sizeof(idx_t));
    *adjwgt = (idx_t*) mxCalloc (nnz, sizeof(idx_t));

    /* Scan the matrix, not copying diagonal terms, and rounding doubles
     * to integer weights */
    (*xadj)[0] = 0;
    jbar = 0;
    for (i = 1; i <= n; i++) {
        for (j = jc[i-1]; j < jc[i]; j++) {
            if (ir[j] != i-1) {
                (*adjncy)[jbar] = ir[j];
                (*adjwgt)[jbar] = (idx_t) pr[j];
                jbar++;
            } else {
                (*vwgt)[i-1] = (idx_t) pr[j];
            }
        }
        (*xadj)[i] = jbar;
    }
}

struct string_map_data {
    char *name;
    int val;
};
static struct string_map_data ctypeMap[] = {
 {"rm",                 METIS_CTYPE_RM},
 {"shem",               METIS_CTYPE_SHEM},
 {NULL,                 0}
};

static struct string_map_data iptypeMap[] = {
  {"grow",               METIS_IPTYPE_GROW},
  {"random",             METIS_IPTYPE_RANDOM},
  /* these are set via a function call EdgeND vs. NodeND */
  /* {"node",               METIS_IPTYPE_NODE}, */ 
  /* {"edge",               METIS_IPTYPE_EDGE}, */
  {NULL,                 0}
};

static struct string_map_data objtypeMap[] = {
  {"cut",                METIS_OBJTYPE_CUT},
  {"vol",                METIS_OBJTYPE_VOL},
  {NULL,                 0}
};

static struct string_map_data rtypeMap[] = {
  {"2sided",             METIS_RTYPE_SEP2SIDED},
  {"1sided",             METIS_RTYPE_SEP1SIDED},
  {NULL,                 0}
};

int parseString(mxArray* arg, struct string_map_data* map, char *argname) {
    char argval[25];
    if (!mxIsChar(arg)) {
      mexErrMsgIdAndTxt("metismex:invalidValue",
          "the value for option %s must be a string",
          argname);
    }
    
    mxGetString (arg, argval, 25);
    while (map->name != NULL) {
        if (strcasecmp(map->name,argval) == 0) {
            return map->val;
        }
        map++; 
    }
    mexErrMsgIdAndTxt("metismex:invalidValue",
          "the value %s for option %s is unrecognized",
          argval,argname);
}

struct option {
    char *name; 
    int has_arg; /* 1 => int arg, 0 => no arg, -1 => custom arg */
    int val;
};

struct parameter_data  {
#define METISMEX_OPTION_WGTFLAG -1
    int wgtflag;
#define METISMEX_OPTION_ADJWGT -2
    int adjwgt;
#define METISMEX_OPTION_VSIZE -3
    idx_t *vsize;
};

static struct option option_names[] = {
 {"seed", 1, METIS_OPTION_SEED},
 {"ctype", -1, METIS_OPTION_CTYPE},
 {"iptype", -1, METIS_OPTION_IPTYPE},
 {"objtype", -1, METIS_OPTION_OBJTYPE},
 {"rtype", -1, METIS_OPTION_RTYPE},
 {"minconn", 0, METIS_OPTION_MINCONN},
 {"contig", 0, METIS_OPTION_CONTIG},
 {"ufactor", 1, METIS_OPTION_UFACTOR},
 {"niter", 1, METIS_OPTION_NITER},
 {"ncuts",  1, METIS_OPTION_NCUTS},
 {"nseps",  1, METIS_OPTION_NSEPS},
 {"ccorder",  0, METIS_OPTION_CCORDER},
 {"nocompress", -1, METIS_OPTION_COMPRESS},
 {"pfactor", 1, METIS_OPTION_PFACTOR},
 {"dbglvl", 1, METIS_OPTION_DBGLVL},
 {"tpwgts", -1, METIS_OPTION_TPWGTS},
 {"ubvec", -1, METIS_OPTION_UBVEC},
 {"wgtflag", -1, METISMEX_OPTION_WGTFLAG},
 {"adjwgt", -1, METISMEX_OPTION_ADJWGT},
 {"vsize",-1, METISMEX_OPTION_VSIZE},
 {NULL, 0, 0},
}; 

void parseOptions(const mxArray *optarg, idx_t *options, struct parameter_data* params)
{
    struct option* opt = option_names;
    mxAssert(mxIsStruct(optarg), "options argument must be a structure");
    while (opt->name != NULL) {
        mxArray* mopt=mxGetField(optarg, 0, opt->name);
        if (mopt) {
            if (opt->has_arg == 1) {
                options[opt->val] = (int)mxGetScalar(mopt);
            } else if (opt->has_arg == 0) {
                options[opt->val] = 1;
            } else {
                switch (opt->val) {
                case METIS_OPTION_CTYPE:
                options[opt->val] = parseString(mopt, ctypeMap, opt->name);
                break;
                case METIS_OPTION_IPTYPE:
                options[opt->val] = parseString(mopt, iptypeMap, opt->name);
                break;
                case METIS_OPTION_OBJTYPE:
                options[opt->val] = parseString(mopt, objtypeMap, opt->name);
                break;
                case METIS_OPTION_RTYPE:
                options[opt->val] = parseString(mopt, rtypeMap, opt->name);
                break;
                case METIS_OPTION_TPWGTS:
                mxAssert(0, "tpwgts not handled");
                break;
                case METIS_OPTION_UBVEC:
                mxAssert(0, "ubvec not handled");
                break;
                case METIS_OPTION_COMPRESS: // this is the nocompress option
                options[opt->val] = 0;
                break;
                case METISMEX_OPTION_WGTFLAG:
                params->wgtflag = 1;
                break;
                case METISMEX_OPTION_ADJWGT:
                params->adjwgt = 1;
                break;
                default:
                    mxAssert(0, "unhandled option");
                }
            }
        }
        opt++;
    }
}



#define FUNC_IN (prhs[0])
#define A_IN (prhs[1])
#define NPARTS_IN (prhs[2])
#define NBWGTFLAG_IN (prhs[2])
#define NBOPTS_IN (prhs[3])

#define PART_OUT (plhs[0])
#define EDGECUT_OUT (plhs[1])
#define PERM_OUT (plhs[0])
#define IPERM_OUT (plhs[1])
#define SEP_OUT (plhs[0])

#define FUNCNAMELEN 25

void checkCall(int rval) {
    switch (rval) {
        case METIS_OK:
            return;
        case METIS_ERROR_INPUT:
            mexErrMsgIdAndTxt("metismex:metisError", "metis input error");
            break;
        case METIS_ERROR_MEMORY:
            mexErrMsgIdAndTxt("metismex:metisError", "metis memory error");
            break;
        default:
            mexErrMsgIdAndTxt("metismex:metisError", "unknown metis error");
            break;
    }
}
         

/****************************************************************************
* mexFunction: gateway routine for MATLAB interface.
*****************************************************************************/
void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    idx_t i, n, nparts, options[METIS_NOPTIONS] = {0}, edgecut, sepsize;
    idx_t *xadj, *adjncy, *vwgt, *adjwgt, *part, *perm, *iperm, *sep;
    
    char funcname[FUNCNAMELEN];
    double *optarray, *partpr, *permpr, *ipermpr, *seppr;
    
    struct parameter_data params = {
            0, // wgtflag = 0
            0, // adjwgt = 0
            NULL, // vsize = NULL
            }; 

    /* First do some general argument checking */
    if (nrhs < 2 || nrhs > 4 || nlhs > 2) {
        mexErrMsgTxt ("Wrong # of arguments");
    }
    if (!mxIsChar(FUNC_IN)) {
        mexErrMsgTxt ("First parameter must be a string");
    }
    n = mxGetN(A_IN);
    if (!mxIsSparse(A_IN) || n!=mxGetM(A_IN)) {
        mexErrMsgTxt ("Second parameter must be a symmetric sparse matrix");
    }

    METIS_SetDefaultOptions(options);

    /* Copy the matrix over, getting rid of diagonal, and converting to
     * integer weights */
    convertMatrix (A_IN, &xadj, &adjncy, &vwgt, &adjwgt);
    
    /* Allocate vsize */
    params.vsize = (idx_t*)mxMalloc(sizeof(idx_t)*n);

    /* Now figure out which function we have to do */
    mxGetString (FUNC_IN, funcname, FUNCNAMELEN);

    if (strcasecmp(funcname,"PartGraphRecursive")==0 ||
        strcasecmp(funcname,"PartGraphKway")==0 ) {

        /* Figure out values for nparts, wgtflag and options */
        if (nrhs < 3) {
            mexErrMsgTxt ("Third parameter needed: nparts");
        }
        nparts = (idx_t) mxGetScalar (NPARTS_IN);
        if (nrhs >= 4) {
            parseOptions(prhs[3], options, &params); 
        }
        
        if (params.wgtflag == 0) {
            for (i=0; i<n; ++i) {
                vwgt[i] = 1;
            }
        }
        
        if (params.adjwgt == 0) {
            for (i=0; i<xadj[n]; ++i) {
                adjwgt[i] = 1;
            }
        }

        if (nparts < 2) { 
            mexErrMsgTxt("nparts must be at least 2");
        }

        /* Allocate memory for result of call */
        part = (idx_t*) mxCalloc (n, sizeof(idx_t));
        
        idx_t ncon = 1;
        for (i=0; i<n; ++i) { 
            params.vsize[i] = 1;
        }

        /* Do the call */
        if (strcasecmp(funcname,"PartGraphRecursive") == 0) {
            checkCall(METIS_PartGraphRecursive (&n, &ncon, xadj, adjncy,
                 vwgt, params.vsize, adjwgt, 
                 &nparts, NULL, NULL, options, &edgecut, part));
        } else if (strcasecmp(funcname, "PartGraphKway") == 0) {
            checkCall(METIS_PartGraphKway (&n, &ncon, xadj, adjncy, 
                        vwgt, params.vsize, adjwgt, 
                        &nparts, NULL, NULL, options, &edgecut, part));
        } else {
            mxAssert(0,"unhandled case");
        }

        /* Figure out output values */
        if (nlhs >= 1) {
            PART_OUT = mxCreateDoubleMatrix (1, n, mxREAL);
            partpr = mxGetPr (PART_OUT);
            for (i = 0; i < n; i++) {
                partpr[i] = (double) part[i];
            }

            if (nlhs >= 2) {
                EDGECUT_OUT = mxCreateDoubleMatrix (1, 1, mxREAL);
                mxGetPr(EDGECUT_OUT)[0] = (double) edgecut;
            }
        }

    } else if (strcasecmp(funcname,"EdgeND")==0 || 
               strcasecmp(funcname,"NodeND")==0) {
        
        mexWarnMsgTxt("untested code");

        /* Figure out values for options */
        if (nrhs >= 3) {
            parseOptions(prhs[2], options, &params); 
        }

        /* Allocate memory for result of call */
        perm = (idx_t*) mxCalloc (n, sizeof(idx_t));
        iperm = (idx_t*) mxCalloc (n, sizeof(idx_t));

        /* Do the call */
        if (strcasecmp(funcname,"EdgeND")==0) {
            options[METIS_OPTION_IPTYPE] = METIS_IPTYPE_EDGE;
            METIS_NodeND (&n, xadj, adjncy, NULL, options, perm, iperm);
        } else {
            options[METIS_OPTION_IPTYPE] = METIS_IPTYPE_NODE;
            METIS_NodeND (&n, xadj, adjncy, NULL, options, perm, iperm);
        }

        /* Figure out output values */
        if (nlhs >= 1) {
            PERM_OUT = mxCreateDoubleMatrix (1, n, mxREAL);
            permpr = mxGetPr (PERM_OUT);
            for (i = 0; i < n; i++) {
                permpr[i] = perm[i]+1.0;
            }

            if (nlhs >= 2) {
                IPERM_OUT = mxCreateDoubleMatrix (1, n, mxREAL);
                ipermpr = mxGetPr (IPERM_OUT);
                for (i = 0; i < n; i++) {
                    ipermpr[i] = iperm[i]+1.0;
                }
            }
        }
#if 0
    } else if (strcasecmp(funcname,"NodeBisect")==0) {

        if (nrhs >= 3) {
            wgtflag = (idx_t) mxGetScalar (NBWGTFLAG_IN);
        } else {
            wgtflag = 0;
        }
        if (nrhs >= 4) {
            optarray = mxGetPr (NBOPTS_IN);
            for (i = 1; i < 4; ++i) {
                options[i] = (idx_t) optarray[i-1];
            }
        }

        /* Allocate memory for result of call */
        sep = (idx_t*) mxCalloc (n, sizeof(idx_t));

        /* Do the call */
        METIS_NodeBisect (n, xadj, adjncy, vwgt, adjwgt, wgtflag,
                          options, &sepsize, sep, 1.5);

        /* Figure out output values */
        if (nlhs >= 1) {
            SEP_OUT = mxCreateDoubleMatrix (1, sepsize, mxREAL);
            seppr = mxGetPr (PART_OUT);
            for (i = 0; i < sepsize; i++) {
                seppr[i] = (double) (sep[i]+1);
            }
        }
#endif    
    } else {
        mexErrMsgIdAndTxt ("metismex:invalidValue","Unknown metismex function %s",
            funcname);
    }
}



/*************************************************************************
* Given a graph, find a node separator bisecting it (roughly). The number
* of bisector nodes is returned in *nbnd, and the nodes themselves in the
* array bnds, which should already be allocated to have enough room.
**************************************************************************/
#if 0
void METIS_NodeBisect(idx_t nvtxs, idx_t *xadj, idx_t *adjncy,
                      idx_t *vwgt, idx_t *adjwgt, idx_t wgtflag,
                      idx_t *options, idx_t *nbnd, idx_t *bnds, double unbalance)
{
  idx_t i, j, tvwgt, tpwgts2[2];
  GraphType graph;
  CtrlType ctrl;
  idx_t *label, *bndind;

  if (options[0] == 0) {  /* Use the default parameters */
    ctrl.CType   = ONMETIS_CTYPE;
    ctrl.IType   = ONMETIS_ITYPE;
    ctrl.RType   = ONMETIS_RTYPE;
    ctrl.dbglvl  = ONMETIS_DBGLVL;
  }
  else {
    ctrl.CType   = options[OPTION_CTYPE];
    ctrl.IType   = options[OPTION_ITYPE];
    ctrl.RType   = options[OPTION_RTYPE];
    ctrl.dbglvl  = options[OPTION_DBGLVL];
  }
  ctrl.nseps = 5;    /* Take the best of 5 separators */
  ctrl.optype = OP_ONMETIS;
  ctrl.CoarsenTo = 50;

  IFSET(ctrl.dbglvl, DBG_TIME, InitTimers(&ctrl));
  IFSET(ctrl.dbglvl, DBG_TIME, starttimer(ctrl.TotalTmr));

  SetUpGraph(&graph, OP_ONMETIS, nvtxs, 1, xadj,adjncy,vwgt,adjwgt,wgtflag);

  /* Determine the weights of the partitions */
  tvwgt = idxsum(nvtxs, graph.vwgt);
  tpwgts2[0] = tvwgt/2;
  tpwgts2[1] = tvwgt-tpwgts2[0];

  ctrl.maxvwgt = (1.5*tvwgt)/ctrl.CoarsenTo;

  InitRandom(-1);

  AllocateWorkSpace(&ctrl, &graph, 2);

  MlevelNodeBisectionMultiple (&ctrl, &graph, tpwgts2, unbalance);

  IFSET(ctrl.dbglvl, DBG_SEPINFO, printf("Nvtxs: %6d, [%6d %6d %6d]\n", graph.nvtxs, graph.pwgts[0], graph.pwgts[1], graph.pwgts[2]));

  /* Now indicate the vertex separator */
  *nbnd = graph.nbnd;
  bndind = graph.bndind;
  label = graph.label;
  for (i = 0; i < *nbnd; ++i) 
    bnds[i] = label[bndind[i]];

  IFSET(ctrl.dbglvl, DBG_TIME, stoptimer(ctrl.TotalTmr));
  IFSET(ctrl.dbglvl, DBG_TIME, PrintTimers(&ctrl));

  FreeWorkSpace(&ctrl, &graph);
}
#endif
