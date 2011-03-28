#include "String.h"

String StringF(const char* sFmt,...){
	va_list listArgs;
	char *vch;

	va_start(listArgs, sFmt);
	vasprintf(&vch, sFmt, listArgs);
	va_end(listArgs);
	String s(vch);
	free(vch);
	return s;
}

