[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

# regenie-nonlinear

**regenie-nonlinear** is a standalone fork of [regenie](https://github.com/rgcgithub/regenie) that extends the original whole genome regression framework with nonlinear covariate transformations for interaction testing.

This fork is based on regenie v4.1.2 and is maintained independently. While it tracks upstream releases, it introduces features not available in the original regenie.

## Added features

### Nonlinear interaction testing (`--nonlinear`)

Applies a nonlinear basis expansion to the interaction covariate before fitting the interaction model. Instead of a single linear SNP×E term, the covariate is expanded into one or more transformed columns, each tested as a separate interaction term and jointly via multi-DF tests.

This is useful for covariates with periodic or cyclical structure — for example, time of day or season — where a linear interaction term would miss the true signal.

#### New CLI options

| Option | Type | Description | Default |
|---|---|---|---|
| `--nonlinear` | bool | Enable nonlinear interaction mode | `false` |
| `--nonlinear-function <name>` | string | Transform to apply (see table below) | `sincos` |
| `--nonlinear-period <val>` | float | Period divisor used in sin/cos/sincos transforms | `1.0` |
| `--nonlinear-offset <val>` | float | Phase offset added inside the transform (radians by default) | `0.0` |
| `--degree` | bool | Interpret and return phase offset in degrees instead of radians | `false` |
| `--nonlinear-test <val>` | float | Scalar test value printed in the `NONLINEAR` output column (size-1 functions only) | `1.0` |
| `--timestamp <col>` | string | Column in the phenotype file containing ISO 8601 timestamps; required for circadian modes | — |
| `--timestamp-tz <offset>` | string | Shift naive/UTC timestamps to local time (e.g. `+03:00`, `-05:30`). Ignored for timestamps that already carry an explicit offset | `+00:00` |
| `--sunrise-zt` | bool | Re-anchor the cosinor clock to local sunrise (Zeitgeber Time) instead of midnight. Requires location via `--latitude`/`--longitude` or `--lat-col`/`--lon-col` | `false` |
| `--latitude <val>` | float | Study-site latitude in decimal degrees N; used by `--sunrise-zt` | — |
| `--longitude <val>` | float | Study-site longitude in decimal degrees E; used by `--sunrise-zt` | — |
| `--lat-col <col>` | string | Per-sample latitude column in the covariate file; used by `--sunrise-zt` | — |
| `--lon-col <col>` | string | Per-sample longitude column in the covariate file; used by `--sunrise-zt` | — |

#### Available functions

There are two families of functions: **generic** (applied to any covariate via `--interaction`) and **circadian** (derived from ISO 8601 sample timestamps via `--timestamp`).

**Generic transforms** — require `--interaction <col>`

| Function | Formula | Interaction columns | DF |
|---|---|---|---|
| `sincos` | sin(2π×x/period + offset), cos(2π×x/period + offset) | `<col>_sin`, `<col>_cos` | 2+1 |
| `sin` | sin(2π×x/period + offset) | `<col>_sin` | 1+1 |
| `cos` | cos(2π×x/period + offset) | `<col>_cos` | 1+1 |
| `invsin` | asin(asin(√x)/period × 2π + offset) | `<col>_invsin` | 1+1 |
| `invcos` | acos(x/period × 2π + offset) | `<col>_invcos` | 1+1 |
| `tan` | tan(x) | `<col>_tan` | 1+1 |

**Circadian transforms** — require `--timestamp <col>` (ISO 8601 column in the phenotype file)

| Function | Source | Formula | Interaction columns | DF |
|---|---|---|---|---|
| `cosinor` | timestamp | sin+cos of time-of-day and day-of-year | `tod_sin`, `tod_cos`, `toy_sin`, `toy_cos` | 4+1 |
| `sinor` | timestamp | sin of time-of-day and day-of-year | `tod_sin`, `toy_sin` | 2+1 |
| `tod_cosinor` | timestamp | cos(hour_of_day / 24 × 2π + offset) | `tod_cos` | 1+1 |
| `toy_cosinor` | timestamp | cos(day_of_year / 365 × 2π + offset) | `toy_cos` | 1+1 |

DF column shows (interaction-only joint test DF) + 1 for the SNP-main-effect joint test.

#### Output rows per SNP

Each variant produces one row per interaction column plus joint tests. Examples:

**`sincos`** (2 interaction columns)
```
ADD-CONDTL                    marginal SNP effect conditioned on interaction covariates
ADD-INT_SNP                   SNP main effect in interaction model
ADD-INT_SNPxV1=V1_sin         SNP × sin(V1)
ADD-INT_SNPxV1=V1_cos         SNP × cos(V1)
ADD-INT_2DF                   joint test: both interactions
ADD-INT_3DF                   joint test: SNP main + both interactions
```

**`cosinor`** (4 interaction columns, from timestamp)
```
ADD-CONDTL                    marginal SNP effect
ADD-INT_SNP                   SNP main effect
ADD-INT_SNPx=tod_sin          SNP × sin(hour_of_day / 24 × 2π)
ADD-INT_SNPx=tod_cos          SNP × cos(hour_of_day / 24 × 2π)
ADD-INT_SNPx=toy_sin          SNP × sin(day_of_year / 365 × 2π)
ADD-INT_SNPx=toy_cos          SNP × cos(day_of_year / 365 × 2π)
ADD-INT_4DF                   joint test: all 4 interactions
ADD-INT_5DF                   joint test: SNP main + all 4 interactions
```

**`sinor`** (2 interaction columns, from timestamp)
```
ADD-CONDTL                    marginal SNP effect
ADD-INT_SNP                   SNP main effect
ADD-INT_SNPx=tod_sin          SNP × sin(hour_of_day / 24 × 2π)
ADD-INT_SNPx=toy_sin          SNP × sin(day_of_year / 365 × 2π)
ADD-INT_2DF                   joint test: both interactions
ADD-INT_3DF                   joint test: SNP main + both interactions
```

**`tod_cosinor`** (1 interaction column, from timestamp)
```
ADD-CONDTL                    marginal SNP effect
ADD-INT_SNP                   SNP main effect
ADD-INT_SNPx=tod_cos          SNP × cos(time-of-day)
ADD-INT_1DF                   joint test: interaction only
ADD-INT_2DF                   joint test: SNP main + interaction
```

**`toy_cosinor`** (1 interaction column, from timestamp)
```
ADD-CONDTL                    marginal SNP effect
ADD-INT_SNP                   SNP main effect
ADD-INT_SNPx=toy_cos          SNP × cos(day-of-year)
ADD-INT_1DF                   joint test: interaction only
ADD-INT_2DF                   joint test: SNP main + interaction
```

The `NONLINEAR` output column is populated for size-1 functions (`sin`, `cos`, `tod_cosinor`, `toy_cosinor`) and contains the transformed value of `--nonlinear-test` at the variant p-value. It is `NA` for multi-column functions.

#### Usage examples

Generic sincos expansion of a covariate:
```bash
./regenie \
  --step 2 \
  --bed example/example \
  --covarFile example/covariates.txt \
  --phenoFile example/phenotype_bin.txt \
  --bsize 200 \
  --ignore-pred \
  --interaction V1 \
  --nonlinear \
  --nonlinear-function sincos \
  --nonlinear-period 24.0 \
  --force-qt \
  --out test_nonlinear_out
```

Full circadian cosinor using sample timestamps (UTC, Helsinki study site):
```bash
./regenie \
  --step 2 \
  --bed example/example \
  --covarFile example/covariates.txt \
  --phenoFile example/phenotype_with_timestamp.txt \
  --bsize 200 \
  --ignore-pred \
  --nonlinear \
  --nonlinear-function cosinor \
  --timestamp MOC_TIMEOFYEAR \
  --timestamp-tz +03:00 \
  --force-qt \
  --out test_cosinor_out
```

Circadian cosinor anchored to local sunrise (Zeitgeber Time):
```bash
./regenie \
  --step 2 \
  --bed example/example \
  --covarFile example/covariates.txt \
  --phenoFile example/phenotype_with_timestamp.txt \
  --bsize 200 \
  --ignore-pred \
  --nonlinear \
  --nonlinear-function cosinor \
  --timestamp MOC_TIMEOFYEAR \
  --timestamp-tz +03:00 \
  --sunrise-zt \
  --latitude 60.17 \
  --longitude 24.94 \
  --force-qt \
  --out test_cosinor_zt_out
```

### Timezone-aware ISO 8601 timestamp parsing

Timestamps in the phenotype file can carry an explicit UTC offset (`+03:00`, `-05:30`, `Z`) or be naive UTC strings. The parser handles all four ISO 8601 formats:

| Format | Example | Behaviour |
|---|---|---|
| Explicit offset | `2024-03-15T09:30:00+03:00` | Local wall-clock time read directly from the string |
| UTC (`Z`) | `2024-03-15T09:30:00Z` | Shifted by `--timestamp-tz` (default 0) |
| Naive | `2024-03-15T09:30:00` | Shifted by `--timestamp-tz` (default 0) |
| Date only | `2024-03-15` | Time-of-day set to 0 |

The system timezone of the machine running regenie is never consulted.

### Sunrise Zeitgeber Time (`--sunrise-zt`)

Re-anchors the cosinor clock to local sunrise rather than midnight, following the concept of Zeitgeber Time (ZT) used in circadian biology. Time-of-day for each sample is expressed as hours elapsed since sunrise on the day of measurement.

Sunrise is computed using the NOAA solar position algorithm. Location can be provided at the study level (`--latitude`/`--longitude`) or per-sample (`--lat-col`/`--lon-col` columns in the covariate file). Polar night (no sunrise) is handled gracefully — affected samples produce a warning and are excluded from ZT computation.

### ISO timestamp utility

Calculate hours between two ISO 8601 timestamps without running a GWAS:

```bash
regenie --iso-from 2024-01-15T08:00:00 --iso-to 2024-01-15T14:30:00
regenie --iso-from 2024-01-15T08:00:00   # hours from timestamp to now
```

### Nonlinear transform of p-values (`--fourier-transform-p`)

Applies the selected nonlinear transform to the association p-values and reports the result in the `NONLINEAR` and `TRANS_INTERACT` output columns (HTP format).

## Original regenie features

All features of the original regenie v4.1.2 are preserved:

- Works on quantitative, binary, and time-to-event traits, including binary traits with unbalanced case-control ratios
- Handles population structure and relatedness
- Processes multiple phenotypes at once efficiently
- Fast and memory efficient
- Supports Firth logistic regression, SPA test, and Firth cox regression
- Gene/region-based tests, interaction tests, and conditional analyses
- Supports BGEN, PLINK bed/bim/fam, and PLINK2 pgen/pvar/psam formats
- Can be implemented in Apache Spark (see [GLOW](https://projectglow.io/))

## Documentation

Full documentation for the original **regenie** can be found at [https://rgcgithub.github.io/regenie/](https://rgcgithub.github.io/regenie/).

The nonlinear features described above extend the `--interaction` functionality documented in the [GxE interaction testing](https://rgcgithub.github.io/regenie/options/#interaction-testing) section.

## Installation

### Docker

```bash
chmod u+x scripts/regenie_docker.sh
scripts/regenie_docker.sh --build
```

### From source

Requires the BGEN library. See the [original regenie installation instructions](https://rgcgithub.github.io/regenie/install/) for dependencies.

```bash
BGEN_PATH=/path/to/bgen cmake .
make
```

## Quick start

**Step 1** — Fit the whole genome regression model (unchanged from regenie):

```bash
./regenie \
  --step 1 \
  --bed example/example \
  --exclude example/snplist_rm.txt \
  --covarFile example/covariates.txt \
  --phenoFile example/phenotype_bin.txt \
  --remove example/fid_iid_to_remove.txt \
  --bsize 100 \
  --bt --lowmem \
  --lowmem-prefix tmp_rg \
  --out fit_bin_out
```

**Step 2** — Test for nonlinear interactions:

```bash
./regenie \
  --step 2 \
  --bgen example/example.bgen \
  --covarFile example/covariates.txt \
  --phenoFile example/phenotype_bin.txt \
  --remove example/fid_iid_to_remove.txt \
  --bsize 200 \
  --bt \
  --firth --approx \
  --pThresh 0.01 \
  --pred fit_bin_out_pred.list \
  --interaction V1 \
  --nonlinear \
  --nonlinear-function sincos \
  --nonlinear-period 24.0 \
  --force-qt \
  --out test_nonlinear_out
```

## Citation

If you use this software, please cite the original regenie paper:

> Mbatchou, J., Barnard, L., Backman, J. et al. Computationally efficient whole-genome regression for quantitative and binary traits. *Nat Genet* 53, 1097-1103 (2021). [https://doi.org/10.1038/s41588-021-00870-7](https://doi.org/10.1038/s41588-021-00870-7)

## Acknowledgements

This fork builds on the excellent work of the regenie team at the Regeneron Genetics Center. We are grateful to Joelle Mbatchou, Andrey Ziyatdinov, Jonathan Marchini, and all contributors to the [original regenie project](https://github.com/rgcgithub/regenie) for developing and open-sourcing this software under the MIT license.

## License

**regenie-nonlinear** is distributed under an [MIT license](https://github.com/rgcgithub/regenie/blob/master/LICENSE), the same license as the original regenie.

## Contact

For issues specific to the nonlinear features in this fork, please use the issue tracker on this repository.

For questions about the original regenie, please contact:
- <jonathan.marchini@regeneron.com>
- <joelle.mbatchou@regeneron.com>

Or use the original **regenie** [issue tracker](https://github.com/rgcgithub/regenie/issues).
