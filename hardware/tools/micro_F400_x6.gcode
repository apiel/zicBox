M3 ; Constant Power Laser On
G21 ; mm-mode
G1 F400 ; set line speed rate to 400 mm/min

G92 X0 Y0 ; set current head position to X0 Y0
G21 ; mm-mode

G0 Z10
G0 X0 Y0
G0 Z0
G1 X0 Y17.000210000000003 S1000
G1 X47.000150000000005 Y17.000210000000003 S1000
G1 X47.000150000000005 Y0 S1000
G1 X0 Y0 S1000

G0 Z10
G0 X33.02241 Y3.9790300000000003
G0 Z0
G1 X33.02241 Y12.978990000000001 S1000

G0 Z10
G0 X43.022310000000004 Y2.9790300000000003
G0 Z0
G1 X34.02241 Y2.9790300000000003 S1000

G0 Z10
G0 X44.022310000000004 Y12.978990000000001
G0 Z0
G1 X44.022310000000004 Y3.9790300000000003 S1000

G0 Z10
G0 X34.02241 Y13.978990000000001
G0 Z0
G1 X43.022310000000004 Y13.978990000000001 S1000

G0 Z10
G0 X18.02241 Y3.9790300000000003
G0 Z0
G1 X18.02241 Y12.978990000000001 S1000

G0 Z10
G0 X28.022420000000004 Y2.9790300000000003
G0 Z0
G1 X19.02241 Y2.9790300000000003 S1000

G0 Z10
G0 X29.022420000000004 Y12.978990000000001
G0 Z0
G1 X29.022420000000004 Y3.9790300000000003 S1000

G0 Z10
G0 X19.02241 Y13.978990000000001
G0 Z0
G1 X28.022420000000004 Y13.978990000000001 S1000

G0 Z10
G0 X3.0112200000000002 Y3.98952
G0 Z0
G1 X3.0112200000000002 Y12.989450000000001 S1000

G0 Z10
G0 X13.011170000000002 Y2.98952
G0 Z0
G1 X4.01121 Y2.98952 S1000

G0 Z10
G0 X14.011170000000002 Y12.989450000000001
G0 Z0
G1 X14.011170000000002 Y3.98952 S1000

G0 Z10
G0 X4.01121 Y13.98948
G0 Z0
G1 X13.011170000000002 Y13.98948 S1000

G0 Z10
G0 X33.02239 Y12.978990000000001
G0 Z0
G2 X34.022890000000004 Y13.978230000000002 I0.9887600000000001 J0.010490000000000001 S1000

G0 Z10
G0 X43.02237 Y13.978990000000001
G0 Z0
G2 X44.022870000000005 Y12.980010000000002 I0.0008900000000000001 J-0.9996200000000001 S1000

G0 Z10
G0 X44.02237 Y3.97901
G0 Z0
G2 X43.022110000000005 Y2.98003 I-1.0020600000000002 J0.0030700000000000002 S1000

G0 Z10
G0 X33.02239 Y3.97901
G0 Z0
G3 X34.022890000000004 Y2.98003 I1.02303 J0.02409 S1000

G0 Z10
G0 X18.02242 Y12.978990000000001
G0 Z0
G2 X19.022920000000003 Y13.978230000000002 I0.9887600000000001 J0.010490000000000001 S1000

G0 Z10
G0 X28.0224 Y13.978990000000001
G0 Z0
G2 X29.022900000000003 Y12.980010000000002 I0.0008900000000000001 J-0.9996200000000001 S1000

G0 Z10
G0 X29.0224 Y3.97901
G0 Z0
G2 X28.022140000000004 Y2.98003 I-1.0020600000000002 J0.0030700000000000002 S1000

G0 Z10
G0 X18.02242 Y3.97901
G0 Z0
G3 X19.022920000000003 Y2.98003 I1.02303 J0.02409 S1000

G0 Z10
G0 X3.0112200000000002 Y3.9895000000000005
G0 Z0
G3 X4.01173 Y2.99052 I1.02303 J0.02409 S1000

G0 Z10
G0 X14.0112 Y3.9895000000000005
G0 Z0
G2 X13.010950000000001 Y2.99052 I-1.00205 J0.0030700000000000002 S1000

G0 Z10
G0 X13.0112 Y13.98948
G0 Z0
G2 X14.01171 Y12.9905 I0.0008900000000000001 J-0.9996100000000001 S1000

