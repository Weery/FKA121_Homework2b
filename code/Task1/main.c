#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "../rng_gen.h"
#include "../helper.h"

// DEFINES

#define d_param 1.0
#define nbr_of_dimensions 3


// HELPER FUNCTION DECLARATIONS
 // -----------------------------------------------------------------
double  trial_wave(double*, double*, double);
double  local_energy(double*, double*, double);
void    new_configuration(double *, double*);
double  montecarlo(int ,int ,double(), double(), double, double*, double*);
double  density_probability(double,double);

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
    double alpha;

    int nbr_of_trials       = 10000000;
    int nbr_of_trials_eq    = nbr_of_trials/10;

    double* rads            = (double*)malloc(nbr_of_trials*2*sizeof(double));
    double* angle_diff      = (double*)malloc(nbr_of_trials*sizeof(double));


    // ---- Initialize Variables ----
    // ------------------------------
    h_bar   = 1;
    e       = 1;
    m_e     = 1;
    e4pi    = 1;
    alpha   = 0.1;


    // ===== RUN MONTE CARLO SIMULATION =====
    double monte = montecarlo(nbr_of_trials,nbr_of_trials_eq,local_energy, trial_wave, alpha,rads,angle_diff);
    // ---- Print energy result -----
    printf("E_0: %e \n", monte );


    // ====== PRINT SIMULATION RESULTS TO FILE ======
    printf("Im here\n");
    FILE* file;
    file = fopen("rads.dat","w");
    for (int i = 0; i < 2*(nbr_of_trials-nbr_of_trials_eq); i++)
    {
        fprintf(file, "%e\n", rads[i] );
    }
    fclose(file);

    printf("Im here again\n");
    file = fopen("angle_diff.dat","w");
    for (int i = 0; i < nbr_of_trials-nbr_of_trials_eq; i++)
    {
        fprintf(file, "%e\n", angle_diff[i] );
    }
    fclose(file);

    printf("Im here yet again\n");
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
        double r1 = randq()-0.5;
        double r2 = randq()-0.5;
        r_1[i] += d_param * r1;
        r_2[i] += d_param * r2;
    }
}


double  montecarlo(int N, int equilibrium_time,double (*local_e)(double*,double*,double), double (*f)(double*,double*,double), double alpha, double* rads, double* angle_diff)
{
    double r_1[nbr_of_dimensions] = { 0 };
    double r_2[nbr_of_dimensions] = { 0 };

    r_1[0] = 1.0;
    r_1[1] = 0.0;
    r_1[2] = 0.0;
    r_2[0] = -1.0;
    r_2[1] = 0.0;
    r_2[2] = 0.0;

    double* energy = malloc(sizeof(double)*(N-equilibrium_time));

    for (int i = 0; i < N; i++)
    {
        // Allocate memory for trial state
        double r_1_new[nbr_of_dimensions];
        double r_2_new[nbr_of_dimensions];

        // Copy values from previous arrays
        memcpy(r_1_new, r_1, nbr_of_dimensions*sizeof(double));
        memcpy(r_2_new, r_2, nbr_of_dimensions*sizeof(double));

        // Generate a new configuration
        new_configuration(r_1_new, r_2_new);


        double relative_prob = relative_probability(r_1_new,r_2_new,r_1,r_2,alpha,f,nbr_of_dimensions);

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

            double R1 = array_abs(r_1, nbr_of_dimensions);
            double R2 = array_abs(r_2, nbr_of_dimensions);
            rads[2*(i-equilibrium_time)] = R1;
            rads[2*(i-equilibrium_time)+1] = R2;

            /* Compute angle between vectors with scalar product */
            angle_diff[i-equilibrium_time] =
                acos( (r_1[0]*r_2[0] + r_1[1]*r_2[1] + r_1[2]*r_2[2]) / (R1*R2) );
            /* Compute local energy */
            energy[i-equilibrium_time] = local_e(r_1,r_2,alpha);
        }
    }



    double mean_energy =calc_mean(energy,N-equilibrium_time);

    free (energy);

    return mean_energy;
}

double density_probability(double r, double Z)
{
    double prob=pow(Z,3)*4*pow(r,2)*exp(-2*Z*r);
    return prob;
}
