#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*字符串匹配：BF算法
 *返回T在S中的位置
 *返回 -1表示不匹配*/
int str_bf(const char *S, const char *T)
{
	int sLen, tLen, i, j, k;

	sLen = strlen(S);
	tLen = strlen(T);
	
	i = 0;
	while (i + tLen <= sLen) {
		k = i;
		for (j = 0; (j < tLen)&&(k < sLen)&&(S[k] == T[j]); k++, j++);
		if (j == tLen) return (k - tLen);
		i ++;
	}

	return -1;
}


int main()
{
	int ret;

	char s[] = "123abcsefedff";
	char t[] = "abcsefe";

	ret = str_bf(s, t);

	if (ret >= 0) 
		printf("yes   %d\n", ret);
	else
		printf("no\n");

	return 0;
}