G0 Z10
G0 X3.0112200000000002 Y12.98948
G0 Z0
G2 X4.01173 Y13.98872 I0.98877 J0.010490000000000001 S1000

G0 Z10


; Pass 2

G21 ; mm-mode

G0 Z10
G0 X0 Y0
G0 Z0
G1 X0 Y17.000210000000003 S1000
G1 X47.000150000000005 Y17.000210000000003 S1000
G1 X47.000150000000005 Y0 S1000
G1 X0 Y0 S1000

G0 Z10
G0 X33.02241 Y3.9790300000000003
G0 Z0
G1 X33.02241 Y12.978990000000001 S1000

G0 Z10
G0 X43.022310000000004 Y2.9790300000000003
G0 Z0
G1 X34.02241 Y2.9790300000000003 S1000

G0 Z10
G0 X44.022310000000004 Y12.978990000000001
G0 Z0
G1 X44.022310000000004 Y3.9790300000000003 S1000

G0 Z10
G0 X34.02241 Y13.978990000000001
G0 Z0
G1 X43.022310000000004 Y13.978990000000001 S1000

G0 Z10
G0 X18.02241 Y3.9790300000000003
G0 Z0
G1 X18.02241 Y12.978990000000001 S1000

G0 Z10
G0 X28.022420000000004 Y2.9790300000000003
G0 Z0
G1 X19.02241 Y2.9790300000000003 S1000

G0 Z10
G0 X29.022420000000004 Y12.978990000000001
G0 Z0
G1 X29.022420000000004 Y3.9790300000000003 S1000

G0 Z10
G0 X19.02241 Y13.978990000000001
G0 Z0
G1 X28.022420000000004 Y13.978990000000001 S1000

G0 Z10
G0 X3.0112200000000002 Y3.98952
G0 Z0
G1 X3.0112200000000002 Y12.989450000000001 S1000

G0 Z10
G0 X13.011170000000002 Y2.98952
G0 Z0
G1 X4.01121 Y2.98952 S1000

G0 Z10
G0 X14.011170000000002 Y12.989450000000001
G0 Z0
G1 X14.011170000000002 Y3.98952 S1000

G0 Z10
G0 X4.01121 Y13.98948
G0 Z0
G1 X13.011170000000002 Y13.98948 S1000

G0 Z10
G0 X33.02239 Y12.978990000000001
G0 Z0
G2 X34.022890000000004 Y13.978230000000002 I0.9887600000000001 J0.010490000000000001 S1000

G0 Z10
G0 X43.02237 Y13.978990000000001
G0 Z0
G2 X44.022870000000005 Y12.980010000000002 I0.0008900000000000001 J-0.9996200000000001 S1000

G0 Z10
G0 X44.02237 Y3.97901
G0 Z0
G2 X43.022110000000005 Y2.98003 I-1.0020600000000002 J0.0030700000000000002 S1000

G0 Z10
G0 X33.02239 Y3.97901
G0 Z0
G3 X34.022890000000004 Y2.98003 I1.02303 J0.02409 S1000

G0 Z10
G0 X18.02242 Y12.978990000000001
G0 Z0
G2 X19.022920000000003 Y13.978230000000002 I0.9887600000000001 J0.010490000000000001 S1000

G0 Z10
G0 X28.0224 Y13.978990000000001
G0 Z0
G2 X29.022900000000003 Y12.980010000000002 I0.0008900000000000001 J-0.9996200000000001 S1000

G0 Z10
G0 X29.0224 Y3.97901
G0 Z0
G2 X28.022140000000004 Y2.98003 I-1.0020600000000002 J0.0030700000000000002 S1000

G0 Z10
G0 X18.02242 Y3.97901
G0 Z0
G3 X19.022920000000003 Y2.98003 I1.02303 J0.02409 S1000

G0 Z10
G0 X3.0112200000000002 Y3.9895000000000005
G0 Z0
G3 X4.01173 Y2.99052 I1.02303 J0.02409 S1000

G0 Z10
G0 X14.0112 Y3.9895000000000005
G0 Z0
G2 X13.010950000000001 Y2.99052 I-1.00205 J0.0030700000000000002 S1000

G0 Z10
G0 X13.0112 Y13.98948
G0 Z0
G2 X14.01171 Y12.9905 I0.0008900000000000001 J-0.9996100000000001 S1000

