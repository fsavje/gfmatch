/* =============================================================================
 * quickmatch -- Fast Matching in Large Data Sets
 * https://github.com/fsavje/quickmatch
 *
 * Copyright (C) 2016  Fredrik Savje -- http://fredriksavje.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/
 * ============================================================================= */

#include "qmc_potential_outcomes.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <R.h>
#include <Rinternals.h>


// =============================================================================
// External function implementations
// =============================================================================

SEXP qmc_potential_outcomes(const SEXP R_outcomes,
                            const SEXP R_matching,
                            const SEXP R_treatment,
                            const SEXP R_estimands,
                            const SEXP R_subset_indicators,
                            const SEXP R_subset_treatments)
{
	if (!isReal(R_outcomes)) error("`R_outcomes` must be numeric.");
	if (!isInteger(R_matching)) error("`R_matching` must be integer.");
	if (!isInteger(R_treatment)) error("`R_treatment` must be integer.");
	if (xlength(R_outcomes) != xlength(R_matching)) {
		error("`R_outcomes` and `R_matching` must be same length.");
	}
	if (xlength(R_outcomes) != xlength(R_treatment)) {
		error("`R_outcomes` and `R_treatment` must be same length.");
	}
	if (asInteger(getAttrib(R_matching, install("cluster_count"))) <= 0) {
		error("`R_matching` is empty.");
	}
	if (!isLogical(R_estimands)) error("`R_estimands` must be logical.");

	const size_t num_observations = (size_t) xlength(R_outcomes);
	const size_t num_groups = (size_t) asInteger(getAttrib(R_matching, install("cluster_count")));
	const size_t num_treatments = (size_t) xlength(R_estimands);

	const double* const outcomes = REAL(R_outcomes);
	const int* const matching = INTEGER(R_matching);
	const int* const treatments = INTEGER(R_treatment);
	const int* const estimands = LOGICAL(R_estimands);

	const int* subset_indicators = NULL;
	if (!isNull(R_subset_indicators)) {
		if (!isLogical(R_subset_indicators)){
			error("`R_subset_indicators` must be logical.");
		}
		if (xlength(R_subset_indicators) != num_observations) {
			error("`R_outcomes` and `R_subset_indicators` must be same length.");
		}
		subset_indicators = LOGICAL(R_subset_indicators);
	}

	const int* subset_treatments = NULL;
	if (!isNull(R_subset_treatments)) {
		if (!isLogical(R_subset_treatments)) {
			error("`R_subset_treatments` must be logical.");
		}
		if (xlength(R_subset_treatments) != num_treatments) {
			error("`R_estimands` and `R_subset_treatments` must be same length.");
		}
		subset_treatments = LOGICAL(R_subset_treatments);
	}

	const bool ATE_weighting = (subset_indicators == NULL &&
	                            subset_treatments == NULL);

	SEXP R_out_means = PROTECT(allocVector(REALSXP, (R_xlen_t) num_treatments));
	double* const out_means = REAL(R_out_means);
	uint32_t* const weight_count = calloc(num_groups, sizeof(uint32_t));
	uint32_t* const treatment_count = calloc(num_groups * num_treatments, sizeof(uint32_t));
	double* const treatment_outcome_sum = calloc(num_groups * num_treatments, sizeof(double));

	if (weight_count == NULL ||
			treatment_count == NULL ||
			treatment_outcome_sum == NULL) {
		free(weight_count);
		free(treatment_count);
		free(treatment_outcome_sum);
		error("Out of memory.");
	}

	for (size_t i = 0; i < num_observations; ++i) {
		if (matching[i] == NA_INTEGER) continue;
		if (matching[i] < 0 || matching[i] >= num_groups) {
			error("Matching out of bounds.");
		}
		if (treatments[i] < 0 || treatments[i] >= num_treatments) {
			error("Treatment out of bounds.");
		}
		weight_count[matching[i]] += (ATE_weighting ||
				(subset_indicators != NULL && subset_indicators[i]) ||
				(subset_treatments != NULL && subset_treatments[treatments[i]]));
		++treatment_count[treatments[i] * num_groups + matching[i]];
		treatment_outcome_sum[treatments[i] * num_groups + matching[i]] += outcomes[i];
	}

	uint64_t total_weight_count = 0;
	for (size_t g = 0; g < num_groups; ++g) {
		total_weight_count += weight_count[g];
	}

	for (size_t t = 0; t < num_treatments; ++t) {
		if (!estimands[t]) {
			out_means[t] = NA_REAL;
		} else {
			out_means[t] = 0.0;
			const size_t t_add = t * num_groups;
			for (size_t g = 0; g < num_groups; ++g) {
				if (weight_count[g] > 0) {
					if (treatment_count[t_add + g] == 0) {
						out_means[t] = NA_REAL;
						break;
					} else {
						out_means[t] +=
							((double) weight_count[g]) *
							treatment_outcome_sum[t_add + g] /
							((double) treatment_count[t_add + g]);
					}
				}
			}
			if (!ISNA(out_means[t])) {
				 out_means[t] /= ((double) total_weight_count);
			}
		}
	}

	free(weight_count);
	free(treatment_count);
	free(treatment_outcome_sum);

	UNPROTECT(1);
	return R_out_means;
}