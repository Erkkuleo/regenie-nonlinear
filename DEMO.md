# Regenie Nonlinear — New Features Demo

All features described here were added after forking from the upstream `rgcgithub/regenie` release.

---

## 1. Nonlinear Interaction Covariate Expansion (`--nonlinear`)

**Commits:** `5021148` → `fd99cbb` → `55a041e` → `459f98e`

Instead of using a covariate directly as the interaction term in a GxE test, `--nonlinear` first transforms it through a nonlinear function (e.g. `sincos`, `invsin`). This is useful for circadian or seasonal phenotypes and for proportion-scale covariates.

### New CLI flags

| Flag | Type | Description |
|---|---|---|
| `--nonlinear` | bool | Enable nonlinear mode |
| `--nonlinear-function` | string | Function: `sincos`, `sin`, `cos`, `sinor`, `invsin`, `invcos`, `tan` |
| `--nonlinear-period` | float | Period divisor before 2π scaling |
| `--nonlinear-offset` | float | Phase offset added before transform |
| `--degree` | bool | Output in degrees instead of radians |

### Demo: single component (sin or cos only)

Use `sin` or `cos` when you only want one component of the circular expansion:

```bash
docker run -v $(pwd):/docker --rm regenie:v4.1.2 regenie \
  --step 2 \
  --bed /docker/example/example \
  --covarFile /docker/example/covariates.txt \
  --phenoFile /docker/example/phenotype_bin.txt \
  --remove /docker/example/fid_iid_to_remove.txt \
  --bsize 200 \
  --ignore-pred \
  --interaction V1 \
  --nonlinear \
  --nonlinear-function sin \
  --nonlinear-period 4.0 \
  --force-qt \
  --out /docker/test_nonlinear/sin_demo
```

Example output (`test_nonlinear/sin_demo_Y1.regenie`):

```
CHROM GENPOS ID ALLELE0 ALLELE1 A1FREQ N TEST            BETA       SE         CHISQ     LOG10P    NONLINEAR    EXTRA
1     1      1  2       1       0.2146  494 ADD-CONDTL      0.01926    0.02852    0.4558    0.3014    -0.002602    NA
1     1      1  2       1       0.2146  494 ADD-INT_SNP     0.02007    0.02880    0.4856    0.3135    -0.09102     NA
1     1      1  2       1       0.2146  494 ADD-INT_SNPxV1  0.02471    0.04098    0.3634    0.2623     0.26483     NA
1     1      1  2       1       0.2146  494 ADD-INT_2DF     NA         NA         0.6315    0.1371     0.83470     NA
```

Each SNP produces four rows:

| TEST | Description |
|---|---|
| `ADD-CONDTL` | Main SNP effect conditioned on the interaction model |
| `ADD-INT_SNP` | Main SNP effect within the interaction model |
| `ADD-INT_SNPxV1` | G×V1 interaction term (V1 transformed via `sin(2π·x/period)`) |
| `ADD-INT_2DF` | Joint 2 d.f. test combining SNP main effect and interaction; no BETA/SE |

The `ADD-INT_2DF` is a joint 2 degrees of freedom test that simultaneously tests:

1. Is there a main SNP effect? (`ADD-INT_SNP` beta)
2. Is there a SNP × covariate interaction? (`ADD-INT_SNPxV1` beta)

The idea is that if you only look at the interaction term alone you might miss a SNP that has a strong main effect but weak interaction, or vice versa. The 2DF test captures both signals at once — a SNP is flagged if either or both are non-zero. In practice it tends to be more powerful than the interaction-only test when the truth is a mixture of main effect and interaction.


### Demo: seasonal covariate (sincos)

Month of birth (1–12) is a circular variable. `sincos` expands it into sin and cos components, capturing the full cycle:

```bash
docker run -v $(pwd):/docker --rm regenie:v4.1.2 regenie \
  --step 2 \
  --bed /docker/example/example \
  --covarFile /docker/example/covariates.txt \
  --phenoFile /docker/example/phenotype_bin.txt \
  --remove /docker/example/fid_iid_to_remove.txt \
  --bsize 200 \
  --ignore-pred \
  --interaction V1 \
  --nonlinear \
  --nonlinear-function sincos \
  --nonlinear-period 4.0 \
  --force-qt \
  --out /docker/test_nonlinear/sincos_demo
```

`V1` is expanded into **two interaction columns**: `V1_sin` and `V1_cos`. The GxE test covers both components jointly.

Example output (`test_nonlinear/sincos_demo_Y1.regenie`):