G0 Z10
G0 X3.0112200000000002 Y12.98948
G0 Z0
G2 X4.01173 Y13.98872 I0.98877 J0.010490000000000001 S1000

G0 Z10


; Pass 3

G21 ; mm-mode

G0 Z10
G0 X0 Y0
G0 Z0
G1 X0 Y17.000210000000003 S1000
G1 X47.000150000000005 Y17.000210000000003 S1000
G1 X47.000150000000005 Y0 S1000
G1 X0 Y0 S1000

G0 Z10
G0 X33.02241 Y3.9790300000000003
G0 Z0
G1 X33.02241 Y12.978990000000001 S1000

G0 Z10
G0 X43.022310000000004 Y2.9790300000000003
G0 Z0
G1 X34.02241 Y2.9790300000000003 S1000

G0 Z10
G0 X44.022310000000004 Y12.978990000000001
G0 Z0
G1 X44.022310000000004 Y3.9790300000000003 S1000

G0 Z10
G0 X34.02241 Y13.978990000000001
G0 Z0
G1 X43.022310000000004 Y13.978990000000001 S1000

G0 Z10
G0 X18.02241 Y3.9790300000000003
G0 Z0
G1 X18.02241 Y12.978990000000001 S1000

G0 Z10
G0 X28.022420000000004 Y2.9790300000000003
G0 Z0
G1 X19.02241 Y2.9790300000000003 S1000

G0 Z10
G0 X29.022420000000004 Y12.978990000000001
G0 Z0
G1 X29.022420000000004 Y3.9790300000000003 S1000

G0 Z10
G0 X19.02241 Y13.978990000000001
G0 Z0
G1 X28.022420000000004 Y13.978990000000001 S1000

G0 Z10
G0 X3.0112200000000002 Y3.98952
G0 Z0
G1 X3.0112200000000002 Y12.989450000000001 S1000

G0 Z10
G0 X13.011170000000002 Y2.98952
G0 Z0
G1 X4.01121 Y2.98952 S1000

G0 Z10
G0 X14.011170000000002 Y12.989450000000001
G0 Z0
G1 X14.011170000000002 Y3.98952 S1000

G0 Z10
G0 X4.01121 Y13.98948
G0 Z0
G1 X13.011170000000002 Y13.98948 S1000

G0 Z10
G0 X33.02239 Y12.978990000000001
G0 Z0
G2 X34.022890000000004 Y13.978230000000002 I0.9887600000000001 J0.010490000000000001 S1000

G0 Z10
G0 X43.02237 Y13.978990000000001
G0 Z0
G2 X44.022870000000005 Y12.980010000000002 I0.0008900000000000001 J-0.9996200000000001 S1000

G0 Z10
G0 X44.02237 Y3.97901
G0 Z0
G2 X43.022110000000005 Y2.98003 I-1.0020600000000002 J0.0030700000000000002 S1000

G0 Z10
G0 X33.02239 Y3.97901
G0 Z0
G3 X34.022890000000004 Y2.98003 I1.02303 J0.02409 S1000

G0 Z10
G0 X18.02242 Y12.978990000000001
G0 Z0
G2 X19.022920000000003 Y13.978230000000002 I0.9887600000000001 J0.010490000000000001 S1000

G0 Z10
G0 X28.0224 Y13.978990000000001
G0 Z0
G2 X29.022900000000003 Y12.980010000000002 I0.0008900000000000001 J-0.9996200000000001 S1000

G0 Z10
G0 X29.0224 Y3.97901
G0 Z0
G2 X28.022140000000004 Y2.98003 I-1.0020600000000002 J0.0030700000000000002 S1000

G0 Z10
G0 X18.02242 Y3.97901
G0 Z0
G3 X19.022920000000003 Y2.98003 I1.02303 J0.02409 S1000

G0 Z10
G0 X3.0112200000000002 Y3.9895000000000005
G0 Z0
G3 X4.01173 Y2.99052 I1.02303 J0.02409 S1000

G0 Z10
G0 X14.0112 Y3.9895000000000005
G0 Z0
G2 X13.010950000000001 Y2.99052 I-1.00205 J0.0030700000000000002 S1000

G0 Z10
G0 X13.0112 Y13.98948
G0 Z0
G2 X14.01171 Y12.9905 I0.0008900000000000001 J-0.9996100000000001 S1000

G0 Z10
G0 X3.0112200000000002 Y12.98948
G0 Z0
G2 X4.01173 Y13.98872 I0.98877 J0.010490000000000001 S1000

