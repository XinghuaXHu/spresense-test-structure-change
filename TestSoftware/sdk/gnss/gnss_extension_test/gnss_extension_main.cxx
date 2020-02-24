//***************************************************************************
// test/sqa/singlefunction/gnss_extension_test/gnss_main.cxx
//
//   Data  : 2019/01/31
//   Author: Neusoft
//***************************************************************************


#include <sdk/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "gnss_api.hpp"

/****************************************************************************
 * Name: gnss_main
 ****************************************************************************/
extern "C" {
    int gnss_extension_main(int argc, char *argv[])
    {
        gnss_api *pGnssApi = gnss_api::GetSingletonInstance();
        if(pGnssApi != NULL)
        {
            printf("gnss app start\n");
            struct Command_ST cmd;
            cmd.type = kInvalid;
            while(pGnssApi->processUserInput(cmd))
            {
                pGnssApi->executeCommand(cmd);
                cmd.type = kInvalid;
            }
            pGnssApi->FreeSingletonInstance();
            printf("gnss app end\n");
            pGnssApi = NULL;
        }
        else
        {
            printf("gnss app failed\n");
        }
        return EXIT_SUCCESS;
    }
}
