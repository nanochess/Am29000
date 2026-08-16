int strncmp(s1, s2)               /* file strcmp.c */
char *s1,*s2;
{
        int     cnt;
        for(cnt=0;;cnt++)
        {       if(s1[cnt]!=s2[cnt])
                        return -1;
                if(s1[cnt]=='\0' || s2[cnt]=='\0') /* line 8 */
                        if(s1[cnt]=='\0' && s2[cnt]=='\0')
                                return 0;
                        else
                                return -1;
        }
} /* line 14 */
