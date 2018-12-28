#include<stdlib.h>
#include<stdio.h>
#include<iostream>
#include<string.h>
#include<math.h>
#include<vector>

#include "SimDC3D-PearsonCorrelation.h"

//--------------------------------------------------------
// FUNCTION PROTOTYPES
//--------------------------------------------------------
static FLOATINGPOINT arithmetic_mean(FLOATINGPOINT* data, int size);
static FLOATINGPOINT mean_of_products(FLOATINGPOINT* data1, FLOATINGPOINT* data2, int size);
static FLOATINGPOINT standard_deviation(FLOATINGPOINT* data, int size);

//--------------------------------------------------------
// FUNCTION pearson_correlation
//--------------------------------------------------------
FLOATINGPOINT pearson_correlation(FLOATINGPOINT* independent, FLOATINGPOINT* dependent, int size)
{
    FLOATINGPOINT rho;

    // covariance
    FLOATINGPOINT independent_mean = arithmetic_mean(independent, size);
    FLOATINGPOINT dependent_mean = arithmetic_mean(dependent, size);
    FLOATINGPOINT products_mean = mean_of_products(independent, dependent, size);
    FLOATINGPOINT covariance = products_mean - (independent_mean * dependent_mean);

    // standard deviations of independent values
    FLOATINGPOINT independent_standard_deviation = standard_deviation(independent, size);

    // standard deviations of dependent values
    FLOATINGPOINT dependent_standard_deviation = standard_deviation(dependent, size);

    // Pearson Correlation Coefficient
    rho = covariance / (independent_standard_deviation * dependent_standard_deviation);

	if ((independent_standard_deviation * dependent_standard_deviation) == 0)
		return 3;

    return rho;
}

//--------------------------------------------------------
// FUNCTION arithmetic_mean
//--------------------------------------------------------
static FLOATINGPOINT arithmetic_mean(FLOATINGPOINT* data, int size)
{
    FLOATINGPOINT total = 0;

    // note that incrementing total is done within the for loop
    for(int i = 0; i < size; total += data[i], i++);

    return total / size;
}

//--------------------------------------------------------
// FUNCTION mean_of_products
//--------------------------------------------------------
static FLOATINGPOINT mean_of_products(FLOATINGPOINT* data1, FLOATINGPOINT* data2, int size)
{
    FLOATINGPOINT total = 0;

    // note that incrementing total is done within the for loop
    for(int i = 0; i < size; total += (data1[i] * data2[i]), i++);

    return total / size;
}

//--------------------------------------------------------
// FUNCTION standard_deviation
//--------------------------------------------------------
static FLOATINGPOINT standard_deviation(FLOATINGPOINT* data, int size)
{
    std::vector<FLOATINGPOINT> squares;



    for(int i = 0; i < size; i++)
    {
		squares.push_back(pow(data[i], 2));
		//std::cout << "O valor na posicao " << i << " eh " << data[i] << "\n";
    }

    FLOATINGPOINT mean_of_squares = arithmetic_mean(&squares[0], size);
    FLOATINGPOINT mean = arithmetic_mean(data, size);
    FLOATINGPOINT square_of_mean = pow(mean, 2);
    FLOATINGPOINT variance = mean_of_squares - square_of_mean;
	//std::cout << "\nO quadrado da media eh: " << square_of_mean << " e a media do quadrado eh: " << mean_of_squares << " e a variancia eh: " << variance << "\n";
    FLOATINGPOINT std_dev = sqrt(variance);

    return std_dev;
}