G0 Z10


; Pass 4

G21 ; mm-mode

G0 Z10
G0 X0 Y0
G0 Z0
G1 X0 Y17.000210000000003 S1000
G1 X47.000150000000005 Y17.000210000000003 S1000
G1 X47.000150000000005 Y0 S1000
G1 X0 Y0 S1000

G0 Z10
G0 X33.02241 Y3.9790300000000003
G0 Z0
G1 X33.02241 Y12.978990000000001 S1000

G0 Z10
G0 X43.022310000000004 Y2.9790300000000003
G0 Z0
G1 X34.02241 Y2.9790300000000003 S1000

G0 Z10
G0 X44.022310000000004 Y12.978990000000001
G0 Z0
G1 X44.022310000000004 Y3.9790300000000003 S1000

G0 Z10
G0 X34.02241 Y13.978990000000001
G0 Z0
G1 X43.022310000000004 Y13.978990000000001 S1000

G0 Z10
G0 X18.02241 Y3.9790300000000003
G0 Z0
G1 X18.02241 Y12.978990000000001 S1000

G0 Z10
G0 X28.022420000000004 Y2.9790300000000003
G0 Z0
G1 X19.02241 Y2.9790300000000003 S1000

G0 Z10
G0 X29.022420000000004 Y12.978990000000001
G0 Z0
G1 X29.022420000000004 Y3.9790300000000003 S1000

G0 Z10
G0 X19.02241 Y13.978990000000001
G0 Z0
G1 X28.022420000000004 Y13.978990000000001 S1000

G0 Z10
G0 X3.0112200000000002 Y3.98952
G0 Z0
G1 X3.0112200000000002 Y12.989450000000001 S1000

G0 Z10
G0 X13.011170000000002 Y2.98952
G0 Z0
G1 X4.01121 Y2.98952 S1000

G0 Z10
G0 X14.011170000000002 Y12.989450000000001
G0 Z0
G1 X14.011170000000002 Y3.98952 S1000

G0 Z10
G0 X4.01121 Y13.98948
G0 Z0
G1 X13.011170000000002 Y13.98948 S1000

G0 Z10
G0 X33.02239 Y12.978990000000001
G0 Z0
G2 X34.022890000000004 Y13.978230000000002 I0.9887600000000001 J0.010490000000000001 S1000

G0 Z10
G0 X43.02237 Y13.978990000000001
G0 Z0
G2 X44.022870000000005 Y12.980010000000002 I0.0008900000000000001 J-0.9996200000000001 S1000

G0 Z10
G0 X44.02237 Y3.97901
G0 Z0
G2 X43.022110000000005 Y2.98003 I-1.0020600000000002 J0.0030700000000000002 S1000

G0 Z10
G0 X33.02239 Y3.97901
G0 Z0
G3 X34.022890000000004 Y2.98003 I1.02303 J0.02409 S1000

G0 Z10
G0 X18.02242 Y12.978990000000001
G0 Z0
G2 X19.022920000000003 Y13.978230000000002 I0.9887600000000001 J0.010490000000000001 S1000

G0 Z10
G0 X28.0224 Y13.978990000000001
G0 Z0
G2 X29.022900000000003 Y12.980010000000002 I0.0008900000000000001 J-0.9996200000000001 S1000

G0 Z10
G0 X29.0224 Y3.97901
G0 Z0
G2 X28.022140000000004 Y2.98003 I-1.0020600000000002 J0.0030700000000000002 S1000

G0 Z10
G0 X18.02242 Y3.97901
G0 Z0
G3 X19.022920000000003 Y2.98003 I1.02303 J0.02409 S1000

G0 Z10
G0 X3.0112200000000002 Y3.9895000000000005
G0 Z0
G3 X4.01173 Y2.99052 I1.02303 J0.02409 S1000

G0 Z10
G0 X14.0112 Y3.9895000000000005
G0 Z0
G2 X13.010950000000001 Y2.99052 I-1.00205 J0.0030700000000000002 S1000

G0 Z10
G0 X13.0112 Y13.98948
G0 Z0
G2 X14.01171 Y12.9905 I0.0008900000000000001 J-0.9996100000000001 S1000

G0 Z10
G0 X3.0112200000000002 Y12.98948
G0 Z0
G2 X4.01173 Y13.98872 I0.98877 J0.010490000000000001 S1000

