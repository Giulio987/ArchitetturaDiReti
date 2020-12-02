#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>


int get_port(const char *str)
{
	long ret;
	char *endptr;

	ret = strtol(str, &endptr, 10);

	if (ret == 0 && errno == EINVAL) {
		// nessuna conversione effettuata
		return -1;
	}

	if (errno == ERANGE) {
		if (ret == LONG_MIN) {
			// underflow
			return -2;
		} else { // ret == LONG_MAX
			// overflow
			return -3;
		}
	}

	if (ret < 0 || ret > 65535) {
		// valore letto al di fuori del range di porte consentite
		return -4;
	}

	if (*endptr != '\0') {
		// non necessariamente un errore, ma condizione da segnalare
		fprintf(stderr, "attenzione: possibile problema!\n");
	}
