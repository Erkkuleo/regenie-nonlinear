[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

# regenie-nonlinear

**regenie-nonlinear** is a standalone fork of [regenie](https://github.com/rgcgithub/regenie) that extends the original whole genome regression framework with nonlinear covariate transformations for interaction testing.

This fork is based on regenie v4.1.2 and is maintained independently. While it tracks upstream releases, it introduces features not available in the original regenie.

## Added features

### Nonlinear interaction testing (`--nonlinear`)

Applies nonlinear basis expansions to the interaction covariate before fitting the interaction model. Instead of testing a single linear SNP x E interaction term, this expands the covariate into a nonlinear basis and tests multiple interaction terms jointly.

For example, with **cosinor** (the default), a covariate E is expanded into sin(E) and cos(E), giving four terms in the model:

- **sin(E)** and **cos(E)** as main-effect covariates
- **sin(E) x SNP** and **cos(E) x SNP** as interaction terms

This is useful for covariates with periodic or cyclical effects (e.g. time of day, season) where the relationship between the covariate and the outcome is not linear.

**Usage:**

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
  --nonlinear-period 24.0 \
  --force-qt \
  --out test_nonlinear_out
```

**Nonlinear options:**

| Option | Description | Default |
|---|---|---|
| `--nonlinear` | Enable nonlinear transformation of the interaction covariate | `false` |
| `--nonlinear-function` | Transform function: `cosinor`, `sinor`, `cos`, `invsin`, `invcos`, `tan` | `cosinor` |
| `--nonlinear-period` | Period parameter for the transform | `0.0` |
| `--nonlinear-offset` | Phase offset for the transform | `0.0` |
| `--degree` | Output results in degrees instead of radians | `false` |

**Available transforms:**

| Function | Expansion | Columns | Use case |
|---|---|---|---|
| `cosinor` | sin(2*pi*x/period + offset), cos(2*pi*x/period + offset) | 2 | Cyclical/periodic covariates |
| `sinor` | sin(2*pi*x/period + offset) | 1 | Single sine transform |
| `cos` | cos(x) | 1 | Single cosine transform |
| `invsin` | asin(asin(sqrt(x))/period * 2*pi + offset) | 1 | Proportion variance stabilization |
| `invcos` | acos(x/period * 2*pi + offset) | 1 | Inverse cosine transform |
| `tan` | tan(x) | 1 | Tangent transform |

The design is extensible — new transforms (e.g. GAM spline bases) can be added by extending three functions in `Nonlinear.cpp`.

**Output format:**

With cosinor, each variant produces 6 result rows (compared to 4 in standard interaction testing):

```
ADD-CONDTL            # SNP main effect conditional on sin(E), cos(E)
ADD-INT_SNP           # SNP marginal effect
ADD-INT_SNPxV1=V1_sin # Interaction: SNP x sin(E)
ADD-INT_SNPxV1=V1_cos # Interaction: SNP x cos(E)
ADD-INT_SNPxV1        # Joint interaction test (sin + cos)
ADD-INT_3DF           # Joint 3-df test (SNP + SNP*sin + SNP*cos)
```

A `NONLINEAR` column in the output contains the transformed interaction value.

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