G0 Z10


; Pass 5

G21 ; mm-mode

G0 Z10
G0 X0 Y0
G0 Z0
G1 X0 Y17.000210000000003 S1000
G1 X47.000150000000005 Y17.000210000000003 S1000
G1 X47.000150000000005 Y0 S1000
G1 X0 Y0 S1000

G0 Z10
G0 X33.02241 Y3.9790300000000003
G0 Z0
G1 X33.02241 Y12.978990000000001 S1000

G0 Z10
G0 X43.022310000000004 Y2.9790300000000003
G0 Z0
G1 X34.02241 Y2.9790300000000003 S1000

G0 Z10
G0 X44.022310000000004 Y12.978990000000001
G0 Z0
G1 X44.022310000000004 Y3.9790300000000003 S1000

G0 Z10
G0 X34.02241 Y13.978990000000001
G0 Z0
G1 X43.022310000000004 Y13.978990000000001 S1000

G0 Z10
G0 X18.02241 Y3.9790300000000003
G0 Z0
G1 X18.02241 Y12.978990000000001 S1000

G0 Z10
G0 X28.022420000000004 Y2.9790300000000003
G0 Z0
G1 X19.02241 Y2.9790300000000003 S1000

G0 Z10
G0 X29.022420000000004 Y12.978990000000001
G0 Z0
G1 X29.022420000000004 Y3.9790300000000003 S1000

G0 Z10
G0 X19.02241 Y13.978990000000001
G0 Z0
G1 X28.022420000000004 Y13.978990000000001 S1000

G0 Z10
G0 X3.0112200000000002 Y3.98952
G0 Z0
G1 X3.0112200000000002 Y12.989450000000001 S1000

G0 Z10
G0 X13.011170000000002 Y2.98952
G0 Z0
G1 X4.01121 Y2.98952 S1000

G0 Z10
G0 X14.011170000000002 Y12.989450000000001
G0 Z0
G1 X14.011170000000002 Y3.98952 S1000

G0 Z10
G0 X4.01121 Y13.98948
G0 Z0
G1 X13.011170000000002 Y13.98948 S1000

G0 Z10
G0 X33.02239 Y12.978990000000001
G0 Z0
G2 X34.022890000000004 Y13.978230000000002 I0.9887600000000001 J0.010490000000000001 S1000

G0 Z10
G0 X43.02237 Y13.978990000000001
G0 Z0
G2 X44.022870000000005 Y12.980010000000002 I0.0008900000000000001 J-0.9996200000000001 S1000

G0 Z10
G0 X44.02237 Y3.97901
G0 Z0
G2 X43.022110000000005 Y2.98003 I-1.0020600000000002 J0.0030700000000000002 S1000

G0 Z10
G0 X33.02239 Y3.97901
G0 Z0
G3 X34.022890000000004 Y2.98003 I1.02303 J0.02409 S1000

G0 Z10
G0 X18.02242 Y12.978990000000001
G0 Z0
G2 X19.022920000000003 Y13.978230000000002 I0.9887600000000001 J0.010490000000000001 S1000

G0 Z10
G0 X28.0224 Y13.978990000000001
G0 Z0
G2 X29.022900000000003 Y12.980010000000002 I0.0008900000000000001 J-0.9996200000000001 S1000

G0 Z10
G0 X29.0224 Y3.97901
G0 Z0
G2 X28.022140000000004 Y2.98003 I-1.0020600000000002 J0.0030700000000000002 S1000

G0 Z10
G0 X18.02242 Y3.97901
G0 Z0
G3 X19.022920000000003 Y2.98003 I1.02303 J0.02409 S1000

G0 Z10
G0 X3.0112200000000002 Y3.9895000000000005
G0 Z0
G3 X4.01173 Y2.99052 I1.02303 J0.02409 S1000

G0 Z10
G0 X14.0112 Y3.9895000000000005
G0 Z0
G2 X13.010950000000001 Y2.99052 I-1.00205 J0.0030700000000000002 S1000

G0 Z10
G0 X13.0112 Y13.98948
G0 Z0
G2 X14.01171 Y12.9905 I0.0008900000000000001 J-0.9996100000000001 S1000

G0 Z10
G0 X3.0112200000000002 Y12.98948
G0 Z0
G2 X4.01173 Y13.98872 I0.98877 J0.010490000000000001 S1000

G0 Z10


