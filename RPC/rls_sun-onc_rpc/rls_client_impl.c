#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "rls.h"

int
main (int argc, char *argv[])
{
	char 		*host, *dirname;
	CLIENT		*clnt;
	readdir_res	*result;
	nametype	rpc_arg;
	namelist	nl;

	if (argc != 3) {
		printf("usage: %s server_host directory_name\n", argv[0]);
		exit(1);
	}

	/* convenience assignments */
	host    = argv[1];
	dirname = argv[2];

	/* create client handle */
	clnt = clnt_create (host, DIRPROG, DIRVERS, "udp");
	if (clnt == NULL) {
		clnt_pcreateerror(host);
		exit(1);
	}

	/* setup rpc argument */
	rpc_arg = dirname;

	/* call remote procedure */
	result = readdir_1(&rpc_arg, clnt);
	if (result == (readdir_res *)NULL) {
		clnt_perror(clnt, "call failed");
	}

        /* check for errors */
        if (result->error_code != 0) {
                errno = result->error_code;
                perror(dirname);
                exit(1);
        }

	/* print results */
        for (nl = result->readdir_res_u.list; nl != NULL; nl = nl->pNext) {
                printf("%s\n", nl->name);
        }

	/* destroy client handle */
	clnt_destroy(clnt);

	return 0;
}

