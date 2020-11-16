#include <stdlib.h>
#include <stdio.h>
#include <sys/dir.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include "rls.h"


readdir_res *
readdir_1_svc(nametype *dirname, struct svc_req *rqstp)
{
        namelist nl;
        namelist *nlp;
        struct direct *d;
	DIR *dirp = NULL;
	static readdir_res res; /* must be static */

        /* open the directory */
        dirp = opendir(*dirname);
        if (dirp == NULL) {
                res.error_code = errno;
                return &res;
        }

        /* free previously allocated memory */
        xdr_free((xdrproc_t)xdr_readdir_res, (char *)&res);

        /* read directory */
        nlp = &res.readdir_res_u.list;
        while ((d = readdir(dirp)) != NULL) {
                nl = *nlp = (namenode *)malloc(sizeof(namenode));
                nl->name = strdup(d->d_name);
                nlp = &nl->pNext;
        }
        *nlp = NULL;

        /* return the result */
        res.error_code = 0;
        closedir(dirp);

        return &res;
}

int
remotelsprog_1_freeresult(SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result)
{
        puts("called freeresult"); fflush(stdout);
        xdr_free (xdr_result, result);

        return 1;
}


