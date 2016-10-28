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

#ifndef QMC_POTENTIAL_OUTCOMES_HG
#define QMC_POTENTIAL_OUTCOMES_HG

#include <R.h>
#include <Rinternals.h>

SEXP qmc_potential_outcomes(SEXP R_outcomes,
                            SEXP R_matching,
                            SEXP R_treatment,
                            SEXP R_estimands,
                            SEXP R_subset_indicators,
                            SEXP R_subset_treatments);

#endif // ifndef QMC_POTENTIAL_OUTCOMES_HG