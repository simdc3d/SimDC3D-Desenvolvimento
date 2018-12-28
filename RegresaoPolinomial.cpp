#include <stdio.h>
#include <math.h>
#include <stdlib.h>
//------------------------------------------------------------------------------
#define   MAX       501
#define   p         6
#define   ENTRADA   "entrada.txt"
#define   SAIDA     "saida.txt"
#define   RESULTADO "resultado.txt"
//------------------------------------------------------------------------------
int    n = 0;
double ym;
//------------------------------------------------------------------------------
double fx        ( double x          , double a[p+1] );
void   dado      ( double x[MAX]     , double y[MAX] );
void   matriz    ( double A[p+1][p+1], double B[p+1], double x[MAX], double y[MAX] );
void   sistema   ( double A[p+1][p+1], double a[p+1], double B[p+1] );
void   calcula   ( double a[p+1]     , double x[MAX], double y[MAX] );
void   resultado ( double a[p+1]     , double x[MAX], double y[MAX] );
void   pivot     ( int l, int m,  double A[p+1][p+1], double B[p+1] );
//------------------------------------------------------------------------------
int RegressaoPolinomial(void)
{
    int    i,k;
    double x[MAX], y[MAX], A[p+1][p+1], a[p+1], B[p+1];

    dado( x, y );
    matriz( A, B, x, y );
    sistema( A, a, B );
    calcula( a, x, y );
    resultado( a, x, y );

    printf( "\nfim do processamento...\n"
            "\nPressione qualquer tecla para finalizar" );
    getchar();

    return 0;
}
//------------------------------------------------------------------------------
double fx(double x, double a[p+1] )
{
    int k;
    double y = 0.0;
    
    for( k = 0; k < p+1; k++ )
         y += a[k] * pow( x, (double) k );
    return y;
}
//------------------------------------------------------------------------------
void dado(double x[MAX], double y[MAX])
{
    FILE *f;
    
    if(( f = fopen(ENTRADA,"r") ) == NULL )
    {
        printf( "\n Erro de I/O" );
        printf( "\nPressione qualquer tecla para finalizar" );
            getchar();
        exit(1);
    }

    while( !feof(f) )
    {
        fscanf( f, "%lf %lf\n", &x[n], &y[n] );
        n++;
    }
    fclose(f);
}
//------------------------------------------------------------------------------
void matriz(double A[p+1][p+1], double B[p+1], double x[MAX], double y[MAX])
{
    int i, j, l;
    for( j = 0; j < p+1; j++ )
        for( l = 0; l < p+1; l++ )
        {
            A[j][l] = 0.0;
            for( i = 0; i < n; i++ )
                 A[j][l] += pow( x[i], (double) j+l );
        }
     
    for( j = 0; j < p+1; j++ )
    {
        B[j] = 0.0;
        for( i = 0; i < n; i++ )
             B[j] += pow( x[i], (double) j ) * y[i];
    }
}
//------------------------------------------------------------------------------
void pivot( int l, int m, double A[p+1][p+1], double B[p+1] )
{
    int     i, l_maior;
    double  maior, auxA, auxB;

    maior   = A[l][l];
    l_maior = l;

    for( i = l; i < m; i++ )
        if( fabs(maior) < fabs(A[i][l]) )
        {
            maior   = A[i][l];
            l_maior = i;
        }

    if( l != l_maior )
    {
        for( i = l; i < m; i++ )
        {
            auxA          = A[l][i];
            A[l][i]       = A[l_maior][i];
            A[l_maior][i] = auxA;
        }
        
        auxB       = B[l];
        B[l]       = B[l_maior];
        B[l_maior] = auxB;
    }
}
//------------------------------------------------------------------------------
void sistema(double A[p+1][p+1], double a[p+1], double B[p+1])
{
    int i, j, l, c, k;
    double aux;
    for( i= 0; i < p+1; i++ )
    {
        pivot( i, p+1, A, B );
        aux = A[i][i];

        for( j = 0; j < p+1; j++ )
            if( aux != 0.0 ) A[i][j] /= aux;
        if( aux != 0.0 ) B[i]    /= aux;

        for( l = i+1; l < p+1; l++ )
        {
            aux = A[l][i];
            for( c = i; c < p+1; c++ )
                 A[l][c] -= aux * A[i][c];
            B[l] -= aux * B[i];
        }
    }

    a[p] = B[p];
    for( l = p-1; l >= 0; l-- )
    {
       a[l] = B[l];
       for( c = l+1; c < p+1; c++ )
           a[l] -= A[l][c] * a[c];
    }
}
//------------------------------------------------------------------------------
void calcula(double a[p+1], double x[MAX], double y[MAX])
{
    int     i, k;
    double  Sx, Sy, e, ye, Se, Sye;
    FILE   *f;
    
    Sx = Sy = Se = Sye = 0.0;

    if(( f = fopen(SAIDA,"w")) == NULL)
    {
        printf( "\n Erro de I/O" );
        printf( "\nPressione qualquer tecla para finalizar" );
        getchar();
        exit(3);
    }

    fprintf( f, " ============================================================\n" );
    fprintf( f, "     i     x[i]      y[i]      ye[i]      e[i] \n" );
    fprintf( f, " ============================================================\n" );
    for( i = 0; i < n; i++ )
    {
        Sx  += x[i];
        Sy  += y[i];
        ye   = fx( x[i], a );
        e    = y[i] - ye;
        Se  += e;
        Sye += ye;
        fprintf(f," %5d %9.5lf %9.5lf %9.5lf %9.5lf\n", i, x[i],y[i], ye, e);
    }
    fprintf(f," ============================================================\n");
    fprintf(f," %5d %9.5lf %9.5lf %9.5lf %9.5lf\n", n, Sx, Sy, Sye, Se);
    ym = Sy / n;
    fclose(f);
}
//------------------------------------------------------------------------------
void resultado(double a[p+1], double x[MAX], double y[MAX])
{
    int    i;
    double SQReg, SQRes, R2, ye, SQT;
    FILE  *f;
    
    SQRes = SQReg = 0.0;

    for( i = 0; i < n; i++ )
    {
        ye     = fx( x[i], a );
        SQReg += pow( ye   - ym, 2.0 );
        SQRes += pow( y[i] - ye, 2.0 );
    }
    SQT   = SQReg + SQRes;
    R2    = SQReg / SQT;

    if((f = fopen(RESULTADO,"w") ) == NULL)
    {
        printf( "\n Erro de I/O" );
        printf( "\nPressione qualquer tecla para finalizar" );
        getchar();
        exit(2);
    }

    fprintf(f," R2= %12e \n\n", R2 );
    printf(" R2= %12e \n\n", R2 );
    for( i = 0; i < p+1; i++ )
    {
         fprintf(f," a[%d]= %+12e \n", i , a[i]);
         printf (  " a[%d]= %+12e \n", i , a[i]);        
    }
    fclose(f);
}
//------------------------------------------------------------------------------
