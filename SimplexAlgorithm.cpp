#include "SimplexAlgorithm.h"
#include <iostream>
#include <iomanip>
#include <limits>

using namespace std;

Simplex::Simplex(const vector<vector<double>>& tableau_init) {
    tableau = tableau_init;
    m = tableau.size() - 1;       // Number of constraint rows
    n = tableau[0].size() - 1;      // Number of variable columns (excluding RHS)
}

void Simplex::printTableau() {
    for (size_t i = 0; i < tableau.size(); i++) {
        for (size_t j = 0; j < tableau[i].size(); j++) {
            cout << setw(10) << tableau[i][j] << " ";
        }
        cout << endl;
    }
    cout << endl;
}

void Simplex::pivot(int pivotRow, int pivotCol) {
    double pivotVal = tableau[pivotRow][pivotCol];
    // Normalize the pivot row so that the pivot element becomes 1.
    for (size_t j = 0; j < tableau[pivotRow].size(); j++) {
        tableau[pivotRow][j] /= pivotVal;
    }
    // For all other rows, eliminate the pivot column value.
    for (size_t i = 0; i < tableau.size(); i++) {
        if ((int)i != pivotRow) {
            double factor = tableau[i][pivotCol];
            for (size_t j = 0; j < tableau[i].size(); j++) {
                tableau[i][j] -= factor * tableau[pivotRow][j];
            }
        }
    }
}

bool Simplex::solve() {
    while (true) {
        // Find the entering variable: the most negative coefficient in the objective row.
        int pivotCol = -1;
        double mostNegative = 0;
        for (int j = 0; j < n; j++) {
            if (tableau[0][j] < mostNegative) {
                mostNegative = tableau[0][j];
                pivotCol = j;
            }
        }
        // If no negative coefficient exists, the solution is optimal.
        if (pivotCol == -1)
            break;

        // Determine the pivot row using the minimum ratio test.
        int pivotRow = -1;
        double minRatio = numeric_limits<double>::max();
        for (int i = 1; i <= m; i++) {
            if (tableau[i][pivotCol] > 0) {
                double ratio = tableau[i][n] / tableau[i][pivotCol];
                if (ratio < minRatio) {
                    minRatio = ratio;
                    pivotRow = i;
                }
            }
        }
        // If no valid pivot row is found, the problem is unbounded.
        if (pivotRow == -1) {
            cout << "The problem is unbounded." << endl;
            return false;
        }

        // Perform the pivot operation.
        pivot(pivotRow, pivotCol);
    }
    return true;
}

vector<double> Simplex::getSolution() {
    vector<double> solution(n, 0.0);
    // For each decision variable column, check if it is basic.
    for (int j = 0; j < n; j++) {
        int basicRow = -1;
        bool isBasic = true;
        for (int i = 1; i <= m; i++) {
            // A basic column has exactly one 1 and all other entries 0.
            if (tableau[i][j] == 1) {
                if (basicRow == -1)
                    basicRow = i;
                else {
                    isBasic = false;
                    break;
                }
            }
            else if (tableau[i][j] != 0) {
                isBasic = false;
                break;
            }
        }
        if (isBasic && basicRow != -1)
            solution[j] = tableau[basicRow][n];
    }
    return solution;
}

double Simplex::getOptimalValue() {
    // The optimal value is stored in the RHS of the objective row.
    return tableau[0][n];
}
