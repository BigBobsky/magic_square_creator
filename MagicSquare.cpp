#include <iostream>
#include <ctime>
#include <map>
#include <unordered_set>

#include "MagicSquare.h"

namespace MagicSquares {

	// CONVENTION : 
	// - en minuscule les variables d'indexation des tableaux (entre 0 et N-1)
	// - en majuscule les variables fonctionnelles (numéros de ligne/colonne de 1 à N

	RegulareSquare::RegulareSquare(size_t grid_root_size) :
			mGridRootSize(grid_root_size),
			mGridHintsNumber(VOID_VALUE),
			mInternalGrid(grid_root_size* grid_root_size, 0),
		    mAllowedValuesMap(grid_root_size* grid_root_size, std::vector< std::set<size_t> >(grid_root_size* grid_root_size, std::set<size_t>())),
			mSolved(false) {
		
		mMinAllowedValue = 1;
		mMaxAllowedValue = mGridRootSize * mGridRootSize;

		this->resetInternalGrids();

	}

	RegulareSquare::~RegulareSquare() {
		// Something to do ?
	}

	RegulareSquare::ERROR_CODE RegulareSquare::completeGridWithHints(size_t hints_number) {

		RegulareSquare::ERROR_CODE ret = ERR_OK;
		size_t new_route_case = 0;
		char   mbstr[100];

		// Copy the grid in a new one
		CellHypothesis h;

		// Introduce entropy;
		srand(time(0));

		// First of all, complete a line randomly
		size_t J = 1 + rand() % this->mMaxAllowedValue;
		std::unordered_set<size_t> allowed_values;

		for (size_t V = 1; V <= this->mMaxAllowedValue; V++) {
			allowed_values.emplace(V);
		}

		size_t I = 1;
		while (!allowed_values.empty() && (I <= this->mMaxAllowedValue)) {
			auto it = allowed_values.begin();
			std::advance(it, rand() % allowed_values.size());
			this->setValue(I,J,*it);
			allowed_values.erase(it);
			I++;
		}

		// Find a first solution (unique or not)
		this->solve(true);

		// get the first solution
		RegulareSquare sol = this->solutions()[0];

		// create a scrambled "route" of cells to clear from a fresh grid
		OrderedHypothesisMap hypothesis_map;
		RegulareSquare       empty_grid(mGridRootSize);

		empty_grid.buildHypothesisMap(hypothesis_map, true);
		OrderedHypothesisMap::iterator it_cell = hypothesis_map.begin();

		// Partie à optimiser
		while (!sol.isSolved() || (sol.solutions().size() > 1) || (sol.filledCellsCount() > hints_number)) {

			size_t old_value = sol.getValue(it_cell->second.I, it_cell->second.J);
			sol.clearCell(it_cell->second.I, it_cell->second.J);

			sol.solve(false);

			// As soon as we diverge in solutions or there are none, we put back the deleted value
			if (sol.solutions().size() != 1) {
				RegulareSquare::ERROR_CODE err = sol.setValue(it_cell->second.I, it_cell->second.J, old_value);
			}

			it_cell++;

			// If we have gone through all the route without results, we regenerate a random route and start again
			if ((it_cell == hypothesis_map.end()) || (sol.filledCellsCount() < hints_number)) {
				std::time_t result = std::time(nullptr);
				std::strftime(mbstr, sizeof mbstr, "%c", std::localtime(&result));
				new_route_case++;
				std::cout << mbstr << " ########## build a new route #" << new_route_case << std::endl;
				sol = this->solutions()[0];
				empty_grid.buildHypothesisMap(hypothesis_map, true);
				it_cell = hypothesis_map.begin();
			}
		}

		// Set the internal grid from the computed result
		this->resetInternalGrids();
		this->setInternalGrid(sol.mInternalGrid);

		return ret;
	}

	RegulareSquare::ERROR_CODE RegulareSquare::solve(bool stop_on_first_solution) {

		RegulareSquare::ERROR_CODE ret = ERR_OK;

		// Switch back to unsolved status
		mSolved = false;
		mSolutions.clear();

		// Copy the grid in a new one
		RegulareSquare grid = *this;

		// build the hypothesis
		// Order the remaining void cells
		OrderedHypothesisMap hypothesis_map;

		grid.buildHypothesisMap(hypothesis_map);

		if (!hypothesis_map.empty()) {
			this->recursiveSolve(grid, hypothesis_map,hypothesis_map.begin(), stop_on_first_solution);
		} else {
			ret = ERR_NO_MORE_HYPOTHESIS;
		}

		return ret;
	}

	size_t RegulareSquare::getValue(size_t I, size_t J) {
		return 	mInternalGrid[I - 1][J - 1];
	}

