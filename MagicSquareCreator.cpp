#include <iostream>
#include "MagicSquare.h"

#define BUILD 1
#define SOLVE 2

int main(int argc, char** argv) {

	std::locale::global(std::locale("C"));

	// 9 * 9 regular grid instance (ROOT SIZE = 3)
	MagicSquares::RegulareSquare regular_grid(3);

#if BUILD

	size_t hints_count = 25;

	if (argc > 1) {
		hints_count = atoi(argv[1]);
	}

	std::cout << "--------------------------- Create random grid with " << hints_count << " hints -------------------------------" << std::endl;

	regular_grid.completeGridWithHints(hints_count);

#endif

#if SOLVE
#if !BUILD
	regular_grid.setValue(6, 1, 7);
	regular_grid.setValue(8, 1, 5);

	regular_grid.setValue(3, 2, 7);
	regular_grid.setValue(5, 2, 3);
	regular_grid.setValue(7, 2, 2);
	regular_grid.setValue(9, 2, 6);

	regular_grid.setValue(2, 3, 4);
	regular_grid.setValue(4, 3, 1);
	regular_grid.setValue(9, 3, 7);

	regular_grid.setValue(4, 4, 6);

	regular_grid.setValue(1, 5, 9);
	regular_grid.setValue(3, 5, 1);
	regular_grid.setValue(5, 5, 8);
	regular_grid.setValue(6, 5, 3);

	regular_grid.setValue(2, 6, 7);
	regular_grid.setValue(5, 6, 2);
	regular_grid.setValue(7, 6, 3);
	regular_grid.setValue(8, 6, 1);

	regular_grid.setValue(3, 7, 4);
	regular_grid.setValue(6, 7, 1);
	regular_grid.setValue(7, 7, 6);
	regular_grid.setValue(8, 7, 3);
	regular_grid.setValue(9, 7, 9);

	regular_grid.setValue(3, 8, 8);

	regular_grid.setValue(2, 9, 9);
	regular_grid.setValue(7, 9, 5);
#endif


	std::cout << "------------------------------- Solve grid -----------------------------------" << std::endl;
	regular_grid.solve(false);
	std::cout << "------------------------------ Initial grid ----------------------------------" << std::endl;
	regular_grid.dump();
	std::cout << "-------------------------------- Solutions -----------------------------------" << std::endl;
	regular_grid.dumpSolutions();

#endif



}