#include <stdio.h>
#include <time.h>

int main(void)
{
    time_t mytime;
    mytime = time(NULL);
    printf("%s",ctime(&mytime));

    return 0;
}