	RegulareSquare::ERROR_CODE RegulareSquare::setValue(size_t I, size_t J, size_t value) {
	
		RegulareSquare::ERROR_CODE ret = ERR_OK;
		bool allowed = false;

		ret = this->checkValueAllowed(I, J, value, allowed);

		if (ret == ERR_OK) {
			if (allowed) {
				mInternalGrid[I - 1][J - 1] = value;
				// Update allowed values for row
				for (size_t i = 0; i < mMaxAllowedValue; i++) {
					mAllowedValuesMap[i][J - 1].erase(value);
				}
				// Update allowed values for column
				for (size_t j = 0; j < mMaxAllowedValue; j++) {
					mAllowedValuesMap[I - 1][j].erase(value);
				}

				// Update allowed values for block (@todo : faire une fonction ?)
				size_t I_min = ((I - 1) / mGridRootSize) * mGridRootSize + 1;
				size_t I_max = I_min + mGridRootSize - 1;
				size_t J_min = ((J - 1) / mGridRootSize) * mGridRootSize + 1;
				size_t J_max = J_min + mGridRootSize - 1;

				for (size_t V = 1; V <= mMaxAllowedValue; V++) {
					// dans le cas d'un set on supprime toutes les possibilités pour la case
					mAllowedValuesMap[I - 1][J - 1].clear();
				}

				for (size_t JJ = J_min; JJ <= J_max; JJ++) {
					for (size_t II = I_min; II <= I_max; II++) {
						mAllowedValuesMap[II - 1][JJ - 1].erase(value);
					}
				}

				// Switch back to unsolved status
				mSolved = false;
				mSolutions.clear();

			} else {
				ret = ERR_FORBIDDEN_VALUE;
			}
		}

		return ret;
	}

	RegulareSquare::ERROR_CODE RegulareSquare::clearCell(size_t I, size_t J) {
		RegulareSquare::ERROR_CODE ret = ERR_OK;
		if (!this->isInGridBounds(I, J)) {
			ret = ERR_OUT_OF_GRIDS_BOUNDS;
		} else {

			// Fast reset of the cell : reset the grid and set all values except this one
			InternalGrid i_grid_copy = mInternalGrid;

			i_grid_copy[I - 1][J - 1] = VOID_VALUE;

			this->resetInternalGrids();
			this->setInternalGrid(i_grid_copy);

		}
		return ret;
	}

	RegulareSquare::ERROR_CODE RegulareSquare::checkValueAllowed(size_t I, size_t J, size_t value, bool & allowed) const {
		RegulareSquare::ERROR_CODE ret = ERR_OK;

		allowed = false;

		if (!this->isInGridBounds(I, J)) {
			ret = ERR_OUT_OF_GRIDS_BOUNDS;
		} else if (!this->isInValuesBounds(value)) {
			ret = ERR_OUT_OF_VALUES_BOUNDS;
		} else {
			// First check if the value is still in the possible velues list for I,J
			allowed = (mAllowedValuesMap[I - 1][J - 1].count(value) > 0);
			if (allowed) {
				// If so, then check that setting this value wont head to
				// an impossible grid solution (i.e. no more solution for another unsetted cell)

				for (size_t i = 0; (i < mMaxAllowedValue) && allowed; i++) {
					if ((i != (I - 1)) && (mAllowedValuesMap[i][J - 1].count(value) == 1) && (mAllowedValuesMap[i][J - 1].size() == 1)) {
						allowed = false;
						ret = ERR_UNABLE_TO_FILL_HINTS;
					}
				}

				for (size_t j = 0; (j < mMaxAllowedValue) && allowed; j++) {
					if ((j != (J - 1)) && (mAllowedValuesMap[I - 1][j].count(value) == 1) && (mAllowedValuesMap[I - 1][j].size() == 1)) {
						allowed = false;
						ret = ERR_UNABLE_TO_FILL_HINTS;
					}
				}

				// Update allowed values for block (@todo : faire une fonction ?)
				size_t I_min = ((I - 1) / mGridRootSize) * mGridRootSize + 1;
				size_t I_max = I_min + mGridRootSize - 1;
				size_t J_min = ((J - 1) / mGridRootSize) * mGridRootSize + 1;
				size_t J_max = J_min + mGridRootSize - 1;

				for (size_t JJ = J_min; (JJ <= J_max) && allowed; JJ++) {
					for (size_t II = I_min; (II <= I_max) && allowed; II++) {
						if (((II != I) && (JJ != J)) && (mAllowedValuesMap[II - 1][JJ - 1].count(value) == 1) && (mAllowedValuesMap[II - 1][JJ - 1].size() == 1)) {
							allowed = false;
							ret = ERR_UNABLE_TO_FILL_HINTS;
						}
					}
				}
			}

		}

		return ret;
	}