```
CHROM GENPOS ID ALLELE0 ALLELE1 A1FREQ N TEST                   BETA        SE         CHISQ      LOG10P     NONLINEAR EXTRA
1     1      1  2       1       0.2146  494 ADD-CONDTL             0.01357     0.02827    0.2305     0.1999     NA        NA
1     1      1  2       1       0.2146  494 ADD-INT_SNP            0.01468     0.03128    0.2202     0.1946     NA        NA
1     1      1  2       1       0.2146  494 ADD-INT_SNPxV1=V1_sin  0.02324     0.04153    0.3132     0.2398     NA        NA
1     1      1  2       1       0.2146  494 ADD-INT_SNPxV1=V1_cos  0.00744     0.04077    0.03333    0.0680     NA        NA
1     1      1  2       1       0.2146  494 ADD-INT_SNPxV1         NA          NA         0.3762     0.0817     NA        NA
1     1      1  2       1       0.2146  494 ADD-INT_3DF            NA          NA         0.5672     0.0439     NA        NA
```

Each SNP produces **6 rows** with `sincos`: individual betas for `V1_sin` and `V1_cos`, a joint interaction chi-sq across both, and a 3DF test combining the SNP main effect with both interaction terms.


### Demo: proportion-scale covariate (arcsine stabilisation)

`invsin` applies `arcsin(sqrt(x))`, the classical variance-stabilising transform for proportion data:

```bash
docker run -v $(pwd):/docker --rm regenie:v4.1.2 regenie \
  --step 2 \
  --bed /docker/example/example \
  --covarFile /docker/example/covariates.txt \
  --phenoFile /docker/example/phenotype_bin.txt \
  --bsize 200 \
  --ignore-pred \
  --interaction V1 \
  --nonlinear \
  --nonlinear-function invsin \
  --nonlinear-period 1 \
  --force-qt \
  --out /docker/test_nonlinear/invsin_demo
```

### Supported functions

| Function | Columns | Formula |
|---|---|---|
| `sincos` | 2 (sin + cos) | `sin(2π·x/period + offset)`, `cos(2π·x/period + offset)` |
| `sin` | 1 | `sin(2π·x/period + offset)` |
| `cos` | 1 | `cos(2π·x/period + offset)` |
| `sinor` | 1 | `sin(2π·x/period + offset)` |
| `invsin` | 1 | `arcsin(sqrt(x))` |
| `invcos` | 1 | `arccos(2π·x/period + offset)` |
| `tan` | 1 | `tan(x)` |

---

## 2. Multi-Column Basis Expansion

**Commit:** `459f98e`

The architectural change underpinning the `sincos` feature. The raw interaction covariate is expanded at covariate-read time into a `MatrixXd` with as many columns as the function requires:

- `sincos` → 2 columns
- all other functions → 1 column

Column names (e.g. `V1_sin`, `V1_cos`) are auto-generated and appear in `--print-cov-betas` output.

---

## 3. Joint Two-Covariate GxE Test (`--interaction2`)

**Status:** uncommitted (current working changes)

Adds a second quantitative covariate to the interaction model so that a single variant test covers `G×E1` and `G×E2` jointly. Requires `--interaction` with a covariate; cannot be used with `--interaction-snp` or `--interaction-prs`.

### Demo

```bash
docker run -v $(pwd):/docker --rm regenie:v4.1.2 regenie \
  --step 2 \
  --bed /docker/example/example \
  --covarFile /docker/example/covariates.txt \
  --phenoFile /docker/example/phenotype_bin.txt \
  --bsize 200 \
  --ignore-pred \
  --interaction V1 \
  --interaction2 V2 \
  --force-qt \
  --out /docker/test_nonlinear/interaction2_demo
```

Example output (`test_nonlinear/interaction2_demo_Y1.regenie`):

```
CHROM GENPOS ID ALLELE0 ALLELE1 A1FREQ N TEST              BETA       SE        CHISQ     LOG10P    NONLINEAR EXTRA
1     1      1  2       1       0.214   500 ADD-CONDTL        0.02530    0.02730   0.8585    0.4508    NA        NA
1     1      1  2       1       0.214   500 ADD-INT_SNP       0.01350    0.02786   0.2350    0.2021    NA        NA
1     1      1  2       1       0.214   500 ADD-INT_SNPxV1=V1 0.00518    0.02122   0.05969   0.0931    NA        NA
1     1      1  2       1       0.214   500 ADD-INT_SNPxV1=V2 -0.02399   0.02290   1.097     0.5303    NA        NA
1     1      1  2       1       0.214   500 ADD-INT_SNPxV1    NA         NA        1.217     0.2642    NA        NA
1     1      1  2       1       0.214   500 ADD-INT_3DF       NA         NA        2.161     0.2679    NA        NA
```

