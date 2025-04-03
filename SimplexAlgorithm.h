#pragma once
#include <vector>

class Simplex {
private:
    int m; // Number of constraints (excluding the objective row)
    int n; // Number of decision variables (excluding the RHS column)
    // The simplex tableau: row 0 is the objective function; rows 1..m are constraints.
    // The last column (index n) is the right-hand side (RHS).
    std::vector<std::vector<double>> tableau;

public:
    // Constructor: initializes the tableau from a given 2D vector.
    Simplex(const std::vector<std::vector<double>>& tableau_init);

    // Prints the current simplex tableau.
    void printTableau();

    // Performs a pivot operation at the specified pivot row and pivot column.
    void pivot(int pivotRow, int pivotCol);

    // Runs the simplex algorithm.
    // Returns true if an optimal solution is found, or false if the problem is unbounded.
    bool solve();

    // Retrieves the optimal solution (values for decision variables).
    // Non-basic variables are set to zero.
    std::vector<double> getSolution();

    // Returns the optimal value of the objective function.
    double getOptimalValue();
};
