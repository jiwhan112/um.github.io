# ATLAS NOTE build-time Picomatch security fork

This private, repository-local package is Picomatch 4.0.7 (MIT), with one
reviewable parser guard. It is NOT an upstream security release. Original license
and JavaScript are retained; `upstream-sha256.json` records the original file
hashes, and a unit test reconstructs and verifies those bytes after removing the
single documented patch.

Source: https://registry.npmjs.org/picomatch/-/picomatch-4.0.7.tgz
Upstream tar integrity: sha512-qcJu88Q2IWqJsDD529JKMdwGm/dvInW4HvQnRwiH9JtihJvzGOscDtHE3x1pBKeUOTysQ8kVmLnJ2kJu7yhcGA==
Advisory: https://github.com/advisories/GHSA-fc5q-39xc-vjpf
Reviewed: 2026-11-18. Upstream latest 4.0.7 had no published fix at review time.

## Actual mitigation and compatibility boundary

The parser always literalizes repeated extglobs `+(...)` and `*(...)`, using
upstream's existing risky-expression literalization path. Thus no outer unbounded
quantifier from these constructs reaches the generated regular expression. This
covers single branches, alternatives, nesting and the upstream
`maxExtglobRecursion: false` opt-out, without depending on a heuristic proof of
which branch overlaps. `+(*a)` matches its literal spelling, NOT a run of `a`s.
This deliberately narrows the supported glob language. Ordinary stars/globstars,
braces, scan/parse/makeRe APIs and nonrepeating extglobs retain upstream behavior.
It is not a claim that every arbitrary regex/glob or explicit raw-regex option is
safe for hostile input. Do not use this build dependency as a general-purpose
security boundary for user-supplied regexes.

The site does not rely on repeated extglob matching. Added tests cover actual
TypeScript discovery through tinyglobby, standard Vite-style glob filters, API
compatibility and attack families in a timeout-bounded child process. The full
existing content, build, browser and publication gates remain mandatory.

The `picomatch` dependency and override resolve Vite, fdir and tinyglobby to this
same local source; no new remote package or install hook is trusted. The scoped
name distinguishes the modified implementation from the vulnerable upstream
release. `npm audit --audit-level=high` remains unchanged and scans all registry
dependencies, but audit does not validate a private patch: the source-integrity
and security regression tests are the independent gate for this fork. No advisory
is suppressed, no audit exit code is ignored, and no vulnerability is claimed
fixed merely because the package name changed.

## Retirement

When upstream publishes a reviewed fix, evaluate it against these security tests,
restore the registry dependency, remove the local override/vendor, and rerun the
complete validation/publication pipeline. Do not remove this mitigation based
only on a new version number or a green audit. Document intentional changes to
repeated-extglob semantics when restoring that feature.
