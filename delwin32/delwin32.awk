BEGIN {
        LVL = 0;
        IF  = 0;
        ELSE = 0;
}
{
        if (substr($0, 1, 12) == "#ifdef WIN32" ||
            substr($0, 1, 18) == "#if defined(WIN32)") {
                IF = 1;
                printf "%d: we are in IF\n", NR > "/dev/stderr";
        }
        if (IF && substr($0, 1, 3) == "#if") {
                ++LVL;
                printf "%d: LVL is incremented. LVL == %d\n", NR, LVL > "/dev/stderr";
                next;
        }
        if (IF && substr($0, 1, 6) == "#endif") {
                --LVL;
                printf "%d: LVL is decremented. LVL == %d\n", NR, LVL > "/dev/stderr";
                if (LVL == 0) {
                        IF = 0;
                        ELSE = 0;
                        printf "%d: we are out of IF and ELSE\n", NR > "/dev/stderr"
                }
                next;
        }
        if (LVL == 1 && substr($0, 1, 5) == "#else") {
                ELSE = 1;
                printf "%d: we are in ELSE\n", NR > "/dev/stderr";
                next;
        }
        if (!IF || ELSE)
                print;
}
