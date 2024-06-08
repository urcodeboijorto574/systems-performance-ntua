#include <iostream>
#include <algorithm> // min()

using namespace std;

enum TypeOfStation
{
  DELAY,
  LD,
  LI
};

const int M = 15; // αριθμός σταθμών (i)
const int C = 2;  // αριθμός κατηγοριών (j)
const int n[C + 1] = {0, 304, 240};
const int N = n[1] + n[2]; // αριθμός εργασιών (k)

double Q[M + 1][C + 1]; // see Bard-Schweitzer approximation (algorithm 5.2)
double X[C + 1];
const double D[M + 1][C + 1] = {
    {0, 0, 0},
    {0, 25.0, 29.0},
    {0, 0.032, 0.0},
    {0, 0.0, 0.090},
    {0, 0.048, 0.134},
    {0, 0.025, 0.0},
    {0, 0.0, 0.016},
    {0, 0.059, 0.0},
    {0, 0.070, 0.0},
    {0, 0.0, 0.28},
    {0, 0.0, 0.025},
    {0, 0.048, 0.058},
    {0, 0.054, 0.066},
    {0, 0.069, 0.106}, /* TODO: different */
    {0, 0.067, 0.072},
    {0, 0.088, 0.096}};
const TypeOfStation type_of_station[M + 1] = {/*DummyValue*/ DELAY,
                                              /* 1 */ DELAY,
                                              /* 2 */ LI,
                                              /* 3 */ LI,
                                              /* 4 */ LI,
                                              /* 5 */ DELAY,
                                              /* 6 */ DELAY,
                                              /* 7 */ LI,
                                              /* 8 */ LI,
                                              /* 9 */ LI,
                                              /* 10 */ LI,
                                              /* 11 */ LI,
                                              /* 12 */ LI,
                                              /* 13 */ LD,
                                              /* 14 */ LI,
                                              /* 15 */ LI};
double a[N + 1];
double p[N + 1];
double R[M + 1][C + 1];
double U[M + 1];

double maxD(int j)
{
  return D[13][j];
}

double sumD(int j)
{
  double sum = 0.0;
  for (int i = 1; i <= M; ++i)
    sum += D[i][j];
  return sum;
}

void calc_p(const int i = 13)
{
  /* Calculating p13(0|N) */
  double sum = 0.0, product[N + 1];
  for (int k = 1; k <= N; ++k)
  {
    product[k] = 1;
    for (int l = 1; l <= k; ++l)
    {
      double sum_nominator = 0;
      for (int j = 1; j <= C; ++j)
        sum_nominator += (X[j] * D[i][j]);

      product[k] *= sum_nominator / a[l];
    }
    sum += product[k];
  }
  p[0] = 1.0 / (1.0 + sum);

  /* Calculating p13(k|N) */
  for (int k = 1; k <= N; ++k)
    p[k] = p[0] * product[k];
}

int main()
{
  /* Initialize Qij */
  for (int j = 1; j <= C; ++j)
    for (int i = 1; i <= M; ++i)
      if (type_of_station[i] == LI)
        Q[i][j] = 0 /* TODO: change back n[j] / M */;

  /* Initialize Xij (for i=13 only) */
  for (int j = 1; j <= C; ++j)
    X[j] = min(1 / maxD(j), n[j] / sumD(j));

  /* Initialization of a */
  for (int k = 1; k <= N; ++k)
    a[k] = (k <= 64) ? 0.40 + 0.60 * k : 38.80;

  /* MVA: Main algorithm */
  for (double error = 0.0; error < 0.01; error += 0.005)
  {
    calc_p();
    for (int k = 1; k <= N; ++k)
    {
      for (int i = 1; i <= M; ++i)
        for (int j = 1; j <= C; ++j)
        {
          double sum = 0.0;
          switch (type_of_station[i])
          {
          case DELAY:
            R[i][j] = D[i][j];
            break;
          case LI:
            for (int l = 1; l <= C; ++l)
              sum += Q[i][l];
            R[i][j] = D[i][j] * (sum + 1.0);
            break;
          case LD:
            for (int j = 1; j <= C; ++j)
            {
              for (int k = 1; k <= N; ++k)
                sum += k * p[k - 1] / a[k];
              R[i][j] = D[i][j] * sum;
            }
            break;
          }
        }

      for (int j = 1; j <= C; ++j)
      {
        double sum = 0.0;
        for (int i = 1; i <= M; ++i)
          sum += R[i][j];

        X[j] = k / sum;
      }

      for (int i = 1; i <= M; ++i)
        for (int j = 1; j <= C; ++j)
          Q[i][j] = X[j] * R[i][j]; // TODO: check if X[j] =?= λj
    }
  }

  for (int j = 1; j <= C; ++j)
    for (int i = 1; i <= M; ++i)
      U[i] = X[j] * D[i][j];

  /* Display Results */ // X,R,Q,U ανά κατηγορία και συνολικά
  printf("Ρυθμός απόδοσης Xj:\n\tX_1: %f\n\tX_2: %f\n", X[1], X[2]);
  printf("Συνολικός ρυθμός απόδοσης X: %f\n", X[1] + X[2]);

  printf("Χρόνος απόκρισης Rj:\n");
  double R_total, R_[C + 1] = {0, 0.0, 0.0};
  for (int j = 1; j <= C; ++j)
  {
    for (int i = 1; i <= M; ++i)
      R_[j] += R[i][j];

    printf("\tR_%d: %f\n", j, R_[j]);
  }
  R_total = R_[1] + R_[2];
  printf("Συνολικός χρόνος απόκρισης R: %f\n", R_total);

  printf("Μέσος αριθμός εργασιών Qi:\n");
  double Q_mean[M + 1];
  for (int i = 1; i <= M; ++i)
  {
    Q_mean[i] = (Q[i][1] + Q[i][2]) /* TODO: change back / 2 */;
    printf("\tQ_%d: %f\n", i, Q_mean[i]);
  }

  printf("Βαθμός χρησιμοποίησης Ui:\n");
  for (int i = 1; i <= M; ++i)
    printf("\tU_%d: %f\n", i, U[i]);
}
