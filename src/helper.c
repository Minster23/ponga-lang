#include "helper.h"
#include <unistd.h>
#include <stdio.h>

bool isFileExists(char *path)
{
	if(access(path, F_OK) == 0)
	{
		return true;
	}else {return false;}
}

bool isRightType(char *path)
{
	if(strstr(path, ".pga"))
	{
		return true;
	}else{return false;}
}