; Pass 6

G21 ; mm-mode

G0 Z10
G0 X0 Y0
G0 Z0
G1 X0 Y17.000210000000003 S1000
G1 X47.000150000000005 Y17.000210000000003 S1000
G1 X47.000150000000005 Y0 S1000
G1 X0 Y0 S1000

G0 Z10
G0 X33.02241 Y3.9790300000000003
G0 Z0
G1 X33.02241 Y12.978990000000001 S1000

G0 Z10
G0 X43.022310000000004 Y2.9790300000000003
G0 Z0
G1 X34.02241 Y2.9790300000000003 S1000

G0 Z10
G0 X44.022310000000004 Y12.978990000000001
G0 Z0
G1 X44.022310000000004 Y3.9790300000000003 S1000

G0 Z10
G0 X34.02241 Y13.978990000000001
G0 Z0
G1 X43.022310000000004 Y13.978990000000001 S1000

G0 Z10
G0 X18.02241 Y3.9790300000000003
G0 Z0
G1 X18.02241 Y12.978990000000001 S1000

G0 Z10
G0 X28.022420000000004 Y2.9790300000000003
G0 Z0
G1 X19.02241 Y2.9790300000000003 S1000

G0 Z10
G0 X29.022420000000004 Y12.978990000000001
G0 Z0
G1 X29.022420000000004 Y3.9790300000000003 S1000

G0 Z10
G0 X19.02241 Y13.978990000000001
G0 Z0
G1 X28.022420000000004 Y13.978990000000001 S1000

G0 Z10
G0 X3.0112200000000002 Y3.98952
G0 Z0
G1 X3.0112200000000002 Y12.989450000000001 S1000

G0 Z10
G0 X13.011170000000002 Y2.98952
G0 Z0
G1 X4.01121 Y2.98952 S1000

G0 Z10
G0 X14.011170000000002 Y12.989450000000001
G0 Z0
G1 X14.011170000000002 Y3.98952 S1000

G0 Z10
G0 X4.01121 Y13.98948
G0 Z0
G1 X13.011170000000002 Y13.98948 S1000

G0 Z10
G0 X33.02239 Y12.978990000000001
G0 Z0
G2 X34.022890000000004 Y13.978230000000002 I0.9887600000000001 J0.010490000000000001 S1000

G0 Z10
G0 X43.02237 Y13.978990000000001
G0 Z0
G2 X44.022870000000005 Y12.980010000000002 I0.0008900000000000001 J-0.9996200000000001 S1000

G0 Z10
G0 X44.02237 Y3.97901
G0 Z0
G2 X43.022110000000005 Y2.98003 I-1.0020600000000002 J0.0030700000000000002 S1000

G0 Z10
G0 X33.02239 Y3.97901
G0 Z0
G3 X34.022890000000004 Y2.98003 I1.02303 J0.02409 S1000

G0 Z10
G0 X18.02242 Y12.978990000000001
G0 Z0
G2 X19.022920000000003 Y13.978230000000002 I0.9887600000000001 J0.010490000000000001 S1000

G0 Z10
G0 X28.0224 Y13.978990000000001
G0 Z0
G2 X29.022900000000003 Y12.980010000000002 I0.0008900000000000001 J-0.9996200000000001 S1000

G0 Z10
G0 X29.0224 Y3.97901
G0 Z0
G2 X28.022140000000004 Y2.98003 I-1.0020600000000002 J0.0030700000000000002 S1000

G0 Z10
G0 X18.02242 Y3.97901
G0 Z0
G3 X19.022920000000003 Y2.98003 I1.02303 J0.02409 S1000

G0 Z10
G0 X3.0112200000000002 Y3.9895000000000005
G0 Z0
G3 X4.01173 Y2.99052 I1.02303 J0.02409 S1000

G0 Z10
G0 X14.0112 Y3.9895000000000005
G0 Z0
G2 X13.010950000000001 Y2.99052 I-1.00205 J0.0030700000000000002 S1000

G0 Z10
G0 X13.0112 Y13.98948
G0 Z0
G2 X14.01171 Y12.9905 I0.0008900000000000001 J-0.9996100000000001 S1000

G0 Z10
G0 X3.0112200000000002 Y12.98948
G0 Z0
G2 X4.01173 Y13.98872 I0.98877 J0.010490000000000001 S1000

G0 Z10

G0 Z10
M5 ; Laser Off