Each SNP produces **6 rows**: individual interaction betas for `V1` and `V2`, a joint interaction chi-sq, and a 3DF test combining the SNP main effect with both interactions.

Output columns are labelled by covariate name (`V1`, `V2`).

### Combined with nonlinear

`--interaction2` can be stacked with `--nonlinear`. The nonlinear basis columns come first, then the second covariate is appended as the final column:

```bash
docker run -v $(pwd):/docker --rm regenie:v4.1.2 regenie \
  --step 2 \
  --bed /docker/example/example \
  --covarFile /docker/example/covariates.txt \
  --phenoFile /docker/example/phenotype_bin.txt \
  --remove /docker/example/fid_iid_to_remove.txt \
  --bsize 200 \
  --ignore-pred \
  --interaction V1 \
  --nonlinear \
  --nonlinear-function sincos \
  --nonlinear-period 4.0 \
  --interaction2 V2 \
  --force-qt \
  --out /docker/test_nonlinear/sincos_interaction2_demo
```

Example output (`test_nonlinear/sincos_interaction2_demo_Y1.regenie`):

```
CHROM GENPOS ID ALLELE0 ALLELE1 A1FREQ N TEST                   BETA       SE        CHISQ     LOG10P    NONLINEAR EXTRA
1     1      1  2       1       0.2146  494 ADD-CONDTL             0.01357    0.02827   0.2305    0.1999    NA        NA
1     1      1  2       1       0.2146  494 ADD-INT_SNP            0.01337    0.03141   0.1813    0.1738    NA        NA
1     1      1  2       1       0.2146  494 ADD-INT_SNPxV1=V1_sin  0.02682    0.03854   0.4844    0.3130    NA        NA
1     1      1  2       1       0.2146  494 ADD-INT_SNPxV1=V1_cos -0.00737    0.03625   0.04134   0.0763    NA        NA
1     1      1  2       1       0.2146  494 ADD-INT_SNPxV1=V2     -0.02717    0.02327   1.364     0.6147    NA        NA
1     1      1  2       1       0.2146  494 ADD-INT_SNPxV1         NA         NA        1.927     0.2308    NA        NA
1     1      1  2       1       0.2146  494 ADD-INT_4DF            NA         NA        2.475     0.1877    NA        NA
```

Each SNP produces **7 rows**: individual betas for `V1_sin`, `V1_cos`, and `V2`, a joint interaction chi-sq across all three, and a 4DF test combining the SNP main effect with all three interaction terms.

This produces three interaction columns: `V1_sin`, `V1_cos`, `V2`.

---

## 4. ISO Timestamp Utility Mode

**Commit:** `55a041e`

A small utility built into the regenie binary. Parses ISO 8601 timestamps and prints the elapsed hours, then exits — no GWAS analysis is run.

```bash
# Hours elapsed since a date
docker run --rm regenie:v4.1.2 regenie --iso-from 2025-09-01

# Hours between two timestamps
docker run --rm regenie:v4.1.2 regenie --iso-from 2025-09-01T08:00:00 --iso-to 2026-02-25T12:00:00
```

Accepted formats: `YYYY-MM-DD` and `YYYY-MM-DDTHH:MM:SS`.

---

## Feature Combination Reference

| Use case | Key flags |
|---|---|
| Seasonal/circadian GxE (full) | `--interaction MONTH --nonlinear --nonlinear-function sincos --nonlinear-period 12` |
| Seasonal GxE (sin only) | `--interaction MONTH --nonlinear --nonlinear-function sin --nonlinear-period 12` |
| Seasonal GxE (cos only) | `--interaction MONTH --nonlinear --nonlinear-function cos --nonlinear-period 12` |
| Proportion-scale covariate GxE | `--interaction PROP --nonlinear --nonlinear-function invsin --nonlinear-period 1` |
| Standard two-E joint test | `--interaction AGE --interaction2 BMI` |
| Seasonal + second covariate | `--interaction MONTH --nonlinear --nonlinear-function sincos --nonlinear-period 12 --interaction2 BMI` |
Put a note to yourself that the version 1.0. of this code sohuld have the followinf feattures cosinor and sinor model 2 Degrees of fredom model and also a day / month based flag that will take te ISO times detailed in the pheno file and then convert those to hours for the nonlinear calculations. That's it. This is for the circadian rithms model. Do you have something to ask or is this clear?