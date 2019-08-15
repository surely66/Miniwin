#include<gtest/gtest.h>
#include <window.h>
#include <signal.h>
#include <execinfo.h>
#include <ngl_log.h>

void signal_handler(int signo)
{
   int j, nptrs;
#define SIZE 100
   void *buffer[100];
   char **strings;

   nptrs = backtrace(buffer, SIZE);
   printf("backtrace() returned %d addresses\n", nptrs);

   /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
      would produce similar output to the following: */

   strings = backtrace_symbols(buffer, nptrs);
   if (strings == NULL) {
       perror("backtrace_symbols");
       exit(EXIT_FAILURE);
   }

   for (j = 0; j < nptrs; j++)
       printf("%s\n", strings[j]);

   free(strings);
   signal(signo, SIG_DFL); /* 恢复信号默认处理 */  
   raise(signo);
}

int main(int argc,char*argv[])
{
    //signal(SIGSEGV, signal_handler);
    //signal(SIGFPE,signal_handler);
    
    nglLogParseModules(argc,(const char**)argv);
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
