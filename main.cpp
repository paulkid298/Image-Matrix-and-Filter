#include "global.h"
#include "utils.hpp"
#include "client.cpp"
#include <unistd.h>
#include <pthread.h>

extern int ind_p_in, ind_conv, ind_sob, ind_supp, ind_thresh,
    ind_hyster, ind_p_out;

int main()
{
    ind_p_out = 0;
    ind_hyster = ind_p_out + 1;
    ind_thresh = ind_hyster + 1;
    ind_supp = ind_thresh + 1;
    ind_sob = ind_supp + 1;
    ind_conv = ind_sob + 1;
    ind_p_in = ind_conv + 1;

    // server side code
    if (ind_p_in > ind_p_out)
    {
        int x = 0 + 0;
    }

    pthread_t tid;

    pthread_create(&tid, NULL, run_convolve_thread, NULL);
    pthread_create(&tid, NULL, run_sobel_thread, NULL);
    pthread_create(&tid, NULL, run_suppression_thread, NULL);
    pthread_create(&tid, NULL, run_threshold_thread, NULL);
    pthread_create(&tid, NULL, run_hysteresis_thread, NULL);
    pthread_create(&tid, NULL, run_client_thread, NULL);
    pthread_create(&tid, NULL, run_server_thread, NULL);

    return 1;
}
