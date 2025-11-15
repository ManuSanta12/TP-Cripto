#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

usage() {
    cat <<EOF
Usage: $0 <carrier_bmp> [stegobmp_binary]

Runs embed/extract tests for:
  - Every supported crypto method/mode combination (LSB1, explicit -a/-m/-pass).
  - Parser defaults when only some crypto options are provided (LSBI, relying on defaults):
      * -a + -pass (mode defaults to cbc)
      * -m + -pass (method defaults to aes128)
      * only -pass (aes128-cbc by default)

The carrier BMP must be a writable 24-bit image.

Arguments:
  carrier_bmp       Path to the BMP file that will be used as carrier.
  stegobmp_binary   (Optional) Path to the stegobmp executable.
                    Defaults to "${PROJECT_ROOT}/build/stegobmp".
EOF
}

if [[ $# -lt 1 || $# -gt 2 ]]; then
    usage
    exit 1
fi

CARRIER_BMP="$1"
if [[ ! -f "${CARRIER_BMP}" ]]; then
    echo "Error: Carrier BMP '${CARRIER_BMP}' does not exist" >&2
    exit 1
fi

STEGOBMP_BIN="${2:-${PROJECT_ROOT}/build/stegobmp}"
if [[ ! -x "${STEGOBMP_BIN}" ]]; then
    echo "Error: stegobmp binary '${STEGOBMP_BIN}' not found or not executable" >&2
    exit 1
fi

WORKDIR="$(mktemp -d)"
cleanup() {
    rm -rf "${WORKDIR}"
}
trap cleanup EXIT

MESSAGE_FILE="${WORKDIR}/message.txt"
printf -- "Test payload generated at %s\n" "$(date -Iseconds)" > "${MESSAGE_FILE}"
printf -- "abcdefghijklmnopqrstuvwxyz0123456789\n" >> "${MESSAGE_FILE}"

declare -A MODES_BY_METHOD=(
    ["aes128"]="ecb cbc cfb ofb"
    ["aes192"]="ecb cbc cfb ofb"
    ["aes256"]="ecb cbc cfb ofb"
    ["3des"]="ecb cbc cfb ofb"
)

METHODS=("aes128" "aes192" "aes256" "3des")

PASSWORD="codex-test-password"
STEG_METHOD="LSB1"

printf -- "Carrier BMP: %s\n" "${CARRIER_BMP}"
printf -- "stegobmp executable: %s\n" "${STEGOBMP_BIN}"
printf -- "Working directory: %s\n" "${WORKDIR}"
printf -- "----------------------------------------\n"

FAILURES=0

for method in "${METHODS[@]}"; do
    for mode in ${MODES_BY_METHOD[${method}]}; do
        TEST_ID="${method}_${mode}"
        OUT_BMP="${WORKDIR}/embedded_${TEST_ID}.bmp"
        RECOVERY_PREFIX="${WORKDIR}/recovered_${TEST_ID}"
        RECOVERED_FILE="${RECOVERY_PREFIX}.txt"
        EMBED_LOG="${WORKDIR}/embed_${TEST_ID}.log"
        EXTRACT_LOG="${WORKDIR}/extract_${TEST_ID}.log"

        printf -- "\n[+] Testing %s with mode %s\n" "${method}" "${mode}"

        if ! "${STEGOBMP_BIN}" \
            -embed \
            -in "${MESSAGE_FILE}" \
            -p "${CARRIER_BMP}" \
            -out "${OUT_BMP}" \
            -steg "${STEG_METHOD}" \
            -a "${method}" \
            -m "${mode}" \
            -pass "${PASSWORD}" >"${EMBED_LOG}" 2>&1; then
            echo "  Embed failed. Check ${EMBED_LOG}"
            ((FAILURES++))
            continue
        fi

        if ! "${STEGOBMP_BIN}" \
            -extract \
            -p "${OUT_BMP}" \
            -out "${RECOVERY_PREFIX}" \
            -steg "${STEG_METHOD}" \
            -a "${method}" \
            -m "${mode}" \
            -pass "${PASSWORD}" >"${EXTRACT_LOG}" 2>&1; then
            echo "  Extract failed. Check ${EXTRACT_LOG}"
            ((FAILURES++))
            continue
        fi

        if ! cmp -s "${MESSAGE_FILE}" "${RECOVERED_FILE}"; then
            echo "  Payload mismatch for ${TEST_ID}"
            ((FAILURES++))
            continue
        fi

        echo "  OK"
    done
done

printf -- "\n[+] Testing parser defaults with LSBI steganography\n"

# 1) -a + -pass, sin -m (modo por defecto CBC)
DEFAULT_TESTS_FAIL=0

run_default_test() {
    local test_id="$1"
    shift
    local embed_args=("$@")

    local out_bmp="${WORKDIR}/embedded_defaults_${test_id}.bmp"
    local recovery_prefix="${WORKDIR}/recovered_defaults_${test_id}"
    local recovered_file="${recovery_prefix}.txt"
    local embed_log="${WORKDIR}/embed_defaults_${test_id}.log"
    local extract_log="${WORKDIR}/extract_defaults_${test_id}.log"

    printf -- "\n  [-] Default test: %s\n" "${test_id}"

    if ! "${STEGOBMP_BIN}" \
        -embed \
        -in "${MESSAGE_FILE}" \
        -p "${CARRIER_BMP}" \
        -out "${out_bmp}" \
        -steg "LSBI" \
        "${embed_args[@]}" >"${embed_log}" 2>&1; then
        echo "    Embed failed. Check ${embed_log}"
        ((FAILURES++))
        ((DEFAULT_TESTS_FAIL++))
        return
    fi

    if ! "${STEGOBMP_BIN}" \
        -extract \
        -p "${out_bmp}" \
        -out "${recovery_prefix}" \
        -steg "LSBI" \
        "${embed_args[@]}" >"${extract_log}" 2>&1; then
        echo "    Extract failed. Check ${extract_log}"
        ((FAILURES++))
        ((DEFAULT_TESTS_FAIL++))
        return
    fi

    if ! cmp -s "${MESSAGE_FILE}" "${recovered_file}"; then
        echo "    Payload mismatch for default test ${test_id}"
        ((FAILURES++))
        ((DEFAULT_TESTS_FAIL++))
        return
    fi

    echo "    OK"
}

run_default_test "a_plus_pass_3des" -a "3des" -pass "${PASSWORD}"
run_default_test "m_plus_pass_cbc" -m "cbc" -pass "${PASSWORD}"
run_default_test "pass_only" -pass "${PASSWORD}"

printf -- "\n----------------------------------------\n"
if [[ ${FAILURES} -eq 0 ]]; then
    printf -- "All crypto embedding tests (including parser defaults) succeeded.\n"
else
    printf -- "%d test(s) failed. Inspect the logs above.\n" "${FAILURES}"
    exit 1
fi