	RegulareSquare::ERROR_CODE RegulareSquare::checkColumn(size_t I, bool& complete) const {
		RegulareSquare::ERROR_CODE ret = ERR_OK;
		complete = true;

		if ((I <= mMaxAllowedValue) && (I >= mMinAllowedValue)) {
			for (size_t j = 0; j < mMaxAllowedValue; j++) {
				if (VOID_VALUE == mInternalGrid[I - 1][j]) {
					complete = false;
				}
			}
		} else {
			ret = ERR_OUT_OF_GRIDS_BOUNDS;
		}

		return ret;
	}

	RegulareSquare::ERROR_CODE RegulareSquare::checkRow(size_t J, bool& complete) const {
		RegulareSquare::ERROR_CODE ret = ERR_OK;
		complete = true;

		if ((J <= mMaxAllowedValue) && (J >= mMinAllowedValue)) {
			for (size_t i = 0; i < mMaxAllowedValue; i++) {
				if (VOID_VALUE == mInternalGrid[i][J - 1]) {
					complete = false;
				}
			}
		}
		else {
			ret = ERR_OUT_OF_GRIDS_BOUNDS;
		}

		return ret;
	}

	RegulareSquare::ERROR_CODE RegulareSquare::checkBlock(size_t K, bool& complete) const {
		RegulareSquare::ERROR_CODE ret = ERR_OK;
		complete = true;

		size_t I_min = 0;
		size_t I_max = 0;
		size_t J_min = 0;
		size_t J_max = 0;

		if ((K <= mMaxAllowedValue) && (K >= mMinAllowedValue)) {

			this->getBlockBounds(K, I_min, I_max, J_min, J_max);

			for (size_t J = J_min; J <= J_max; J++) {
				for (size_t I = I_min; I <= I_max; I++) {
					if (VOID_VALUE == mInternalGrid[I - 1][J - 1]) {
						complete = false;
					}
				}
			}
		}
		else {
			ret = ERR_OUT_OF_GRIDS_BOUNDS;
		}

		return ret;
	}

	bool RegulareSquare::isGridCompleted() const {

		bool complete = true;

		for (size_t K = mMinAllowedValue; K <= mMaxAllowedValue; K++) {
			bool complete_block = false;
			checkBlock(K, complete_block);
			complete = complete && complete_block;
			if (!complete_block) {
				break;
			}
		}

		return complete;
	}

	size_t RegulareSquare::filledCellsCount() const {
		size_t ret = 0;

		for (size_t I = mMinAllowedValue; I <= mMaxAllowedValue; I++) {
			for (size_t J = mMinAllowedValue; J <= mMaxAllowedValue; J++) {
				if (VOID_VALUE != mInternalGrid[I-1][J-1]) {
					ret++;
				}
			}
		}

		return ret;
	}

	bool RegulareSquare::isInGridBounds(size_t i, size_t j) const {
		return (i <= mMaxAllowedValue) && (i >= mMinAllowedValue) && (j <= mMaxAllowedValue) && (j >= mMinAllowedValue);
	}

	bool RegulareSquare::isInValuesBounds(size_t value) const {
		return (value <= mMaxAllowedValue) && (value >= mMinAllowedValue);
	}

	void RegulareSquare::dump() const {

		for (size_t j = 0; j < mMaxAllowedValue; j++) {
			for (size_t i = 0; i < mMaxAllowedValue; i++) {
				if (VOID_VALUE != mInternalGrid[i][j]) {
					std::cout << " " << mInternalGrid[i][j];
				} else {
					std::cout << " " << "-";
				}
			}
			std::cout << std::endl;
		}

	}

	void RegulareSquare::dumpSolutions() const {
		size_t s = 1;
		for (auto it : mSolutions) {
			std::cout << "-------------------------------- Solution #" << s << std::endl;
			it.dump();
			s++;
		}
	}

	void RegulareSquare::dumpAllowedValuesNumbers() const {
		for (size_t j = 0; j < mMaxAllowedValue; j++) {
			for (size_t i = 0; i < mMaxAllowedValue; i++) {
				std::cout << " " << mAllowedValuesMap[i][j].size();
			}
			std::cout << std::endl;
		}
	}

	void RegulareSquare::resetInternalGrids() {
		for (size_t i = 0; i < mMaxAllowedValue; i++) {
			for (size_t j = 0; j < mMaxAllowedValue; j++) {
				mInternalGrid[i][j] = VOID_VALUE;
				for (size_t V = 1; V <= mMaxAllowedValue; V++) {
					mAllowedValuesMap[i][j].emplace(V);
				}
			}
		}
	}

