package me.shadow.cobalt.core.verification

import me.shadow.cobalt.core.model.*
import me.shadow.cobalt.core.router.DefaultPolicyRouter
import me.shadow.cobalt.core.version.SemanticVersionNormalizer
import me.shadow.cobalt.core.version.routeFamily

/**
 * Verification script for the routing policy.
 * Run this to validate that version normalization and route selection work correctly.
 *
 * Expected results:
 *   "1.16.5"    -> LEGACY
 *   "1.17"      -> LEGACY
 *   "1.17.0"    -> LEGACY
 *   "1.17.1"    -> MODERN
 *   "1.18.0"    -> MODERN
 *   "1.20.4"    -> MODERN
 *   "latest"    -> AMBIGUOUS (rejected)
 *   "release"   -> AMBIGUOUS (rejected)
 *   ""          -> AMBIGUOUS (rejected)
 */
fun main() {
    val normalizer = SemanticVersionNormalizer()
    val router = DefaultPolicyRouter()

    println("=== Cobalt Routing Policy Verification ===")
    println("Policy: <= 1.17.0 -> LEGACY, > 1.17.0 -> MODERN")
    println()

    val testCases = listOf(
        // version text -> expected result
        "1.16.5" to RouteFamily.LEGACY,
        "1.16" to RouteFamily.LEGACY,
        "1.17" to RouteFamily.LEGACY,
        "1.17.0" to RouteFamily.LEGACY,
        "1.17.1" to RouteFamily.MODERN,
        "1.18.0" to RouteFamily.MODERN,
        "1.19" to RouteFamily.MODERN,
        "1.20.4" to RouteFamily.MODERN,
        "1.21" to RouteFamily.MODERN,
        "2.0" to RouteFamily.MODERN,
    )

    val rejectCases = listOf(
        "latest",
        "release",
        "",
        "snapshot",
        "newest",
    )

    var passed = 0
    var failed = 0

    println("--- Version Normalization ---")
    for ((versionText, expectedFamily) in testCases) {
        val info = normalizer.normalize(versionText, VersionSource.LAUNCHER_DESCRIPTOR)
        val key = info.normalizedKey
        val isAmbiguous = info.isAmbiguous

        if (isAmbiguous || key == null) {
            println("FAIL: '$versionText' -> AMBIGUOUS (expected $expectedFamily)")
            failed++
            continue
        }

        val actualFamily = key.routeFamily()
        if (actualFamily == expectedFamily) {
            println("PASS: '$versionText' -> ${key.toDisplayString()} -> $actualFamily")
            passed++
        } else {
            println("FAIL: '$versionText' -> ${key.toDisplayString()} -> $actualFamily (expected $expectedFamily)")
            failed++
        }
    }

    println()
    println("--- Rejected Aliases ---")
    for (alias in rejectCases) {
        val info = normalizer.normalize(alias, VersionSource.LAUNCHER_DESCRIPTOR)
        if (info.isAmbiguous) {
            println("PASS: '$alias' -> AMBIGUOUS (correctly rejected)")
            passed++
        } else {
            println("FAIL: '$alias' -> ${info.normalizedKey} (should be rejected)")
            failed++
        }
    }

    println()
    println("--- Policy Boundary Comparison ---")
    val boundary = VersionKey.POLICY_BOUNDARY
    println("Boundary: ${boundary.toDisplayString()}")

    val boundaryTests = listOf(
        VersionKey(1, 16, 5) to RouteFamily.LEGACY,
        VersionKey(1, 17, 0) to RouteFamily.LEGACY,
        VersionKey(1, 17, 1) to RouteFamily.MODERN,
        VersionKey(1, 18, 0) to RouteFamily.MODERN,
    )

    for ((key, expected) in boundaryTests) {
        val actual = router.policyFamily(key)
        if (actual == expected) {
            println("PASS: ${key.toDisplayString()} -> $actual")
            passed++
        } else {
            println("FAIL: ${key.toDisplayString()} -> $actual (expected $expected)")
            failed++
        }
    }

    println()
    println("--- Version Comparison ---")
    val comparisonTests = listOf(
        Triple(VersionKey(1, 17, 0), VersionKey(1, 17, 1), true),   // 1.17.0 < 1.17.1
        Triple(VersionKey(1, 17, 1), VersionKey(1, 17, 0), false),  // 1.17.1 > 1.17.0
        Triple(VersionKey(1, 17, 0), VersionKey(1, 17, 0), false),  // 1.17.0 == 1.17.0
    )

    for ((a, b, expectLess) in comparisonTests) {
        val isLess = a < b
        if (isLess == expectLess) {
            println("PASS: ${a.toDisplayString()} < ${b.toDisplayString()} = $isLess")
            passed++
        } else {
            println("FAIL: ${a.toDisplayString()} < ${b.toDisplayString()} = $isLess (expected $expectLess)")
            failed++
        }
    }

    println()
    println("--- Consistency Validation ---")
    val info1 = normalizer.normalize("1.17.1", VersionSource.LAUNCHER_DESCRIPTOR)
    val info2 = normalizer.normalize("1.17.1", VersionSource.INSTALLATION_METADATA)
    val info3 = normalizer.normalize("1.17.0", VersionSource.INSTALLATION_METADATA)

    val consistency1 = normalizer.validateConsistency(info1, info2)
    val consistency2 = normalizer.validateConsistency(info1, info3)

    if (consistency1 == null) {
        println("PASS: '1.17.1' vs '1.17.1' -> consistent")
        passed++
    } else {
        println("FAIL: '1.17.1' vs '1.17.1' -> ${consistency1.message}")
        failed++
    }

    if (consistency2 != null) {
        println("PASS: '1.17.1' vs '1.17.0' -> ${consistency2.message}")
        passed++
    } else {
        println("FAIL: '1.17.1' vs '1.17.0' -> should be inconsistent")
        failed++
    }

    println()
    println("=== Results: $passed passed, $failed failed ===")

    if (failed > 0) {
        println("VERIFICATION FAILED")
        System.exit(1)
    } else {
        println("ALL VERIFICATIONS PASSED")
    }
}
