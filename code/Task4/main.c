#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "../rng_gen.h"
#include "../helper.h"

// DEFINES
#define d_param 0.8
#define nbr_of_dimensions 3


// HELPER FUNCTION DECLARATIONS
 // -----------------------------------------------------------------
double trial_wave(double*, double*, double);
double local_energy(double*, double*, double);
void   new_configuration(double *, double*);
double density_probability(double,double);
void montecarlo_E_trial(int,int,double(),double(),double(),double,double*);

// MAIN PROGRAM
// ------------------------------------------------------------------
int main()
{
    // Initialize the gsl random number generator
    Initialize_Generator();

    // ---- Variable Declarations ----
    // -------------------------------
    double h_bar;
    double e;
    double m_e;
    double e4pi;
    double alpha_0;

    // Task Specific parameters
    double A;
    double beta_min;
    double beta_max;

    int nbr_of_trials       =   100000;
    int nbr_of_trials_eq    =   1000;
    int max_p               =   500;
    int nbr_of_runs         =   10;
    int nbr_of_beta_runs    =   4;

    #define alpha_p(i,a,p,b) (alpha_p_arr[b*max_p*2*nbr_of_beta_runs+i*max_p*2+a*max_p+p])
    double* alpha_p_arr = (double*)malloc(nbr_of_beta_runs*2*max_p*nbr_of_runs*sizeof(double));
    int size = nbr_of_beta_runs*2*max_p*nbr_of_runs;
    printf("%i\t%i\t%i\t%i\t%i\n",size, nbr_of_beta_runs,2,max_p, nbr_of_runs );

    // ---- Initialize Variables ----
    // ------------------------------
    h_bar       = 1.0;
    e           = 1.0;
    m_e         = 1.0;
    e4pi        = 1.0;
    alpha_0     = 0.15;
    A           = 1.0;
    beta_min    = 0.50;
    beta_max    = 1.0;


    for (int b = 0; b < nbr_of_beta_runs; b++)
    {
        double min_alpha= 100;
        double min_energy = 100;
        double beta = beta_min+(beta_max-beta_min)*(double)(b)/(double)(nbr_of_beta_runs);
        printf("Beta run: %i\n",b );
        for (int i = 0; i < nbr_of_runs; i++)
        {
            double current_alpha = alpha_0;
            for (int p = 0; p < max_p; p++)
            {
                double* output = (double*)malloc(3*sizeof(double));
                montecarlo_E_trial(nbr_of_trials,nbr_of_trials_eq,local_energy,gradient_alpha, trial_wave, current_alpha,output);

                double current_energy = output[0];
                double current_gradie = output[1];
                double current_produc = output[2];

                double grad_energy = 2*(current_produc-current_energy*current_gradie);

                double step = step_length(A,p+1,beta);
                alpha_p(i,0,p,b)=current_alpha;
                alpha_p(i,1,p,b)=current_energy;
                if (current_energy < min_energy)
                {
                    min_energy = current_energy;
                    min_alpha = current_alpha;
                }
                current_alpha -= step*grad_energy;
            }
        }
    }


    // ----- Print results to file -----
    // ---------------------------------
    FILE* file;

    file = fopen("alpha.dat","w");
    for (int b = 0 ; b < nbr_of_beta_runs; b++)
    {
        for (int p = 0; p < max_p; p++)
        {
            for (int i = 0; i < nbr_of_runs; i++)
            {
                fprintf(file,"%e \t %e \t", alpha_p(i,0,p,b), alpha_p(i,1,p,b));
            }
            fprintf(file, "\n");
        }
        fprintf(file, "\n");
    }
    fclose(file);

    free(alpha_p_arr);
    // Free the gsl random number generator
    Free_Generator();
    return 0;
}

// HELPER FUNCTION DEFINITIONS
// ------------------------------------------------------------------
double trial_wave(double* r_1, double* r_2, double alpha)
{
    double r1 = array_abs(r_1,nbr_of_dimensions);
    double r2 = array_abs(r_2,nbr_of_dimensions);
    double r_12[nbr_of_dimensions];
    array_diff(r_1,r_2,r_12, nbr_of_dimensions);
    double r12 = array_abs(r_12,nbr_of_dimensions);

    double f_val = exp(-2*r1-2*r2+r12/(2.0*(1.0+alpha*r12)));

    return f_val;
}