	void RegulareSquare::setInternalGrid(const InternalGrid& grid) {
		for (size_t I = 1; I <= mMaxAllowedValue; I++) {
			for (size_t J = 1; J <= mMaxAllowedValue; J++) {
				this->setValue(I,J,grid[I-1][J-1]);
			}
		}
	}

	void RegulareSquare::buildHypothesisMap(OrderedHypothesisMap& hypothesis_map, bool scrambled) const {
		hypothesis_map.clear();
		for (size_t j = 0; j < this->mMaxAllowedValue; j++) {
			for (size_t i = 0; i < this->mMaxAllowedValue; i++) {
				if (this->mAllowedValuesMap[i][j].size() > 0) {
					CellHypothesis hypothesis;
					hypothesis.I = i + 1;
					hypothesis.J = j + 1;

					std::unordered_set<size_t> allowed_values;
					std::set<size_t> remainings_allowed_values = this->mAllowedValuesMap[i][j];
					while (remainings_allowed_values.size() > 0) {
						auto it = remainings_allowed_values.begin();
						if (scrambled) {
							std::advance(it, rand() % remainings_allowed_values.size());
						}
						allowed_values.emplace(*it);
						remainings_allowed_values.erase(it);
					}

					// Build the ordered hypothesis (the lower number of possible values first)
					for (auto possible_value : allowed_values) {
						hypothesis.Values.emplace(possible_value);
					}

					if (this->mAllowedValuesMap[i][j].size() > 0) {
						size_t weight;
						if (!scrambled) {
							weight = this->mAllowedValuesMap[i][j].size();
						} else {
							weight = rand() % (mMaxAllowedValue * mMaxAllowedValue);
						}
						hypothesis_map.emplace(weight, hypothesis);
					}
				}
			}
		}
	}

	bool RegulareSquare::recursiveSolve(const RegulareSquare & grid, 
		                                const OrderedHypothesisMap & hypothesis_map, 
		                                OrderedHypothesisMap::iterator next_hypothesis, 
		                                bool stop_on_first_solution) {

		bool solved = false;

		std::unordered_set<size_t>::iterator it_first_value = next_hypothesis->second.Values.begin();

		// Apply the first hypothesis to a grid copy
		RegulareSquare grid_copy = grid;

		RegulareSquare::ERROR_CODE ret = ERR_FORBIDDEN_VALUE;
		while ((ret != ERR_OK) &&
			   (!solved || !stop_on_first_solution) &&
			   (it_first_value != next_hypothesis->second.Values.end())) {

			RegulareSquare::ERROR_CODE ret = grid_copy.setValue(next_hypothesis->second.I, next_hypothesis->second.J, *it_first_value);

			if (ret == ERR_OK) {
				bool complete_grid = grid_copy.isGridCompleted();
				
				if (complete_grid) {
					solved = true;
					mSolved = true;
					mSolutions.push_back(grid_copy);
				} else {
					OrderedHypothesisMap::iterator it = next_hypothesis;
					it++;
					if (it != hypothesis_map.end()) {
						solved = recursiveSolve(grid_copy, hypothesis_map, it, stop_on_first_solution);
						if ((!solved) || (!stop_on_first_solution)) {
							// on tente la valeur possible suivante pour cette case
							it_first_value++;
							// on réinitialise la grille à l'état précédent
							grid_copy = grid;
						}
					}
				}
			} else {
				it_first_value++;
			}
		}

		return solved;
	}

	RegulareSquare::ERROR_CODE RegulareSquare::getBlockBounds(size_t K, size_t& I_MIN, size_t& I_MAX, size_t& J_MIN, size_t& J_MAX) const {
		RegulareSquare::ERROR_CODE ret = ERR_OK;

		size_t i_min = 0;
		size_t i_max = 0;
		size_t j_min = 0;
		size_t j_max = 0;

		if ((K <= mMaxAllowedValue) && (K >= mMinAllowedValue)) {

			i_min = ((K - 1) * mGridRootSize) % (mGridRootSize * mGridRootSize);
			i_max = i_min + mGridRootSize - 1;

			j_min = ((K - 1) / mGridRootSize) * mGridRootSize;
			j_max = j_min + mGridRootSize - 1;

			I_MIN = i_min + 1;
			I_MAX = i_max + 1;
			J_MIN = j_min + 1;
			J_MAX = j_max + 1;
		}
		else {
			ret = ERR_OUT_OF_GRIDS_BOUNDS;
		}

		return ret;
	} 

}