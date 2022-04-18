#include <time.h>

void tt_time()
{
        char *wday[]={
                "星期天","星期一","星期二","星期三","星期四","星期五","星期六"
        };
        time_t timep;
        struct tm *p;
        time(&timep);
        p = gmtime(&timep);
#ifdef DEBUG
        printf("%d年%02d月%02d日",(1900+p->tm_year),(1+p->tm_mon),p->tm_mday);
        printf("%s%02d:%02d:%02d\n",wday[p->tm_wday],(p->tm_hour+8),p->tm_min,p->tm_sec);
        fflush(NULL);
#endif
}
