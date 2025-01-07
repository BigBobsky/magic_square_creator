#pragma once

#include <cstdint>
#include <set>
#include <unordered_set>
#include <vector>
#include <map>

namespace MagicSquares {

	// @todo On pourrait faire une classe template pour pouvoir utiliser autre chose que des size_t pour la grille

	// Structure locale décrivant un triplet colonne I, ligne J, valeurs Values
	typedef struct {
		size_t I;
		size_t J;
		std::unordered_set<size_t> Values;
	} CellHypothesis;

	class RegulareSquare
	{
	public:

		enum ERROR_CODE {
			ERR_OK = 0,
			ERR_FORBIDDEN_VALUE,
			ERR_OUT_OF_GRIDS_BOUNDS,
			ERR_OUT_OF_VALUES_BOUNDS,
			ERR_UNABLE_TO_FILL_HINTS,
			ERR_NO_MORE_HYPOTHESIS
		};

		RegulareSquare(size_t grid_root_size);

		virtual ~RegulareSquare();

		ERROR_CODE completeGridWithHints(size_t hints_number);

		ERROR_CODE solve(bool stop_on_first_solution = true);

		size_t getValue(size_t I, size_t J);
		ERROR_CODE setValue(size_t I, size_t J, size_t value);
		ERROR_CODE clearCell(size_t I, size_t J);

		ERROR_CODE checkValueAllowed(size_t I, size_t J, size_t value, bool & allowed) const;

		ERROR_CODE checkColumn(size_t I, bool& complete) const;
		ERROR_CODE checkRow(size_t J, bool & complete) const;
		ERROR_CODE checkBlock(size_t K, bool& complete) const;
		
		bool isGridCompleted() const;

		size_t filledCellsCount() const;

		bool isInGridBounds(size_t I, size_t J) const;
		bool isInValuesBounds(size_t value) const;

		bool isSolved() const {
			return mSolved;
		};

		size_t getSolutionsNumber() const {
			return mSolutions.size();
		};

		const std::vector<RegulareSquare>& solutions() const {
			return mSolutions;
		};

		void dump() const;

		void dumpSolutions() const;

		void dumpAllowedValuesNumbers() const;

	private:

		class InternalGrid : public std::vector< std::vector<size_t> > {
		
		public: 
			InternalGrid(size_t N, size_t V) : std::vector< std::vector<size_t> > (N, std::vector<size_t>(N,0)) {

			}

		};

		class OrderedHypothesisMap : public std::multimap<size_t, CellHypothesis> {};

		static const size_t VOID_VALUE = 0;


		size_t mGridRootSize;
		size_t mGridHintsNumber;

		size_t mMinAllowedValue;
		size_t mMaxAllowedValue;

		InternalGrid					                  mInternalGrid;         // i,j array of k values
		std::vector< std::vector< std::set<size_t> > >    mAllowedValuesMap;     // i,j set of values V allowed in i,j

		bool mSolved;
		std::vector<RegulareSquare> mSolutions;

		void resetInternalGrids();
		void setInternalGrid(const InternalGrid& grid);

		void buildHypothesisMap(OrderedHypothesisMap & hypothesis_map, bool scrambled = false) const;

		bool recursiveSolve(const RegulareSquare& grid,
							const OrderedHypothesisMap& hypothesis_map,
							OrderedHypothesisMap::iterator next_hypothesis,
							bool stop_on_first_solution = true);

		ERROR_CODE getBlockBounds(size_t K,
			size_t& I_MIN,
			size_t& I_MAX,
			size_t& J_MIN,
			size_t& J_MAX) const;

	};


}