double local_energy(double* r_1, double* r_2, double alpha)
{
    double r1 = array_abs(r_1,nbr_of_dimensions);
    double r_12[nbr_of_dimensions];
    double r2 = array_abs(r_2,nbr_of_dimensions);
    array_diff(r_1,r_2,r_12,nbr_of_dimensions);
    double r12 = array_abs(r_12,nbr_of_dimensions);

    double* unit_1 = malloc(sizeof(double)*nbr_of_dimensions);
    double* unit_2 = malloc(sizeof(double)*nbr_of_dimensions);
    array_scalar(unit_1, r_1, 1.0/r1,nbr_of_dimensions);
    array_scalar(unit_2, r_2, 1.0/r2,nbr_of_dimensions);
    double unit_diff[nbr_of_dimensions] ;
    array_diff(unit_1,unit_2,unit_diff,nbr_of_dimensions);

    double term1 = array_mult(unit_diff,r_12,nbr_of_dimensions)/(r12*(1+alpha*r12)*(1+alpha*r12));
    double term2 = - 1/(r12*(1+alpha*r12)*(1+alpha*r12)*(1+alpha*r12));
    double term3 = - 1/(4*(1+alpha*r12)*(1+alpha*r12)*(1+alpha*r12)*(1+alpha*r12));
    double term4 = 1/r12;

    double energy = 0;
    energy = -4 +term1+term2+term3+term4;

    free(unit_1);
    free(unit_2);

    return energy;
}

void new_configuration(double * r_1, double* r_2)
{
    for (int i = 0; i < nbr_of_dimensions; i++)
    {
        r_1[i]+= d_param*(randq()-0.5);
        r_2[i]+= d_param*(randq()-0.5);
    }
}

void montecarlo_E_trial(int N ,int equilibrium_time,double (*local_e)(double*,double*,double),double(*gradient)(double*,double*,double,int),double(*prob)(double*,double*,double),double alpha,double* out)
{
    double r_1[nbr_of_dimensions] = { 0 };
    double r_2[nbr_of_dimensions] = { 0 };

    r_1[0] = 1.0;
    r_1[1] = 0.0;
    r_1[2] = 0.0;
    r_2[0] = -1.0;
    r_2[1] = 0.0;
    r_2[2] = 0.0;

    double* energy              = malloc(sizeof(double)*(N-equilibrium_time));
    double* grad_trial          = malloc(sizeof(double)*(N-equilibrium_time));
    double* grad_trial_energy   = malloc(sizeof(double)*(N-equilibrium_time));

    for (int i = 0; i < N; i++)
    {
        // Allocate memory for trial state
        double r_1_new[nbr_of_dimensions];
        double r_2_new[nbr_of_dimensions];

        // Copy values from previous arrays
        memcpy(r_1_new, r_1, nbr_of_dimensions*sizeof(double));
        memcpy(r_2_new, r_2, nbr_of_dimensions*sizeof(double));

        // Generate new configuration
        new_configuration(r_1_new, r_2_new);

        double relative_prob = relative_probability(r_1_new,r_2_new,r_1,r_2,alpha,prob,nbr_of_dimensions);

        if (relative_prob > 1)
        {
            memcpy(r_1, r_1_new, nbr_of_dimensions*sizeof(double));
            memcpy(r_2, r_2_new, nbr_of_dimensions*sizeof(double));
        }
        else if (relative_prob > randq())
        {
            memcpy(r_1, r_1_new, nbr_of_dimensions*sizeof(double));
            memcpy(r_2, r_2_new, nbr_of_dimensions*sizeof(double));
        }
        if (i >= equilibrium_time)
        {
            energy[i-equilibrium_time]      = local_e(r_1,r_2,alpha);
            grad_trial[i-equilibrium_time]  = gradient(r_1,r_2,alpha,nbr_of_dimensions);
            grad_trial_energy[i-equilibrium_time]  = grad_trial[i-equilibrium_time]*energy[i-equilibrium_time];
        }
    }

    double mean_energy              = calc_mean(energy,N-equilibrium_time);
    double mean_grad_trial          = calc_mean(grad_trial,N-equilibrium_time);
    double mean_grad_trial_energy   = calc_mean(grad_trial_energy,N-equilibrium_time);

    free(energy);
    free(grad_trial);

    out[0] = mean_energy;
    out[1] = mean_grad_trial;
    out[2] = mean_grad_trial_energy;
}

double density_probability(double r, double Z)
{
    double prob=pow(Z,3)*4*pow(r,2)*exp(-2*Z*r);
    return prob;
}
