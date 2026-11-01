# Compare syscall histograms, not PIDs, addresses, or strace exit-summary lines.
BEGIN {
    expected["getpid"] = 1
    expected["getppid"] = 1
    expected["getuid"] = 1
    expected["geteuid"] = 1
    expected["getgid"] = 1
}
/^\+\+\+ exited with 0 \+\+\+$/ {
    if (FNR == NR)
        empty_ok = 1
    else
        part1_ok = 1
}
/^[[:alnum:]_]+\(/ {
    name = $0
    sub(/\(.*/, "", name)
    names[name] = 1
    if (FNR == NR) {
        baseline[name]++
        empty_count++
    } else {
        added[name]++
        part1_count++
    }
}
END {
    for (name in expected)
        names[name] = 1
    for (name in names) {
        delta = added[name] - baseline[name]
        if (delta != expected[name]) {
            printf "FAIL: %s changed by %d; expected %d\n", name, delta, expected[name]
            failed = 1
        }
    }
    printf "empty: %d calls; part1: %d calls; difference: %d\n", \
        empty_count, part1_count, part1_count - empty_count
    if (!empty_ok || !part1_ok || part1_count - empty_count != 5)
        failed = 1
    if (failed) {
        print "FAIL: both programs must exit successfully with exactly"
        print "the five expected extra calls."
        exit 1
    }
    print "PASS: exactly one extra getpid, getppid, getuid, geteuid, and getgid."
}
